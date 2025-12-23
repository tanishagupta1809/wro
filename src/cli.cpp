#include "CLI.h"
#include <iostream>
#include <limits>
#include <algorithm>

CLI::CLI(MongoHandler &db): db_(db) {}

void CLI::run() { showMenu(); }

void CLI::showMenu() {
    while (true) {
        std::cout << "\n=== Smart Garbage Routing System ===\n";
        std::cout << "1. View all bins\n";
        std::cout << "2. View full bins only\n";
        std::cout << "3. Add a bin\n";
        std::cout << "4. Mark bin as collected\n";
        std::cout << "5. Optimize route\n";
        std::cout << "6. Exit\n";
        std::cout << "Enter choice: ";
        int ch; if (!(std::cin >> ch)) { std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); continue; }
        switch(ch) {
            case 1: viewAllBins(); break;
            case 2: viewFullBins(); break;
            case 3: addBin(); break;
            case 4: markCollected(); break;
            case 5: optimizeRoute(); break;
            case 6: std::cout << "Goodbye\n"; return;
            default: std::cout << "Invalid\n"; break;
        }
    }
}

void CLI::viewAllBins() {
    auto bins = db_.getAllBins();
    std::cout << "\n--- All bins ---\n";
    for (auto &b : bins) {
        std::cout << b.id << " : (" << b.lat << ", " << b.lng << ") - " << (b.is_full ? "FULL" : "EMPTY") << "\n";
    }
}

void CLI::viewFullBins() {
    auto bins = db_.getFullBins();
    std::cout << "\n--- Full bins ---\n";
    for (auto &b : bins) {
        std::cout << b.id << " : (" << b.lat << ", " << b.lng << ")\n";
    }
}

void CLI::addBin() {
    std::string id; double lat, lng; char fullch;
    std::cout << "Enter bin id: "; std::cin >> id;
    std::cout << "Enter lat lng: "; std::cin >> lat >> lng;
    std::cout << "Is full? (y/n): "; std::cin >> fullch;
    bool is_full = (fullch=='y' || fullch=='Y');
    Bin bin{id, lat, lng, is_full};
    db_.insertBin(bin);
    std::cout << "Inserted.\n";
}

void CLI::markCollected() {
    std::string id; std::cout << "Enter bin id to mark collected: "; std::cin >> id;
    db_.markCollected(id);
    std::cout << "Marked as collected.\n";
}

void CLI::optimizeRoute() {
    auto full = db_.getFullBins();
    if (full.empty()) { std::cout << "No full bins to optimize.\n"; return; }
    AStar astar;
    for (auto &b: full) astar.addNode(b.id, b.lat, b.lng);
    for (size_t i=0;i<full.size();++i) for (size_t j=i+1;j<full.size();++j) astar.addEdge(full[i].id, full[j].id);
    std::string start = full[0].id;
    std::vector<std::string> route;
    std::string current = start;
    std::vector<std::string> remaining; for (auto &b: full) remaining.push_back(b.id);
    while (!remaining.empty()) {
        double bestCost = 1e18; std::string bestId = remaining[0];
        for (auto &cand: remaining) {
            auto res = astar.a_star(current, cand);
            if (res.first < bestCost) { bestCost = res.first; bestId = cand; }
        }
        route.push_back(bestId);
        remaining.erase(std::remove(remaining.begin(), remaining.end(), bestId), remaining.end());
        current = bestId;
    }
    std::cout << "Planned route: \n";
    for (auto &r: route) std::cout << r << " -> "; std::cout << "END\n";
    db_.storeRoute(route);
}
