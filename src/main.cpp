#include "CLI.h"
#include "MongoHandler.h"
#include <mongocxx/instance.hpp>
#include <iostream>

int main() {
    const char* uri = std::getenv("MONGODB_URI");
    if (!uri) {
        std::cerr << "Error: please set MONGODB_URI environment variable (Atlas connection string)\n";
        return 1;
    }
    mongocxx::instance inst{};
    std::string uri_s(uri);
    MongoHandler db(uri_s, "waste_db");
    CLI cli(db);
    cli.run();
    return 0;
}
