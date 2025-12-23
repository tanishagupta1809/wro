cmake_minimum_required(VERSION 3.14)
project(waste_route_optimizer_full LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# --- Find pybind11 ---
set(PYBIND11_FINDPYTHON ON)
execute_process(
    COMMAND python3 -m pybind11 --cmakedir
    OUTPUT_VARIABLE PYBIND11_CMAKE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
list(APPEND CMAKE_PREFIX_PATH ${PYBIND11_CMAKE_DIR})
find_package(pybind11 REQUIRED)

# --- Find Python ---
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# --- Tell CMake where to find MongoDB C++ drivers ---
set(bsoncxx_DIR "/usr/local/lib/cmake/bsoncxx-4.1.4")
set(mongocxx_DIR "/usr/local/lib/cmake/mongocxx-4.1.4")

# --- Find MongoDB drivers ---
find_package(bsoncxx REQUIRED)
find_package(mongocxx REQUIRED)

# --- Source files ---
set(SRC_FILES
    src/ASTAR.cpp
    src/astar_bindings.cpp
)
# --- Create C++ library (for A* optimizer) ---
add_library(astar_optimizer STATIC src/ASTAR.cpp)
target_include_directories(astar_optimizer PUBLIC ${CMAKE_SOURCE_DIR}/include)

# --- Pybind11 module ---
pybind11_add_module(waste_route_optimizer_full MODULE ${SRC_FILES})
target_include_directories(waste_route_optimizer_full PRIVATE ${CMAKE_SOURCE_DIR}/include)

find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)

target_link_libraries(waste_route_optimizer_full
    PRIVATE
    mongo::mongocxx_shared
    mongo::bsoncxx_shared
)


# --- Output path ---
set_target_properties(waste_route_optimizer_full PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build
)