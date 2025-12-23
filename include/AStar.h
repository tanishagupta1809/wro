#pragma once
#include <string>
#include <unordered_map>
#include <vector>

class AStar {
public:
    void addNode(const std::string &id, double lat, double lng);
    void addEdge(const std::string &a, const std::string &b);
    std::pair<double, std::vector<std::string>> a_star(const std::string &src, const std::string &target);
private:
    struct Node { double lat,lng; };
    std::unordered_map<std::string, Node> nodes_;
    std::unordered_map<std::string, std::vector<std::string>> adj_;
    double heuristic(const std::string &a, const std::string &b);
    double distance(const std::string &a, const std::string &b);
};
