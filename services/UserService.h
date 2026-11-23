#pragma once

#include <memory>
#include <string>
#include <vector>
#include <optional>

#include "Result.h"
#include "../models/User.h"
#include "../models/UserRepo.h"
#include "../models/AccessPolicyRepo.h"
#include "CryptoService.h"

class UserService {
private:
    std::shared_ptr<UserRepo> userRepo_;
    std::shared_ptr<AccessPolicyRepo> policyRepo_;
    std::shared_ptr<CryptoService> cryptoService_;

public:
    UserService(
        std::shared_ptr<UserRepo> userRepo,
        std::shared_ptr<AccessPolicyRepo> policyRepo,
        std::shared_ptr<CryptoService> cryptoService
    );

    Services::Result<User, Services::UserError> getUserById(const std::string& userId);

    std::vector<User> listAllUsers();

    Services::Result<User, Services::UserError> updateProfile(
        const std::string& userId,
        const std::string& firstName,
        const std::string& lastName,
        const std::string& username,
        const std::optional<std::string>& newPassword = std::nullopt
    );

    Services::Result<User, Services::UserError> updateName(
        const std::string& userId,
        const std::string& firstName,
        const std::string& lastName
    );

    Services::Result<User, Services::UserError> createUser(
        const std::string& firstName,
        const std::string& lastName,
        const std::string& username,
        const std::string& password
    );

    Services::Result<void, Services::UserError> assignAdminRole(const std::string& userId);

    Services::Result<void, Services::UserError> assignPolicy(
        const std::string& userId,
        const std::string& policyId
    );

    Services::Result<bool, Services::UserError> toggleUserActive(const std::string& userId);
};
