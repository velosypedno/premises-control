#pragma once

#include <vector>
#include <optional>
#include <string>
#include <memory>
#include <sqlite3.h>
#include "AccessLogRecord.h"

class AccessLogRecordRepo {
private:
    std::shared_ptr<sqlite3> db;

    static AccessLogRecord extractRecord(sqlite3_stmt* stmt);

public:
    explicit AccessLogRecordRepo(std::shared_ptr<sqlite3> dbConnection);

    void create(const AccessLogRecord& record);
    
    std::optional<AccessLogRecord> getById(const std::string& id);

    std::vector<AccessLogRecord> getByCardId(const std::string& cardId);
    std::vector<AccessLogRecord> list();
};