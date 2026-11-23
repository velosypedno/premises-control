#include "CardCheck.h"
#include <chrono>
#include <ctime>
#include <cstdio>
#include <iomanip>
#include <sstream>

namespace {

std::string formatCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::pair<DayOfWeek, std::string> getCurrentDayAndTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);

    DayOfWeek day;
    switch (tm.tm_wday) {
        case 1: day = DayOfWeek::Monday; break;
        case 2: day = DayOfWeek::Tuesday; break;
        case 3: day = DayOfWeek::Wednesday; break;
        case 4: day = DayOfWeek::Thursday; break;
        case 5: day = DayOfWeek::Friday; break;
        case 6: day = DayOfWeek::Saturday; break;
        case 0:
        default: day = DayOfWeek::Sunday; break;
    }

    char buf[6];
    std::snprintf(buf, sizeof(buf), "%02d:%02d", tm.tm_hour, tm.tm_min);
    return {day, std::string(buf)};
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

void CardCheck::recordAudit(const std::string& cardId, LogEventType eventType) const {
    AccessLogRecord record(formatCurrentTimestamp(), cardId, eventType);
    logRepo_->create(record);
}

void CardCheck::logCancellation(const std::string& cardId) const {
    try {
        recordAudit(cardId.empty() ? "-" : cardId, LogEventType::Cancelled);
    } catch (...) {
        // Best-effort audit logging on UI exit
    }
}

Services::Result<AuthSuccess, AuthError> CardCheck::authenticate(
    const std::string& cardId,
    const std::string& pin
) const {
    auto [day, time] = getCurrentDayAndTime();
    return authenticate(cardId, pin, day, time);
}

Services::Result<AuthSuccess, AuthError> CardCheck::authenticate(
    const std::string& cardId,
    const std::string& pin,
    DayOfWeek day,
    const std::string& time
) const {
    auto fail = [&](AuthError err) -> Services::Result<AuthSuccess, AuthError> {
        recordAudit(cardId, toLogEvent(err));
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

        recordAudit(cardId, LogEventType::Allowed);
        return Services::Ok(AuthSuccess{std::move(card), std::move(user)});
    } catch (const std::exception&) {
        return Services::Err(AuthError::PersistFailed);
    }
}
