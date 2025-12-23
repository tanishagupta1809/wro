#ifndef MONGOHANDLER_H
#define MONGOHANDLER_H

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <string>
#include <vector>

struct Bin {
    std::string id;
    double lat;
    double lng;
    bool is_full;
};

class MongoHandler {
public:
    MongoHandler(const std::string &uri_str, const std::string &db_name);
    void storeRoute(const std::vector<std::string> &route);
    void insertBin(const Bin &b);
    std::vector<Bin> getAllBins();
    std::vector<Bin> getFullBins();
    void markCollected(const std::string &id);
    std::vector<std::string> getOptimizedRoute();
    
private:
    mongocxx::client client_;
    mongocxx::database db_;
};

#endif
