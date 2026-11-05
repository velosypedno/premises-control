#include "AccessLogRecordRepo.h"
#include <stdexcept>

AccessLogRepo::AccessLogRepo(std::shared_ptr<sqlite3> dbConnection) : db(std::move(dbConnection)) {}

AccessLogRecord AccessLogRepo::extractRecord(sqlite3_stmt* stmt) {
    std::string id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    std::string timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    std::string cardId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    std::string eventTypeStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

    LogEventType eventType = AccessLogRecord::stringToEventType(eventTypeStr);

    return AccessLogRecord(std::move(id), std::move(timestamp), std::move(cardId), eventType);
}

void AccessLogRepo::create(const AccessLogRecord& record) {
    const char* sql = "INSERT INTO access_logs (id, timestamp, card_id, event_type) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    sqlite3_bind_text(stmt, 1, record.getId().data(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, record.getTimestamp().data(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, record.getCardId().data(), -1, SQLITE_TRANSIENT);
    
    std::string eventTypeStr = AccessLogRecord::eventTypeToString(record.getEventType());
    sqlite3_bind_text(stmt, 4, eventTypeStr.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    sqlite3_finalize(stmt);
}

std::optional<AccessLogRecord> AccessLogRepo::getById(const std::string& id) {
    const char* sql = "SELECT id, timestamp, card_id, event_type FROM access_logs WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<AccessLogRecord> record = std::nullopt;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        record.emplace(extractRecord(stmt));
    }

    sqlite3_finalize(stmt);
    return record;
}

std::vector<AccessLogRecord> AccessLogRepo::getByCardId(const std::string& cardId) {
    const char* sql = "SELECT id, timestamp, card_id, event_type FROM access_logs WHERE card_id = ? ORDER BY timestamp DESC;";
    sqlite3_stmt* stmt = nullptr;
    std::vector<AccessLogRecord> records;

    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    sqlite3_bind_text(stmt, 1, cardId.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        records.emplace_back(extractRecord(stmt));
    }

    sqlite3_finalize(stmt);
    return records;
}

std::vector<AccessLogRecord> AccessLogRepo::list() {
    const char* sql = "SELECT id, timestamp, card_id, event_type FROM access_logs ORDER BY timestamp DESC;";
    sqlite3_stmt* stmt = nullptr;
    std::vector<AccessLogRecord> records;

    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        records.emplace_back(extractRecord(stmt));
    }

    sqlite3_finalize(stmt);
    return records;
}