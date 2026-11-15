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
        case LogEventType::Alarm:            return "ALARM";
        case LogEventType::Cancelled:        return "CANCELLED";
        case LogEventType::CardDoesNotExist: return "CARD_DOES_NOT_EXIST";
        case LogEventType::TimeIsNotAllowed: return "TIME_IS_NOT_ALLOWED";
        case LogEventType::DayIsNotAllowed:  return "DAY_IS_NOT_ALLOWED";
        case LogEventType::PolicyNotSet:     return "POLICY_NOT_SET";
        case LogEventType::UserInactive:     return "USER_INACTIVE";
        case LogEventType::PersistFailed:    return "PERSIST_FAILED";
        case LogEventType::CardIssued:       return "CARD_ISSUED";
        case LogEventType::CardRevoked:      return "CARD_REVOKED";
        case LogEventType::PinChanged:       return "PIN_CHANGED";
        case LogEventType::CardUnlocked:     return "CARD_UNLOCKED";
    }
    return "BLOCKED";
}

LogEventType AccessLogRecord::stringToEventType(std::string_view str) {
    if (str == "ALLOWED")              return LogEventType::Allowed;
    if (str == "WRONG_PIN")            return LogEventType::WrongPin;
    if (str == "REVOKED_CARD")         return LogEventType::RevokedCard;
    if (str == "BLOCKED")              return LogEventType::Blocked;
    if (str == "ALARM")                return LogEventType::Alarm;
    if (str == "CANCELLED")            return LogEventType::Cancelled;
    if (str == "CARD_DOES_NOT_EXIST")  return LogEventType::CardDoesNotExist;
    if (str == "TIME_IS_NOT_ALLOWED")  return LogEventType::TimeIsNotAllowed;
    if (str == "DAY_IS_NOT_ALLOWED")   return LogEventType::DayIsNotAllowed;
    if (str == "POLICY_NOT_SET")       return LogEventType::PolicyNotSet;
    if (str == "USER_INACTIVE")        return LogEventType::UserInactive;
    if (str == "PERSIST_FAILED")       return LogEventType::PersistFailed;
    if (str == "CARD_ISSUED")          return LogEventType::CardIssued;
    if (str == "CARD_REVOKED")         return LogEventType::CardRevoked;
    if (str == "PIN_CHANGED")          return LogEventType::PinChanged;
    if (str == "CARD_UNLOCKED")        return LogEventType::CardUnlocked;

    return LogEventType::Blocked;
}