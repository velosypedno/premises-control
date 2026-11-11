#include "Database.h"
#include "../models/uuid.h"
#include "Config.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>


namespace fs = std::filesystem;

std::shared_ptr<sqlite3> Database::openConnection(const std::string& dbPath) {
    sqlite3* rawDb = nullptr;
    if (sqlite3_open(dbPath.c_str(), &rawDb) != SQLITE_OK) {
        std::string err = rawDb ? sqlite3_errmsg(rawDb) : "Unknown error";
        if (rawDb) sqlite3_close(rawDb);
        throw std::runtime_error("Failed to open database: " + err);
    }

    return std::shared_ptr<sqlite3>(rawDb, [](sqlite3* ptr) {
        if (ptr) {
            sqlite3_close(ptr);
            std::cout << "Database connection closed.\n";
        }
    });
}

void Database::initializeSchema(sqlite3* db, const std::string& schemaPath) {
    if (!fs::exists(schemaPath)) {
        throw std::runtime_error("Schema file not found at path: " + schemaPath);
    }

    std::ifstream file(schemaPath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open schema file: " + schemaPath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    char* errMsg = nullptr;
    if (sqlite3_exec(db, buffer.str().c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string error(errMsg);
        sqlite3_free(errMsg);
        throw std::runtime_error("SQL schema execution error: " + error);
    }

    std::cout << "Schema successfully initialized from file: " << schemaPath << '\n';
}

void Database::clear(sqlite3* db) {
    char* errMsg = nullptr;
    const char* clearSql =
        "DELETE FROM access_logs; "
        "DELETE FROM access_policy_windows; "
        "DELETE FROM cards; "
        "DELETE FROM users; "
        "DELETE FROM access_policies;";

    if (sqlite3_exec(db, clearSql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string error(errMsg ? errMsg : "Unknown SQL error");
        sqlite3_free(errMsg);
        throw std::runtime_error("Table clearing error: " + error);
    }

    std::cout << "All previous data from the database has been cleared.\n\n";
}

void Database::initializeRootAdmin(sqlite3* db, const std::string& passwordHash) {
    std::string rootId = uuid::generate();

    const char* sql =
        "INSERT INTO users (id, first_name, last_name, policy_id, role, username, password_hash) "
        "VALUES (?, 'Root', 'Admin', NULL, 'ADMIN', 'root', ?) "
        "ON CONFLICT(username) DO UPDATE SET password_hash = excluded.password_hash;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db));
    }

    sqlite3_bind_text(stmt, 1, rootId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, passwordHash.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error(sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}
