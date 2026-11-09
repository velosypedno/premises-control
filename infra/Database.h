#pragma once

#include <string>
#include <memory>
#include <sqlite3.h>

struct Config {
    std::string dbPath;
    std::string schemaPath;

    static Config loadFromEnv();
};

class Database {
public:
    static std::shared_ptr<sqlite3> openConnection(const std::string& dbPath);
    static void initializeSchema(sqlite3* db, const std::string& schemaPath);
    static void clear(sqlite3* db);
};
