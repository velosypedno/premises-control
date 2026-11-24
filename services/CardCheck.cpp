#include "CardCheck.h"
#include <chrono>
#include <ctime>
#include <cstdio>
#include <iomanip>
#include <sstream>

namespace {

DayOfWeek tmWdayToDayOfWeek(int wday) {
    switch (wday) {
        case 1: return DayOfWeek::Monday;
        case 2: return DayOfWeek::Tuesday;
        case 3: return DayOfWeek::Wednesday;
        case 4: return DayOfWeek::Thursday;
        case 5: return DayOfWeek::Friday;
        case 6: return DayOfWeek::Saturday;
        case 0:
        default: return DayOfWeek::Sunday;
    }
}

int dayOfWeekToTmWday(DayOfWeek day) {
    switch (day) {
        case DayOfWeek::Monday:    return 1;
        case DayOfWeek::Tuesday:   return 2;
        case DayOfWeek::Wednesday: return 3;
        case DayOfWeek::Thursday:  return 4;
        case DayOfWeek::Friday:    return 5;
        case DayOfWeek::Saturday:  return 6;
        case DayOfWeek::Sunday:    return 0;
    }
    return 1;
}

std::string formatCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string formatSimulatedTimestamp(DayOfWeek targetDay, const std::string& timeHHMM) {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);

    int currentWday = tm.tm_wday;
    int targetWday = dayOfWeekToTmWday(targetDay);
    int dayDiff = targetWday - currentWday;

    tm.tm_mday += dayDiff;
    std::mktime(&tm);

    char dateBuf[16];
    std::strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d", &tm);
    return std::string(dateBuf) + " " + timeHHMM + ":00";
}

std::pair<DayOfWeek, std::string> getCurrentDayAndTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);

    char buf[6];
    std::snprintf(buf, sizeof(buf), "%02d:%02d", tm.tm_hour, tm.tm_min);
    return {tmWdayToDayOfWeek(tm.tm_wday), std::string(buf)};
}

LogEventType toLogEvent(AuthError err) {
    switch (err) {
        case AuthError::WrongPin:         return LogEventType::WrongPin;
        case AuthError::Blocked:          return LogEventType::Blocked;
        case AuthError::Alarm:            return LogEventType::Alarm;
        case AuthError::RevokedCard:      return LogEventType::RevokedCard;
        case AuthError::CardDoesNotExist: return LogEventType::CardDoesNotExist;
        case AuthError::PolicyNotSet:     return LogEventType::PolicyNotSet;
        case AuthError::TimeIsNotAllowed: return LogEventType::TimeIsNotAllowed;
        case AuthError::DayIsNotAllowed:  return LogEventType::DayIsNotAllowed;
        case AuthError::UserInactive:     return LogEventType::UserInactive;
        case AuthError::PersistFailed:    return LogEventType::PersistFailed;
    }
    return LogEventType::PersistFailed;
}

}

std::string toErrorMessage(AuthError err) {
    switch (err) {
        case AuthError::WrongPin:         return "Incorrect PIN.";
        case AuthError::Blocked:          return "Card is blocked.";
        case AuthError::Alarm:            return "Lockdown and alarm triggered.";
        case AuthError::RevokedCard:      return "Card has been revoked.";
        case AuthError::CardDoesNotExist: return "Unknown card.";
        case AuthError::PolicyNotSet:     return "Access schedule is not assigned.";
        case AuthError::TimeIsNotAllowed: return "Outside allowed schedule hours.";
        case AuthError::DayIsNotAllowed:  return "Day is not permitted by schedule.";
        case AuthError::UserInactive:     return "User is inactive.";
        case AuthError::PersistFailed:    return "Failed to persist state.";
    }
    return "Access denied.";
}

CardCheck::CardCheck(
    std::shared_ptr<CardRepo> cardRepo,
    std::shared_ptr<UserRepo> userRepo,
    std::shared_ptr<AccessPolicyRepo> policyRepo,
    std::shared_ptr<AccessLogRecordRepo> logRepo,
    std::shared_ptr<CryptoService> cryptoService,
    int maxFailedAttempts
)
    : cardRepo_(std::move(cardRepo)),
      userRepo_(std::move(userRepo)),
      policyRepo_(std::move(policyRepo)),
      logRepo_(std::move(logRepo)),
      cryptoService_(std::move(cryptoService)),
      maxFailedAttempts_(maxFailedAttempts) {}

void CardCheck::recordAudit(const std::string& timestamp, const std::string& cardId, LogEventType eventType) const {
    AccessLogRecord record(timestamp, cardId, eventType);
    logRepo_->create(record);
}

void CardCheck::logCancellation(const std::string& cardId) const {
    try {
        recordAudit(formatCurrentTimestamp(), cardId.empty() ? "-" : cardId, LogEventType::Cancelled);
    } catch (...) {
        // Best-effort audit logging on UI exit
    }
}

Services::Result<AuthSuccess, AuthError> CardCheck::authenticate(
    const std::string& cardId,
    const std::string& pin
) const {
    auto [day, time] = getCurrentDayAndTime();
    return authenticateAt(cardId, pin, day, time, formatCurrentTimestamp());
}

Services::Result<AuthSuccess, AuthError> CardCheck::authenticate(
    const std::string& cardId,
    const std::string& pin,
    DayOfWeek day,
    const std::string& time
) const {
    return authenticateAt(cardId, pin, day, time, formatSimulatedTimestamp(day, time));
}

Services::Result<AuthSuccess, AuthError> CardCheck::authenticateAt(
    const std::string& cardId,
    const std::string& pin,
    DayOfWeek day,
    const std::string& time,
    const std::string& auditTimestamp
) const {
    auto fail = [&](AuthError err) -> Services::Result<AuthSuccess, AuthError> {
        recordAudit(auditTimestamp, cardId, toLogEvent(err));
        return Services::Err(err);
    };

    try {
        auto cardOpt = cardRepo_->getByCardId(cardId);
        if (!cardOpt.has_value()) {
            return fail(AuthError::CardDoesNotExist);
        }

        Card card = std::move(*cardOpt);

        if (card.getStatus() == CardStatus::Revoked) {
            return fail(AuthError::RevokedCard);
        }
        if (card.getStatus() == CardStatus::Blocked) {
            return fail(AuthError::Blocked);
        }

        if (cryptoService_->hashPin(pin) != card.getPinHash()) {
            card.incrementFailedAttempts();
            if (card.getFailedAttempts() >= maxFailedAttempts_) {
                card.setStatus(CardStatus::Blocked);
                cardRepo_->update(card);
                return fail(AuthError::Alarm);
            }
            cardRepo_->update(card);
            return fail(AuthError::WrongPin);
        }

        if (card.getFailedAttempts() > 0) {
            card.resetFailedAttempts();
            cardRepo_->update(card);
        }

        auto userOpt = userRepo_->getById(std::string(card.getUserId()));
        if (!userOpt.has_value()) {
            return fail(AuthError::CardDoesNotExist);
        }

        User user = std::move(*userOpt);
        if (!user.isActive()) {
            return fail(AuthError::UserInactive);
        }

        const auto& policyIdOpt = user.getPolicyIdOpt();
        if (!policyIdOpt.has_value()) {
            return fail(AuthError::PolicyNotSet);
        }

        auto policyOpt = policyRepo_->getById(*policyIdOpt);
        if (!policyOpt.has_value()) {
            return fail(AuthError::PolicyNotSet);
        }

        if (!policyOpt->isAllowed(day, time)) {
            bool dayHasWindow = false;
            for (const auto& window : policyOpt->getWindows()) {
                if (window.getDay() == day) {
                    dayHasWindow = true;
                    break;
                }
            }
            return fail(dayHasWindow ? AuthError::TimeIsNotAllowed : AuthError::DayIsNotAllowed);
        }

        recordAudit(auditTimestamp, cardId, LogEventType::Allowed);
        return Services::Ok(AuthSuccess{std::move(card), std::move(user)});
    } catch (const std::exception&) {
        return Services::Err(AuthError::PersistFailed);
    }
}
