#pragma once

#include <string>
#include <memory>
#include "../models/UserRepo.h"
#include "CryptoService.h"

class AdminCheck {
private:
    std::shared_ptr<UserRepo> userRepo;
    std::shared_ptr<CryptoService> cryptoService;

public:
    AdminCheck(std::shared_ptr<UserRepo> userRepo, std::shared_ptr<CryptoService> cryptoService);
    bool authenticate(const std::string& username, const std::string& password) const;
};
