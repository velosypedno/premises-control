#pragma once

#include <string>
#include <memory>
#include "../models/UserRepo.h"
#include "../models/CardRepo.h"
#include "../models/User.h"
#include "CryptoService.h"
#include "Result.h"

enum class AdminAuthError {
    InvalidCredentials,
    NotAnAdmin,
    UserInactive,
    PasswordNotSet,
    DatabaseError
};

std::string toErrorMessage(AdminAuthError err);

class AdminCheck {
private:
    std::shared_ptr<UserRepo> userRepo_;
    std::shared_ptr<CardRepo> cardRepo_;
    std::shared_ptr<CryptoService> cryptoService_;

public:
    AdminCheck(std::shared_ptr<UserRepo> userRepo,
               std::shared_ptr<CardRepo> cardRepo,
               std::shared_ptr<CryptoService> cryptoService);

    Services::Result<User, AdminAuthError> authenticate(
        const std::string& username,
        const std::string& password
    ) const;

    Services::Result<User, AdminAuthError> authenticateByCard(
        const std::string& cardId,
        const std::string& pin
    ) const;
};

using AdminService = AdminCheck;
