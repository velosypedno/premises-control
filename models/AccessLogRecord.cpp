#include "AccessLogRecord.h"
#include "uuid.h"
#include <stdexcept>

AccessLogRecord::AccessLogRecord(std::string id, std::string timestamp, std::string cardId, LogEventType eventType)
    : id(std::move(id)), timestamp(std::move(timestamp)), cardId(std::move(cardId)), eventType(eventType) {}

AccessLogRecord::AccessLogRecord(std::string timestamp, std::string cardId, LogEventType eventType)
    : id(uuid::generate()), timestamp(std::move(timestamp)), cardId(std::move(cardId)), eventType(eventType) {}

std::string_view AccessLogRecord::getId() const noexcept {
    return id;
}

std::string_view AccessLogRecord::getTimestamp() const noexcept {
    return timestamp;
}

std::string_view AccessLogRecord::getCardId() const noexcept {
    return cardId;
}

LogEventType AccessLogRecord::getEventType() const noexcept {
    return eventType;
}

std::string AccessLogRecord::eventTypeToString(LogEventType type) {
    switch (type) {
        case LogEventType::Allowed:          return "ALLOWED";
        case LogEventType::WrongPin:         return "WRONG_PIN";
        case LogEventType::RevokedCard:      return "REVOKED_CARD";
        case LogEventType::Blocked:          return "BLOCKED";
        case LogEventType::CardDoesNotExist: return "CARD_DOES_NOT_EXIST";
        case LogEventType::TimeIsNotAllowed: return "TIME_IS_NOT_ALLOWED";
        case LogEventType::DayIsNotAllowed:  return "DAY_IS_NOT_ALLOWED";
        case LogEventType::PolicyNotSet:     return "POLICY_NOT_SET";
    }
    return "BLOCKED";
}

LogEventType AccessLogRecord::stringToEventType(std::string_view str) {
    if (str == "ALLOWED")              return LogEventType::Allowed;
    if (str == "WRONG_PIN")            return LogEventType::WrongPin;
    if (str == "REVOKED_CARD")         return LogEventType::RevokedCard;
    if (str == "BLOCKED")              return LogEventType::Blocked;
    if (str == "CARD_DOES_NOT_EXIST")  return LogEventType::CardDoesNotExist;
    if (str == "TIME_IS_NOT_ALLOWED")  return LogEventType::TimeIsNotAllowed;
    if (str == "DAY_IS_NOT_ALLOWED")   return LogEventType::DayIsNotAllowed;
    if (str == "POLICY_NOT_SET")       return LogEventType::PolicyNotSet;
    
    return LogEventType::Blocked;
}