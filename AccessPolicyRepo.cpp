#include "AccessPolicyRepo.h"
#include <stdexcept>

AccessPolicyRepo::AccessPolicyRepo(std::shared_ptr<sqlite3> dbConnection) : db(std::move(dbConnection)) {}

std::vector<AccessWindow> AccessPolicyRepo::loadWindowsForPolicy(const std::string& policyId) {
    const char* sql = "SELECT id, policy_id, day_of_week, from_time, to_time FROM access_policy_windows WHERE policy_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    std::vector<AccessWindow> windows;

    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    sqlite3_bind_text(stmt, 1, policyId.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string winId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string polId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string dayStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::string fromTime = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        std::string toTime = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

        DayOfWeek day = AccessWindow::stringToDay(dayStr);
        windows.emplace_back(std::move(winId), std::move(polId), day, std::move(fromTime), std::move(toTime));
    }

    sqlite3_finalize(stmt);
    return windows;
}

void AccessPolicyRepo::create(const AccessPolicy& policy) {
    char* errMsg = nullptr;
    if (sqlite3_exec(db.get(), "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string err(errMsg);
        sqlite3_free(errMsg);
        throw std::runtime_error("Не вдалося почати транзакцію: " + err);
    }

    try {
        const char* policySql = "INSERT INTO access_policies (id, name) VALUES (?, ?);";
        sqlite3_stmt* stmt = nullptr;

        if (sqlite3_prepare_v2(db.get(), policySql, -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error(sqlite3_errmsg(db.get()));
        }

        sqlite3_bind_text(stmt, 1, policy.getId().data(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, policy.getName().data(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error(sqlite3_errmsg(db.get()));
        }
        sqlite3_finalize(stmt);

        const char* windowSql = "INSERT INTO access_policy_windows (id, policy_id, day_of_week, from_time, to_time) VALUES (?, ?, ?, ?, ?);";
        
        for (const auto& window : policy.getWindows()) {
            if (sqlite3_prepare_v2(db.get(), windowSql, -1, &stmt, nullptr) != SQLITE_OK) {
                throw std::runtime_error(sqlite3_errmsg(db.get()));
            }

            sqlite3_bind_text(stmt, 1, window.getId().data(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, policy.getId().data(), -1, SQLITE_TRANSIENT);
            
            std::string dayStr = AccessWindow::dayToString(window.getDay());
            sqlite3_bind_text(stmt, 3, dayStr.c_str(), -1, SQLITE_TRANSIENT);
            
            sqlite3_bind_text(stmt, 4, window.getFromTime().data(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 5, window.getToTime().data(), -1, SQLITE_TRANSIENT);

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                sqlite3_finalize(stmt);
                throw std::runtime_error(sqlite3_errmsg(db.get()));
            }
            sqlite3_finalize(stmt);
        }

        sqlite3_exec(db.get(), "COMMIT;", nullptr, nullptr, nullptr);
    } catch (...) {
        sqlite3_exec(db.get(), "ROLLBACK;", nullptr, nullptr, nullptr);
        throw;
    }
}

std::optional<AccessPolicy> AccessPolicyRepo::getById(const std::string& id) {
    const char* sql = "SELECT id, name FROM access_policies WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<AccessPolicy> policy = std::nullopt;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string polId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        sqlite3_finalize(stmt);

        std::vector<AccessWindow> windows = loadWindowsForPolicy(polId);
        policy.emplace(std::move(polId), std::move(name), std::move(windows));
        return policy;
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::optional<AccessPolicy> AccessPolicyRepo::getByUserId(const std::string& userId) {
    const char* sql = "SELECT p.id, p.name FROM access_policies p JOIN users u ON u.policy_id = p.id WHERE u.id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<AccessPolicy> policy = std::nullopt;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string polId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        sqlite3_finalize(stmt);

        std::vector<AccessWindow> windows = loadWindowsForPolicy(polId);
        policy.emplace(std::move(polId), std::move(name), std::move(windows));
        return policy;
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

void AccessPolicyRepo::update(const AccessPolicy& policy) {
    char* errMsg = nullptr;
    if (sqlite3_exec(db.get(), "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string err(errMsg);
        sqlite3_free(errMsg);
        throw std::runtime_error("Не вдалося почати транзакцію: " + err);
    }

    try {
        const char* updateSql = "UPDATE access_policies SET name = ? WHERE id = ?;";
        sqlite3_stmt* stmt = nullptr;

        if (sqlite3_prepare_v2(db.get(), updateSql, -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error(sqlite3_errmsg(db.get()));
        }

        sqlite3_bind_text(stmt, 1, policy.getName().data(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, policy.getId().data(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error(sqlite3_errmsg(db.get()));
        }
        sqlite3_finalize(stmt);

        const char* deleteWindowsSql = "DELETE FROM access_policy_windows WHERE policy_id = ?;";
        if (sqlite3_prepare_v2(db.get(), deleteWindowsSql, -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error(sqlite3_errmsg(db.get()));
        }
        sqlite3_bind_text(stmt, 1, policy.getId().data(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error(sqlite3_errmsg(db.get()));
        }
        sqlite3_finalize(stmt);

        const char* windowSql = "INSERT INTO access_policy_windows (id, policy_id, day_of_week, from_time, to_time) VALUES (?, ?, ?, ?, ?);";
        
        for (const auto& window : policy.getWindows()) {
            if (sqlite3_prepare_v2(db.get(), windowSql, -1, &stmt, nullptr) != SQLITE_OK) {
                throw std::runtime_error(sqlite3_errmsg(db.get()));
            }

            sqlite3_bind_text(stmt, 1, window.getId().data(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, policy.getId().data(), -1, SQLITE_TRANSIENT);
            
            std::string dayStr = AccessWindow::dayToString(window.getDay());
            sqlite3_bind_text(stmt, 3, dayStr.c_str(), -1, SQLITE_TRANSIENT);
            
            sqlite3_bind_text(stmt, 4, window.getFromTime().data(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 5, window.getToTime().data(), -1, SQLITE_TRANSIENT);

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                sqlite3_finalize(stmt);
                throw std::runtime_error(sqlite3_errmsg(db.get()));
            }
            sqlite3_finalize(stmt);
        }

        sqlite3_exec(db.get(), "COMMIT;", nullptr, nullptr, nullptr);
    } catch (...) {
        sqlite3_exec(db.get(), "ROLLBACK;", nullptr, nullptr, nullptr);
        throw;
    }
}

void AccessPolicyRepo::remove(const std::string& id) {
    const char* sql = "DELETE FROM access_policies WHERE id = ?;";
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

std::vector<AccessPolicy> AccessPolicyRepo::list() {
    const char* sql = "SELECT id, name FROM access_policies;";
    sqlite3_stmt* stmt = nullptr;
    std::vector<AccessPolicy> policies;

    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string polId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        std::vector<AccessWindow> windows = loadWindowsForPolicy(polId);
        policies.emplace_back(std::move(polId), std::move(name), std::move(windows));
    }

    sqlite3_finalize(stmt);
    return policies;
}