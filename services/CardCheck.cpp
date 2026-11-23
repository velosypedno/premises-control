#include "CardCheck.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <stdexcept>

CardCheck::CardCheck(
    std::shared_ptr<CardRepo> cardRepo,
    std::shared_ptr<UserRepo> userRepo,
    std::shared_ptr<AccessPolicyRepo> policyRepo,
    std::shared_ptr<AccessLogRecordRepo> logRepo,
    std::shared_ptr<CryptoService> cryptoService,
    int maxFailedAttempts
)
    : cardRepo(std::move(cardRepo)),
      userRepo(std::move(userRepo)),
      policyRepo(std::move(policyRepo)),
      logRepo(std::move(logRepo)),
      cryptoService(std::move(cryptoService)),
      maxFailedAttempts(maxFailedAttempts) {}

std::string CardCheck::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void CardCheck::logEvent(const std::string& cardId, LogEventType eventType) {
    lastEventType = eventType;
    AccessLogRecord record(getCurrentTimestamp(), cardId, eventType);
    logRepo->create(record);
}

void CardCheck::logCancellation(const std::string& cardId) {
    try {
        std::string id = cardId.empty() ? "-" : cardId;
        logEvent(id, LogEventType::Cancelled);
    } catch (...) {
        lastEventType = LogEventType::Cancelled;
    }
}

std::string CardCheck::eventTypeToMessage(LogEventType event) {
    switch (event) {
        case LogEventType::Allowed: return "Access granted. Door unlocked [simulation].";
        case LogEventType::WrongPin: return "Incorrect PIN.";
        case LogEventType::Blocked: return "Card is blocked.";
        case LogEventType::Alarm: return "Lockdown and alarm triggered.";
        case LogEventType::Cancelled: return "Input cancelled.";
        case LogEventType::RevokedCard: return "Card has been revoked.";
        case LogEventType::CardDoesNotExist: return "Unknown card.";
        case LogEventType::PolicyNotSet: return "Access schedule is not assigned.";
        case LogEventType::TimeIsNotAllowed: return "Outside allowed schedule hours.";
        case LogEventType::DayIsNotAllowed: return "Day is not permitted by schedule.";
        case LogEventType::UserInactive: return "User is inactive.";
        case LogEventType::PersistFailed: return "Failed to persist state.";
        case LogEventType::CardIssued: return "Card issued.";
        case LogEventType::CardRevoked: return "Card revoked by administrator.";
        case LogEventType::PinChanged: return "PIN changed.";
        case LogEventType::CardUnlocked: return "Card unlocked.";
    }
    return "Access denied.";
}

bool CardCheck::authenticate(const std::string& cardId, const std::string& pin, DayOfWeek currentDay, const std::string& currentTime) {
    try {
        auto cardOpt = cardRepo->getByCardId(cardId);
        if (!cardOpt.has_value()) {
            logEvent(cardId, LogEventType::CardDoesNotExist);
            return false;
        }

        Card card = cardOpt.value();

        if (card.getStatus() == CardStatus::Revoked) {
            logEvent(cardId, LogEventType::RevokedCard);
            return false;
        }

        if (card.getStatus() == CardStatus::Blocked) {
            logEvent(cardId, LogEventType::Blocked);
            return false;
        }

        std::string inputHash = cryptoService->hashPin(pin);

        if (inputHash != card.getPinHash()) {
            card.incrementFailedAttempts();

            if (card.getFailedAttempts() >= maxFailedAttempts) {
                card.setStatus(CardStatus::Blocked);
                cardRepo->update(card);
                logEvent(cardId, LogEventType::Alarm);
            } else {
                cardRepo->update(card);
                logEvent(cardId, LogEventType::WrongPin);
            }
            return false;
        }

        if (card.getFailedAttempts() > 0) {
            card.resetFailedAttempts();
            cardRepo->update(card);
        }

        auto userOpt = userRepo->getById(std::string(card.getUserId()));
        if (!userOpt.has_value()) {
            logEvent(cardId, LogEventType::CardDoesNotExist);
            return false;
        }

        User user = userOpt.value();
        if (!user.isActive()) {
            logEvent(cardId, LogEventType::UserInactive);
            return false;
        }

        const auto& policyIdOpt = user.getPolicyIdOpt();
        if (!policyIdOpt.has_value()) {
            logEvent(cardId, LogEventType::PolicyNotSet);
            return false;
        }

        auto policyOpt = policyRepo->getById(policyIdOpt.value());
        if (!policyOpt.has_value()) {
            logEvent(cardId, LogEventType::PolicyNotSet);
            return false;
        }

        AccessPolicy policy = policyOpt.value();

        if (!policy.isAllowed(currentDay, currentTime)) {
            bool dayHasWindow = false;
            for (const auto& window : policy.getWindows()) {
                if (window.getDay() == currentDay) {
                    dayHasWindow = true;
                    break;
                }
            }
            logEvent(cardId, dayHasWindow ? LogEventType::TimeIsNotAllowed : LogEventType::DayIsNotAllowed);
            return false;
        }

        logEvent(cardId, LogEventType::Allowed);
        return true;
    } catch (const std::exception&) {
        lastEventType = LogEventType::PersistFailed;
        return false;
    }
}
