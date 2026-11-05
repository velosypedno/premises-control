#include "Card.h"
#include "uuid.h"
#include <stdexcept>

Card::Card(std::string id, std::string userId, std::string cardId, 
           std::string pinHash, CardStatus status, int failedAttempts)
    : id(std::move(id)), userId(std::move(userId)), cardId(std::move(cardId)),
      pinHash(std::move(pinHash)), status(status), failedAttempts(failedAttempts) {}

Card::Card(std::string userId, std::string cardId, std::string pinHash)
    : id(uuid::generate()), userId(std::move(userId)), cardId(std::move(cardId)),
      pinHash(std::move(pinHash)), status(CardStatus::Active), failedAttempts(0) {}

std::string_view Card::getId() const noexcept {
    return id;
}

std::string_view Card::getUserId() const noexcept {
    return userId;
}

std::string_view Card::getCardId() const noexcept {
    return cardId;
}

std::string_view Card::getPinHash() const noexcept {
    return pinHash;
}

CardStatus Card::getStatus() const noexcept {
    return status;
}

int Card::getFailedAttempts() const noexcept {
    return failedAttempts;
}

void Card::setStatus(CardStatus newStatus) noexcept {
    status = newStatus;
}

void Card::setPinHash(std::string newPinHash) {
    pinHash = std::move(newPinHash);
}

void Card::incrementFailedAttempts() noexcept {
    failedAttempts++;
}

void Card::resetFailedAttempts() noexcept {
    failedAttempts = 0;
}

std::string Card::statusToString(CardStatus status) {
    switch (status) {
        case CardStatus::Active:  return "ACTIVE";
        case CardStatus::Blocked: return "BLOCKED";
        case CardStatus::Revoked: return "REVOKED";
    }
    return "ACTIVE";
}

CardStatus Card::stringToStatus(std::string_view str) {
    if (str == "BLOCKED") return CardStatus::Blocked;
    if (str == "REVOKED") return CardStatus::Revoked;
    return CardStatus::Active;
}