#include "CardCheck.h"
#include <chrono>
#include <iomanip>
#include <sstream>

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

std::string CardCheck::eventTypeToMessage(LogEventType event) {
    switch (event) {
        case LogEventType::Allowed: return "Access granted.";
        case LogEventType::WrongPin: return "Incorrect PIN code.";
        case LogEventType::Blocked: return "Card is blocked.";
        case LogEventType::RevokedCard: return "Card is revoked.";
        case LogEventType::CardDoesNotExist: return "Card does not exist.";
        case LogEventType::PolicyNotSet: return "No access policy assigned to user.";
        case LogEventType::TimeIsNotAllowed: return "Access denied: outside allowed schedule slot.";
    }
    return "Unknown event.";
}

bool CardCheck::authenticate(const std::string& cardId, const std::string& pin, DayOfWeek currentDay, const std::string& currentTime) {
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
            logEvent(cardId, LogEventType::Blocked);
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
        logEvent(cardId, LogEventType::TimeIsNotAllowed);
        return false;
    }

    logEvent(cardId, LogEventType::Allowed);
    return true;
}
