#pragma once

#include <string>
#include <memory>
#include <sqlite3.h>

class Database {
public:
    static std::shared_ptr<sqlite3> openConnection(const std::string& dbPath);
    static void initializeSchema(sqlite3* db, const std::string& schemaPath);
    static void clear(sqlite3* db);
    static void initializeRootAdmin(sqlite3* db, const std::string& passwordHash);
};
