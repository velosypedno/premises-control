#pragma once

#include <string>
#include <memory>
#include <optional>
#include "../models/UserRepo.h"
#include "../models/CardRepo.h"
#include "../models/User.h"
#include "CryptoService.h"

class AdminCheck {
private:
    std::shared_ptr<UserRepo> userRepo;
    std::shared_ptr<CardRepo> cardRepo;
    std::shared_ptr<CryptoService> cryptoService;

public:
    AdminCheck(std::shared_ptr<UserRepo> userRepo,
               std::shared_ptr<CardRepo> cardRepo,
               std::shared_ptr<CryptoService> cryptoService);

    bool authenticate(const std::string& username, const std::string& password) const;
    std::optional<User> authenticateByCard(const std::string& cardId, const std::string& pin) const;
};

using AdminService = AdminCheck;
