#pragma once

#include <string>
#include <string_view>

enum class LogEventType {
    Allowed,
    WrongPin,
    RevokedCard,
    Blocked,
    CardDoesNotExist,
    TimeIsNotAllowed,
    DayIsNotAllowed,
    PolicyNotSet
};

class AccessLogRecord {
private:
    std::string id;
    std::string timestamp;
    std::string cardId;
    LogEventType eventType;

public:
    AccessLogRecord() = default;
    AccessLogRecord(std::string id, std::string timestamp, std::string cardId, LogEventType eventType);
    AccessLogRecord(std::string timestamp, std::string cardId, LogEventType eventType);

    AccessLogRecord(const AccessLogRecord&) = default;
    AccessLogRecord& operator=(const AccessLogRecord&) = default;
    AccessLogRecord(AccessLogRecord&&) noexcept = default;
    AccessLogRecord& operator=(AccessLogRecord&&) noexcept = default;

    ~AccessLogRecord() = default;

    std::string_view getId() const noexcept;
    std::string_view getTimestamp() const noexcept;
    std::string_view getCardId() const noexcept;
    LogEventType getEventType() const noexcept;

    static std::string eventTypeToString(LogEventType type);
    static LogEventType stringToEventType(std::string_view str);
};