#include "CardService.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace {

bool isDigitsOnly(const std::string& str) {
    return !str.empty() && std::all_of(str.begin(), str.end(), [](unsigned char c) {
        return std::isdigit(c);
    });
}

} // namespace

CardService::CardService(
    std::shared_ptr<CardRepo> cardRepo,
    std::shared_ptr<UserRepo> userRepo,
    std::shared_ptr<AccessLogRecordRepo> logRepo,
    std::shared_ptr<CryptoService> cryptoService
) : cardRepo_(std::move(cardRepo)),
    userRepo_(std::move(userRepo)),
    logRepo_(std::move(logRepo)),
    cryptoService_(std::move(cryptoService)) {}

std::string CardService::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void CardService::logEvent(const std::string& cardId, LogEventType eventType) {
    if (logRepo_) {
        try {
            AccessLogRecord record(getCurrentTimestamp(), cardId, eventType);
            logRepo_->create(record);
        } catch (...) {
            // logging failure shouldn't crash
        }
    }
}

Services::Result<Card, Services::CardError> CardService::getCardByNumber(const std::string& cardNumber) {
    if (cardNumber.empty()) {
        return Services::Err(Services::CardError::InvalidCardFormat);
    }
    auto cardOpt = cardRepo_->getByCardId(cardNumber);
    if (!cardOpt.has_value()) {
        return Services::Err(Services::CardError::CardNotFound);
    }
    return Services::Ok(*cardOpt);
}

std::vector<Card> CardService::listAllCards() {
    return cardRepo_->list();
}

std::vector<Card> CardService::getCardsByUserId(const std::string& userId) {
    return cardRepo_->listByUserId(userId);
}

Services::Result<Card, Services::CardError> CardService::addCard(
    const std::string& userId,
    const std::string& cardNumber,
    const std::string& pin
) {
    if (userId.empty()) {
        return Services::Err(Services::CardError::UserNotFound);
    }
    if (cardNumber.length() != 16 || !isDigitsOnly(cardNumber)) {
        return Services::Err(Services::CardError::InvalidCardFormat);
    }
    if (pin.length() != 4 || !isDigitsOnly(pin)) {
        return Services::Err(Services::CardError::InvalidPinFormat);
    }

    auto userOpt = userRepo_->getById(userId);
    if (!userOpt.has_value()) {
        return Services::Err(Services::CardError::UserNotFound);
    }

    auto existing = cardRepo_->getByCardId(cardNumber);
    if (existing.has_value()) {
        return Services::Err(Services::CardError::CardAlreadyExists);
    }

    try {
        std::string hash = cryptoService_->hashPin(pin);
        Card newCard(userId, cardNumber, hash);
        cardRepo_->create(newCard);
        logEvent(cardNumber, LogEventType::CardIssued);
        return Services::Ok(newCard);
    } catch (...) {
        return Services::Err(Services::CardError::DatabaseError);
    }
}

Services::Result<void, Services::CardError> CardService::blockCard(const std::string& cardNumber) {
    if (cardNumber.empty()) {
        return Services::Err(Services::CardError::InvalidCardFormat);
    }

    auto cardOpt = cardRepo_->getByCardId(cardNumber);
    if (!cardOpt.has_value()) {
        return Services::Err(Services::CardError::CardNotFound);
    }

    Card card = *cardOpt;
    if (card.getStatus() == CardStatus::Blocked) {
        return Services::Err(Services::CardError::CardAlreadyBlocked);
    }
    if (card.getStatus() == CardStatus::Revoked) {
        return Services::Err(Services::CardError::CardRevoked);
    }

    card.setStatus(CardStatus::Blocked);
    try {
        cardRepo_->update(card);
        logEvent(cardNumber, LogEventType::Alarm);
        return Services::Ok();
    } catch (...) {
        return Services::Err(Services::CardError::DatabaseError);
    }
}

Services::Result<void, Services::CardError> CardService::unblockCard(const std::string& cardNumber) {
    if (cardNumber.empty()) {
        return Services::Err(Services::CardError::InvalidCardFormat);
    }

    auto cardOpt = cardRepo_->getByCardId(cardNumber);
    if (!cardOpt.has_value()) {
        return Services::Err(Services::CardError::CardNotFound);
    }

    Card card = *cardOpt;
    if (card.getStatus() == CardStatus::Revoked) {
        return Services::Err(Services::CardError::CardRevoked);
    }
    if (card.getStatus() == CardStatus::Active && card.getFailedAttempts() == 0) {
        return Services::Err(Services::CardError::CardAlreadyActive);
    }

    card.resetFailedAttempts();
    card.setStatus(CardStatus::Active);

    try {
        cardRepo_->update(card);
        logEvent(cardNumber, LogEventType::CardUnlocked);
        return Services::Ok();
    } catch (...) {
        return Services::Err(Services::CardError::DatabaseError);
    }
}

Services::Result<void, Services::CardError> CardService::revokeCard(const std::string& cardNumber) {
    if (cardNumber.empty()) {
        return Services::Err(Services::CardError::InvalidCardFormat);
    }

    auto cardOpt = cardRepo_->getByCardId(cardNumber);
    if (!cardOpt.has_value()) {
        return Services::Err(Services::CardError::CardNotFound);
    }

    Card card = *cardOpt;
    if (card.getStatus() == CardStatus::Revoked) {
        return Services::Ok();
    }

    card.setStatus(CardStatus::Revoked);

    try {
        cardRepo_->update(card);
        logEvent(cardNumber, LogEventType::CardRevoked);
        return Services::Ok();
    } catch (...) {
        return Services::Err(Services::CardError::DatabaseError);
    }
}

Services::Result<void, Services::CardError> CardService::changePinByAdmin(
    const std::string& cardNumber,
    const std::string& newPin
) {
    if (cardNumber.empty()) {
        return Services::Err(Services::CardError::InvalidCardFormat);
    }
    if (newPin.length() != 4 || !isDigitsOnly(newPin)) {
        return Services::Err(Services::CardError::InvalidPinFormat);
    }

    auto cardOpt = cardRepo_->getByCardId(cardNumber);
    if (!cardOpt.has_value()) {
        return Services::Err(Services::CardError::CardNotFound);
    }

    Card card = *cardOpt;
    if (card.getStatus() == CardStatus::Revoked) {
        return Services::Err(Services::CardError::CardRevoked);
    }

    try {
        card.setPinHash(cryptoService_->hashPin(newPin));
        card.resetFailedAttempts();
        cardRepo_->update(card);
        logEvent(cardNumber, LogEventType::PinChanged);
        return Services::Ok();
    } catch (...) {
        return Services::Err(Services::CardError::DatabaseError);
    }
}

Services::Result<void, Services::CardError> CardService::changePinByUser(
    const std::string& cardNumber,
    const std::string& oldPin1,
    const std::string& oldPin2,
    const std::string& newPin,
    const std::string& confirmNewPin
) {
    if (oldPin1.empty() || oldPin2.empty() || newPin.empty() || confirmNewPin.empty()) {
        return Services::Err(Services::CardError::InvalidPinFormat);
    }

    if (oldPin1 != oldPin2) {
        return Services::Err(Services::CardError::OldPinsDoNotMatch);
    }

    if (newPin != confirmNewPin) {
        return Services::Err(Services::CardError::NewPinsDoNotMatch);
    }

    if (newPin.length() != 4 || !isDigitsOnly(newPin)) {
        return Services::Err(Services::CardError::InvalidPinFormat);
    }

    auto cardOpt = cardRepo_->getByCardId(cardNumber);
    if (!cardOpt.has_value()) {
        return Services::Err(Services::CardError::CardNotFound);
    }

    Card card = *cardOpt;
    if (card.getStatus() == CardStatus::Revoked) {
        return Services::Err(Services::CardError::CardRevoked);
    }
    if (card.getStatus() == CardStatus::Blocked) {
        return Services::Err(Services::CardError::CardAlreadyBlocked);
    }

    std::string oldHash = cryptoService_->hashPin(oldPin1);
    if (oldHash != card.getPinHash()) {
        return Services::Err(Services::CardError::OldPinIncorrect);
    }

    try {
        std::string newHash = cryptoService_->hashPin(newPin);
        card.setPinHash(newHash);
        cardRepo_->update(card);
        logEvent(cardNumber, LogEventType::PinChanged);
        return Services::Ok();
    } catch (...) {
        return Services::Err(Services::CardError::DatabaseError);
    }
}
