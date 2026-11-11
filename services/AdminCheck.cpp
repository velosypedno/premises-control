#include "AdminCheck.h"

AdminCheck::AdminCheck(std::shared_ptr<UserRepo> userRepo, std::shared_ptr<CryptoService> cryptoService)
    : userRepo(std::move(userRepo)), cryptoService(std::move(cryptoService)) {}

bool AdminCheck::authenticate(const std::string& username, const std::string& password) const {
    auto userOpt = userRepo->getByUsername(username);

    if (!userOpt.has_value() || userOpt->getRole() != UserRole::Admin) {
        return false;
    }

    const auto& passwordHashOpt = userOpt->getPasswordHashOpt();
    if (!passwordHashOpt.has_value()) {
        return false;
    }

    std::string inputHash = cryptoService->hashPin(password);
    return *passwordHashOpt == inputHash;
}
