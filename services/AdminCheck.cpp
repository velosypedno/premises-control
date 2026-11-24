#include "AdminCheck.h"
#include "../models/Card.h"

std::string toErrorMessage(AdminAuthError err) {
    switch (err) {
        case AdminAuthError::InvalidCredentials:
            return "Invalid username or password.";
        case AdminAuthError::NotAnAdmin:
            return "Access denied: administrator privileges required.";
        case AdminAuthError::UserInactive:
            return "Administrator account is inactive.";
        case AdminAuthError::PasswordNotSet:
            return "Password authentication is not configured for this account.";
        case AdminAuthError::DatabaseError:
            return "Database error during authentication.";
    }
    return "Administrator access denied.";
}

AdminCheck::AdminCheck(std::shared_ptr<UserRepo> userRepo,
                       std::shared_ptr<CardRepo> cardRepo,
                       std::shared_ptr<CryptoService> cryptoService)
    : userRepo_(std::move(userRepo)),
      cardRepo_(std::move(cardRepo)),
      cryptoService_(std::move(cryptoService)) {}

Services::Result<User, AdminAuthError> AdminCheck::authenticate(
    const std::string& username,
    const std::string& password
) const {
    try {
        auto userOpt = userRepo_->getByUsername(username);
        if (!userOpt.has_value()) {
            return Services::Err(AdminAuthError::InvalidCredentials);
        }

        User user = std::move(*userOpt);

        if (user.getRole() != UserRole::Admin) {
            return Services::Err(AdminAuthError::NotAnAdmin);
        }

        if (!user.isActive()) {
            return Services::Err(AdminAuthError::UserInactive);
        }

        const auto& passwordHashOpt = user.getPasswordHashOpt();
        if (!passwordHashOpt.has_value()) {
            return Services::Err(AdminAuthError::PasswordNotSet);
        }

        std::string inputHash = cryptoService_->hashPin(password);
        if (*passwordHashOpt != inputHash) {
            return Services::Err(AdminAuthError::InvalidCredentials);
        }

        return Services::Ok(std::move(user));
    } catch (const std::exception&) {
        return Services::Err(AdminAuthError::DatabaseError);
    }
}

Services::Result<User, AdminAuthError> AdminCheck::authenticateByCard(
    const std::string& cardId,
    const std::string& pin
) const {
    try {
        auto cardOpt = cardRepo_->getByCardId(cardId);
        if (!cardOpt.has_value()) {
            return Services::Err(AdminAuthError::InvalidCredentials);
        }

        const Card& card = *cardOpt;
        if (card.getStatus() != CardStatus::Active) {
            return Services::Err(AdminAuthError::InvalidCredentials);
        }

        if (cryptoService_->hashPin(pin) != card.getPinHash()) {
            return Services::Err(AdminAuthError::InvalidCredentials);
        }

        auto userOpt = userRepo_->getById(std::string(card.getUserId()));
        if (!userOpt.has_value()) {
            return Services::Err(AdminAuthError::InvalidCredentials);
        }

        User user = std::move(*userOpt);
        if (user.getRole() != UserRole::Admin) {
            return Services::Err(AdminAuthError::NotAnAdmin);
        }
        if (!user.isActive()) {
            return Services::Err(AdminAuthError::UserInactive);
        }

        return Services::Ok(std::move(user));
    } catch (const std::exception&) {
        return Services::Err(AdminAuthError::DatabaseError);
    }
}
