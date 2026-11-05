#include "UserRepo.h"
#include <stdexcept>

UserRepo::UserRepo(std::shared_ptr<sqlite3> dbConnection) : db(std::move(dbConnection)) {}

User UserRepo::extractUser(sqlite3_stmt* stmt) {
    std::string id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    std::string firstName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    std::string lastName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

    std::optional<std::string> policyId = std::nullopt;
    if (sqlite3_column_type(stmt, 3) != SQLITE_NULL) {
        policyId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    }

    return User(std::move(id), std::move(firstName), std::move(lastName), std::move(policyId));
}

void UserRepo::create(const User& user) {
    const char* sql = "INSERT INTO users (id, first_name, last_name, policy_id) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    sqlite3_bind_text(stmt, 1, user.getId().data(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user.getFirstName().data(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user.getLastName().data(), -1, SQLITE_TRANSIENT);

    const auto& policyOpt = user.getPolicyIdOpt();
    if (policyOpt.has_value()) {
        sqlite3_bind_text(stmt, 4, policyOpt->data(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, 4);
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    sqlite3_finalize(stmt);
}

std::optional<User> UserRepo::getById(const std::string& id) {
    const char* sql = "SELECT id, first_name, last_name, policy_id FROM users WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<User> user = std::nullopt;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user.emplace(extractUser(stmt));
    }

    sqlite3_finalize(stmt);
    return user;
}

void UserRepo::update(const User& user) {
    const char* sql = "UPDATE users SET first_name = ?, last_name = ?, policy_id = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    sqlite3_bind_text(stmt, 1, user.getFirstName().data(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user.getLastName().data(), -1, SQLITE_TRANSIENT);

    const auto& policyOpt = user.getPolicyIdOpt();
    if (policyOpt.has_value()) {
        sqlite3_bind_text(stmt, 3, policyOpt->data(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, 3);
    }

    sqlite3_bind_text(stmt, 4, user.getId().data(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    sqlite3_finalize(stmt);
}

void UserRepo::remove(const std::string& id) {
    const char* sql = "DELETE FROM users WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    sqlite3_finalize(stmt);
}

std::vector<User> UserRepo::list() {
    const char* sql = "SELECT id, first_name, last_name, policy_id FROM users;";
    sqlite3_stmt* stmt = nullptr;
    std::vector<User> users;

    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        users.emplace_back(extractUser(stmt));
    }

    sqlite3_finalize(stmt);
    return users;
}