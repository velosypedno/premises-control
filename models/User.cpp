#include "User.h"
#include "uuid.h"

User::User(std::string id, std::string firstName,
    std::string lastName, std::optional<std::string> policyId,
    UserRole role, std::optional<std::string> username,
    std::optional<std::string> passwordHash)
    : id(std::move(id)), firstName(std::move(firstName)),
    lastName(std::move(lastName)), policyId(std::move(policyId)),
    role(role), username(std::move(username)), passwordHash(std::move(passwordHash)) {}

User::User(std::string firstName, std::string lastName,
    std::optional<std::string> policyId, UserRole role,
    std::optional<std::string> username, std::optional<std::string> passwordHash)
    : id(uuid::generate()), firstName(std::move(firstName)),
    lastName(std::move(lastName)), policyId(std::move(policyId)),
    role(role), username(std::move(username)), passwordHash(std::move(passwordHash)) {}

std::string_view User::getId() const noexcept {
    return id;
}

std::string_view User::getFirstName() const noexcept {
    return firstName;
}

std::string_view User::getLastName() const noexcept {
    return lastName;
}

std::optional<std::string_view> User::getPolicyId() const noexcept {
    if (policyId.has_value()) {
        return *policyId;
    }
    return std::nullopt;
}

const std::optional<std::string>& User::getPolicyIdOpt() const noexcept {
    return policyId;
}

UserRole User::getRole() const noexcept {
    return role;
}

std::optional<std::string_view> User::getUsername() const noexcept {
    if (username.has_value()) {
        return *username;
    }
    return std::nullopt;
}

const std::optional<std::string>& User::getUsernameOpt() const noexcept {
    return username;
}

std::optional<std::string_view> User::getPasswordHash() const noexcept {
    if (passwordHash.has_value()) {
        return *passwordHash;
    }
    return std::nullopt;
}

const std::optional<std::string>& User::getPasswordHashOpt() const noexcept {
    return passwordHash;
}

std::string User::getFullName() const {
    std::string fullName;
    fullName.reserve(firstName.size() + lastName.size() + 1);
    fullName = firstName;
    fullName += " ";
    fullName += lastName;
    return fullName;
}

void User::setFirstName(std::string newFirstName) {
    firstName = std::move(newFirstName);
}

void User::setLastName(std::string newLastName) {
    lastName = std::move(newLastName);
}

void User::setPolicyId(std::optional<std::string> newPolicyId) {
    policyId = std::move(newPolicyId);
}

void User::setRole(UserRole newRole) noexcept {
    role = newRole;
}

void User::setUsername(std::optional<std::string> newUsername) {
    username = std::move(newUsername);
}

void User::setPasswordHash(std::optional<std::string> newPasswordHash) {
    passwordHash = std::move(newPasswordHash);
}

std::string User::roleToString(UserRole role) {
    switch (role) {
        case UserRole::Admin: return "ADMIN";
        case UserRole::User:  return "USER";
    }
    return "USER";
}

UserRole User::stringToRole(std::string_view str) {
    if (str == "ADMIN") {
        return UserRole::Admin;
    }
    return UserRole::User;
}
