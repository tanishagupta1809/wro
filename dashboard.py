import streamlit as st
import pandas as pd
import folium
from streamlit_folium import st_folium
import geocoder
from geopy.distance import geodesic
from ortools.constraint_solver import pywrapcp, routing_enums_pb2
from pymongo import MongoClient
import certifi

# -------------------------------
# MongoDB Connection
# -------------------------------
MONGO_URI = "mongodb+srv://tanisha:mongodb47@cluster0.duy9rx8.mongodb.net/?retryWrites=true&w=majority"
client = MongoClient(MONGO_URI, tlsCAFile=certifi.where())
db = client["waste_db"]
bins_collection = db["bins"]

# -------------------------------
# Helper Functions
# -------------------------------
def get_all_bins():
    docs = list(bins_collection.find({}, {"_id": 0}))
    for d in docs:
        d.setdefault("bin_id", d.get("id", "unknown"))
        d.setdefault("location", d.get("location", "N/A"))
        d.setdefault("status", d.get("status", "Full"))
        d.setdefault("latitude", d.get("latitude", d.get("lat", None)))
        d.setdefault("longitude", d.get("longitude", d.get("lng", None)))
    return docs

def add_bin(bin_id, location, lat, lon):
    # Prevent duplicate bin IDs
    existing = bins_collection.find_one({"bin_id": bin_id})
    if existing:
        st.error(f"Bin ID '{bin_id}' already exists. Please choose another.")
        return False
    bin_data = {
        "bin_id": bin_id,
        "location": location,
        "latitude": float(lat),
        "longitude": float(lon),
        "status": "Full"
    }
    bins_collection.insert_one(bin_data)
    return True

def mark_collected(bin_id):
    bins_collection.update_one({"bin_id": bin_id}, {"$set": {"status": "Collected"}})

def optimize_route(bins):
    coords = [(b["latitude"], b["longitude"]) for b in bins]
    n = len(coords)
    dist_matrix = [[0 if i == j else int(geodesic(coords[i], coords[j]).km * 1000)
                    for j in range(n)] for i in range(n)]

    manager = pywrapcp.RoutingIndexManager(n, 1, 0)
    routing = pywrapcp.RoutingModel(manager)

    def distance_callback(from_index, to_index):
        f, t = manager.IndexToNode(from_index), manager.IndexToNode(to_index)
        return dist_matrix[f][t]

    transit_callback_index = routing.RegisterTransitCallback(distance_callback)
    routing.SetArcCostEvaluatorOfAllVehicles(transit_callback_index)

    search_params = pywrapcp.DefaultRoutingSearchParameters()
    search_params.first_solution_strategy = routing_enums_pb2.FirstSolutionStrategy.PATH_CHEAPEST_ARC

    solution = routing.SolveWithParameters(search_params)
    if solution:
        route = []
        index = routing.Start(0)
        while not routing.IsEnd(index):
            route.append(manager.IndexToNode(index))
            index = solution.Value(routing.NextVar(index))
        return route
    return None

# -------------------------------
# Streamlit UI
# -------------------------------
st.set_page_config(page_title="Smart Waste Route Optimizer", layout="wide")
st.title("Waste Route Optimizer")

menu = st.sidebar.radio("Menu", ["View All Bins", "Add New Bin", "Mark Bin as Collected", "View Optimized Route"])

# -------------------------------
# View All Bins
# -------------------------------
if menu == "View All Bins":
    st.header("All Bins")
    bins = get_all_bins()
    if not bins:
        st.info("No bins found. Add a new bin from 'Add New Bin'.")
    else:
        df = pd.DataFrame(bins)
        show_cols = [c for c in ["bin_id", "location", "status"] if c in df.columns]
        st.dataframe(df[show_cols], use_container_width=True)

        full_bins = [b["bin_id"] for b in bins if b.get("status", "").lower() in ("full", "true", "1") or b.get("status","")=="Full"]
        if full_bins:
            chosen = st.selectbox("Select a Full Bin to mark collected", ["(none)"] + full_bins)
            if chosen and chosen != "(none)":
                if st.button("Mark Selected Bin as Collected"):
                    mark_collected(chosen)
                    st.success(f"Bin {chosen} marked as collected.")
                    st.rerun()

        coords_df = df.dropna(subset=["latitude", "longitude"])
        if not coords_df.empty:
            avg_lat = coords_df["latitude"].mean()
            avg_lng = coords_df["longitude"].mean()
            m = folium.Map(location=[avg_lat, avg_lng], zoom_start=13)
            for _, row in coords_df.iterrows():
                color = "green" if row.get("status","").lower() in ("collected","empty") else "red"
                folium.Marker(
                    [row["latitude"], row["longitude"]],
                    popup=f"ID: {row['bin_id']}<br>{row.get('location','') }<br>Status: {row.get('status','')}",
                    icon=folium.Icon(color=color)
                ).add_to(m)
            st.subheader("Map")
            st_folium(m, width=750, height=450)
        else:
            st.info("No bins have valid coordinates to map.")

# -------------------------------
# Add New Bin (draggable map + auto city start)
# -------------------------------
elif menu == "Add New Bin":
    st.header("Add New Bin (drag marker to set location)")
    bin_id = st.text_input("Bin ID")
    location_name = st.text_input("Landmark / Area Name (optional)")

    st.markdown("**Drag or click on the map to set the bin’s exact location.**")

    # Try detecting approximate city location via IP
    loc = geocoder.ip("me")
    if loc.ok and loc.latlng:
        default_lat, default_lon = loc.latlng
        city_text = f"Approximate starting location detected near **{loc.city or 'your area'}**"
    else:
        default_lat, default_lon = 20.5937, 78.9629
        city_text = "Default starting location: India (couldn't detect your city)"
    st.caption(city_text)

    # Initialize draggable map
    m = folium.Map(location=[default_lat, default_lon], zoom_start=12)
    marker = folium.Marker(
        [default_lat, default_lon],
        draggable=True,
        popup="Drag or click to set bin location",
        icon=folium.Icon(color="red")
    )
    marker.add_to(m)
    output = st_folium(m, width=700, height=450)

    lat, lon = None, None
    if output and output.get("last_clicked"):
        lat = output["last_clicked"]["lat"]
        lon = output["last_clicked"]["lng"]
        st.success(f"Selected coordinates — lat: {lat:.6f}, lon: {lon:.6f}")

    if st.button("Add Bin"):
        if not bin_id:
            st.error("Bin ID is required.")
        elif lat is None or lon is None:
            st.error("Please drag or click a location on the map.")
        else:
            success = add_bin(bin_id, location_name or "N/A", lat, lon)
            if success:
                st.success(f"Bin {bin_id} added successfully.")
                st.rerun()

# -------------------------------
# Mark Bin as Collected
# -------------------------------
elif menu == "Mark Bin as Collected":
    st.header("Mark Bin as Collected")
    bins = get_all_bins()
    if not bins:
        st.info("No bins available.")
    else:
        uncollected = [b["bin_id"] for b in bins if b.get("status","").lower() not in ("collected","empty")]
        if not uncollected:
            st.info("All bins appear collected.")
        else:
            sel = st.selectbox("Choose a bin to mark collected", uncollected)
            if st.button("Mark Collected"):
                mark_collected(sel)
                st.success(f"Bin {sel} marked as collected.")
                st.rerun()

# -------------------------------
# View Optimized Route
# -------------------------------
elif menu == "View Optimized Route":
    st.header("Optimized Route (Full bins only)")
    bins = [b for b in get_all_bins() if b.get("status","").lower() not in ("collected","empty")]
    if len(bins) < 2:
        st.info("At least two full bins are required to build a route.")
    else:
        route = optimize_route(bins)
        if route:
            ordered = [bins[i] for i in route]
            st.subheader("Visit Order:")
            st.write(" → ".join([b["bin_id"] for b in ordered]))

            coords = [[b["latitude"], b["longitude"]] for b in ordered]
            m = folium.Map(location=coords[0], zoom_start=13)
            for b in ordered:
                folium.Marker([b["latitude"], b["longitude"]],
                              popup=f"{b['bin_id']} ({b.get('location','')})",
                              icon=folium.Icon(color="blue")).add_to(m)
            folium.PolyLine(coords, color="blue", weight=3.5).add_to(m)
            st_folium(m, width=800, height=500)
        else:
            st.error("Could not compute optimized route. Try again.")
