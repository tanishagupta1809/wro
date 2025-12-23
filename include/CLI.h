#pragma once
#include "MongoHandler.h"
#include "AStar.h"
#include <string>
#include <vector>

class CLI {
public:
    CLI(MongoHandler &db);
    void run();
private:
    MongoHandler &db_;
    void showMenu();
    void viewAllBins();
    void viewFullBins();
    void addBin();
    void markCollected();
    void optimizeRoute();
};
