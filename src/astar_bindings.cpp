#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../include/AStar.h"

namespace py = pybind11;

PYBIND11_MODULE(waste_route_optimizer_full, m) {
    py::class_<AStar>(m, "AStar")
        .def(py::init<>())
        .def("addNode", &AStar::addNode)
        .def("addEdge", &AStar::addEdge)
        .def("a_star", &AStar::a_star);
}
