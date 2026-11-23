#include "UserService.h"

UserService::UserService(
    std::shared_ptr<UserRepo> userRepo,
    std::shared_ptr<AccessPolicyRepo> policyRepo,
    std::shared_ptr<CryptoService> cryptoService
) : userRepo_(std::move(userRepo)),
    policyRepo_(std::move(policyRepo)),
    cryptoService_(std::move(cryptoService)) {}

Services::Result<User, Services::UserError> UserService::getUserById(const std::string& userId) {
    if (userId.empty()) {
        return Services::Err(Services::UserError::EmptyFields);
    }
    auto userOpt = userRepo_->getById(userId);
    if (!userOpt.has_value()) {
        return Services::Err(Services::UserError::UserNotFound);
    }
    return Services::Ok(*userOpt);
}

std::vector<User> UserService::listAllUsers() {
    return userRepo_->list();
}

Services::Result<User, Services::UserError> UserService::updateProfile(
    const std::string& userId,
    const std::string& firstName,
    const std::string& lastName,
    const std::string& username,
    const std::optional<std::string>& newPassword
) {
    if (userId.empty()) {
        return Services::Err(Services::UserError::UserNotFound);
    }
    if (firstName.empty() || lastName.empty() || username.empty()) {
        return Services::Err(Services::UserError::EmptyFields);
    }

    auto userOpt = userRepo_->getById(userId);
    if (!userOpt.has_value()) {
        return Services::Err(Services::UserError::UserNotFound);
    }

    User user = *userOpt;

    auto existing = userRepo_->getByUsername(username);
    if (existing.has_value() && existing->getId() != user.getId()) {
        return Services::Err(Services::UserError::UsernameAlreadyTaken);
    }

    user.setFirstName(firstName);
    user.setLastName(lastName);
    user.setUsername(username);

    if (newPassword.has_value() && !newPassword->empty()) {
        std::string newHash = cryptoService_->hashPin(*newPassword);
        user.setPasswordHash(newHash);
    }

    try {
        userRepo_->update(user);
        return Services::Ok(user);
    } catch (...) {
        return Services::Err(Services::UserError::DatabaseError);
    }
}

Services::Result<User, Services::UserError> UserService::updateName(
    const std::string& userId,
    const std::string& firstName,
    const std::string& lastName
) {
    if (userId.empty()) {
        return Services::Err(Services::UserError::UserNotFound);
    }
    if (firstName.empty() || lastName.empty()) {
        return Services::Err(Services::UserError::EmptyFields);
    }

    auto userOpt = userRepo_->getById(userId);
    if (!userOpt.has_value()) {
        return Services::Err(Services::UserError::UserNotFound);
    }

    User user = *userOpt;
    user.setFirstName(firstName);
    user.setLastName(lastName);

    try {
        userRepo_->update(user);
        return Services::Ok(user);
    } catch (...) {
        return Services::Err(Services::UserError::DatabaseError);
    }
}

Services::Result<User, Services::UserError> UserService::createUser(
    const std::string& firstName,
    const std::string& lastName,
    const std::string& username,
    const std::string& password
) {
    if (firstName.empty() || lastName.empty() || username.empty() || password.empty()) {
        return Services::Err(Services::UserError::EmptyFields);
    }

    auto existing = userRepo_->getByUsername(username);
    if (existing.has_value()) {
        return Services::Err(Services::UserError::UsernameAlreadyTaken);
    }

    try {
        std::string hash = cryptoService_->hashPin(password);
        User newUser(firstName, lastName, std::nullopt, UserRole::User, username, hash);
        userRepo_->create(newUser);
        return Services::Ok(newUser);
    } catch (...) {
        return Services::Err(Services::UserError::DatabaseError);
    }
}

Services::Result<void, Services::UserError> UserService::assignAdminRole(const std::string& userId) {
    if (userId.empty()) {
        return Services::Err(Services::UserError::EmptyFields);
    }

    auto userOpt = userRepo_->getById(userId);
    if (!userOpt.has_value()) {
        return Services::Err(Services::UserError::UserNotFound);
    }

    User user = *userOpt;
    user.setRole(UserRole::Admin);

    try {
        userRepo_->update(user);
        return Services::Ok();
    } catch (...) {
        return Services::Err(Services::UserError::DatabaseError);
    }
}

Services::Result<void, Services::UserError> UserService::assignPolicy(
    const std::string& userId,
    const std::string& policyId
) {
    if (userId.empty() || policyId.empty()) {
        return Services::Err(Services::UserError::EmptyFields);
    }

    auto userOpt = userRepo_->getById(userId);
    if (!userOpt.has_value()) {
        return Services::Err(Services::UserError::UserNotFound);
    }

    auto policyOpt = policyRepo_->getById(policyId);
    if (!policyOpt.has_value()) {
        return Services::Err(Services::UserError::PolicyNotFound);
    }

    User user = *userOpt;
    user.setPolicyId(policyId);

    try {
        userRepo_->update(user);
        return Services::Ok();
    } catch (...) {
        return Services::Err(Services::UserError::DatabaseError);
    }
}

Services::Result<bool, Services::UserError> UserService::toggleUserActive(const std::string& userId) {
    if (userId.empty()) {
        return Services::Err(Services::UserError::EmptyFields);
    }

    auto userOpt = userRepo_->getById(userId);
    if (!userOpt.has_value()) {
        return Services::Err(Services::UserError::UserNotFound);
    }

    User user = *userOpt;
    bool newActive = !user.isActive();
    user.setActive(newActive);

    try {
        userRepo_->update(user);
        return Services::Ok(newActive);
    } catch (...) {
        return Services::Err(Services::UserError::DatabaseError);
    }
}
