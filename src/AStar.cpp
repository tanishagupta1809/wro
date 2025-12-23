#include "AStar.h"
#include <queue>
#include <unordered_map>
#include <limits>
#include <cmath>
#include <algorithm>

double AStar::heuristic(const std::string &a, const std::string &b) {
    auto &na = nodes_[a]; auto &nb = nodes_[b];
    double R = 6371000.0;
    auto deg2rad = [](double d){ return d*M_PI/180.0; };
    double dLat = deg2rad(nb.lat - na.lat); double dLon = deg2rad(nb.lng - na.lng);
    double A = sin(dLat/2)*sin(dLat/2) + cos(deg2rad(na.lat))*cos(deg2rad(nb.lat))*sin(dLon/2)*sin(dLon/2);
    double C = 2*atan2(sqrt(A), sqrt(1-A));
    return R*C;
}

double AStar::distance(const std::string &a, const std::string &b) { return heuristic(a,b); }

void AStar::addNode(const std::string &id, double lat, double lng) { nodes_[id] = {lat,lng}; }

void AStar::addEdge(const std::string &a, const std::string &b) {
    adj_[a].push_back(b); adj_[b].push_back(a);
}

std::pair<double, std::vector<std::string>> AStar::a_star(const std::string &src, const std::string &target) {
    using P = std::pair<double, std::string>;
    std::priority_queue<P, std::vector<P>, std::greater<P>> open;
    std::unordered_map<std::string,double> gscore, fscore;
    std::unordered_map<std::string,std::string> came;
    for (auto &kv: nodes_) { gscore[kv.first] = std::numeric_limits<double>::infinity(); fscore[kv.first] = std::numeric_limits<double>::infinity(); }
    gscore[src]=0; fscore[src]=heuristic(src,target); open.push({fscore[src], src});
    while(!open.empty()){
        auto [f,u]=open.top(); open.pop();
        if(u==target){
            std::vector<std::string> path; std::string cur=target;
            while(came.find(cur)!=came.end()){ path.push_back(cur); cur=came[cur]; }
            path.push_back(src); std::reverse(path.begin(), path.end());
            return {gscore[target], path};
        }
        for(auto &v: adj_[u]){
            double tentative = gscore[u] + distance(u,v);
            if(tentative < gscore[v]){
                came[v]=u; gscore[v]=tentative; fscore[v]=tentative + heuristic(v,target);
                open.push({fscore[v], v});
            }
        }
    }
    return {std::numeric_limits<double>::infinity(), {}};
}
