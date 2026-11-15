#include "AdminCheck.h"
#include "../models/Card.h"

AdminCheck::AdminCheck(std::shared_ptr<UserRepo> userRepo,
                       std::shared_ptr<CardRepo> cardRepo,
                       std::shared_ptr<CryptoService> cryptoService)
    : userRepo(std::move(userRepo)),
      cardRepo(std::move(cardRepo)),
      cryptoService(std::move(cryptoService)) {}

bool AdminCheck::authenticate(const std::string& username, const std::string& password) const {
    auto userOpt = userRepo->getByUsername(username);

    if (!userOpt.has_value() || userOpt->getRole() != UserRole::Admin) {
        return false;
    }

    if (!userOpt->isActive()) {
        return false;
    }

    const auto& passwordHashOpt = userOpt->getPasswordHashOpt();
    if (!passwordHashOpt.has_value()) {
        return false;
    }

    std::string inputHash = cryptoService->hashPin(password);
    return *passwordHashOpt == inputHash;
}

std::optional<User> AdminCheck::authenticateByCard(const std::string& cardId, const std::string& pin) const {
    auto cardOpt = cardRepo->getByCardId(cardId);
    if (!cardOpt.has_value()) {
        return std::nullopt;
    }

    const Card& card = *cardOpt;
    if (card.getStatus() != CardStatus::Active) {
        return std::nullopt;
    }

    if (cryptoService->hashPin(pin) != card.getPinHash()) {
        return std::nullopt;
    }

    auto userOpt = userRepo->getById(std::string(card.getUserId()));
    if (!userOpt.has_value()) {
        return std::nullopt;
    }

    if (userOpt->getRole() != UserRole::Admin || !userOpt->isActive()) {
        return std::nullopt;
    }

    return userOpt;
}
