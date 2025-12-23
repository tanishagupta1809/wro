#include "../include/MongoHandler.h"
#include <iostream>
#include <vector>

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/json.hpp>

using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::kvp;

MongoHandler::MongoHandler(const std::string &uri_str, const std::string &db_name)
    : client_(mongocxx::uri(uri_str)) {
    db_ = client_[db_name];
}

void MongoHandler::insertBin(const Bin &b) {
    auto bins = db_["bins"];
    auto doc = make_document(
        kvp("id", b.id),
        kvp("lat", b.lat),
        kvp("lng", b.lng),
        kvp("is_full", b.is_full)
    );
    bins.insert_one(doc.view());
}

void MongoHandler::storeRoute(const std::vector<std::string> &route) {
    auto arr_builder = bsoncxx::builder::basic::array{};
    for (const auto &r : route) {
        arr_builder.append(r);
    }

    auto doc = make_document(
        kvp("route", arr_builder)
    );

    db_["routes"].insert_one(doc.view());
}

std::vector<Bin> MongoHandler::getAllBins() {
    std::vector<Bin> bins;
    auto cursor = db_["bins"].find({});
    for (auto &&doc : cursor) {
        Bin b;
        b.id = std::string(doc["id"].get_string().value);
        b.lat = doc["lat"].get_double().value;
        b.lng = doc["lng"].get_double().value;
        b.is_full = doc["is_full"].get_bool().value;
        bins.push_back(b);
    }
    return bins;
}

std::vector<Bin> MongoHandler::getFullBins() {
    std::vector<Bin> bins;
    auto filter = make_document(kvp("is_full", true));
    auto cursor = db_["bins"].find(filter.view());
    for (auto &&doc : cursor) {
        Bin b;
        b.id = std::string(doc["id"].get_string().value);
        b.lat = doc["lat"].get_double().value;
        b.lng = doc["lng"].get_double().value;
        b.is_full = true;
        bins.push_back(b);
    }
    return bins;
}

void MongoHandler::markCollected(const std::string &id) {
    auto filter = make_document(kvp("id", id));
    auto update = make_document(kvp("$set", make_document(kvp("is_full", false))));
    db_["bins"].update_one(filter.view(), update.view());
}

std::vector<std::string> MongoHandler::getOptimizedRoute() {
    // Dummy route for now
    return {"Bin1", "Bin2", "Bin3"};
}
