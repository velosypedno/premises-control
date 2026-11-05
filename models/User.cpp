#include "User.h"
#include "uuid.h"

User::User(std::string id, std::string firstName, std::string lastName, std::optional<std::string> policyId)
    : id(std::move(id)), firstName(std::move(firstName)), lastName(std::move(lastName)), policyId(std::move(policyId)) {}

User::User(std::string firstName, std::string lastName, std::optional<std::string> policyId)
    : id(uuid::generate()), firstName(std::move(firstName)), lastName(std::move(lastName)), policyId(std::move(policyId)) {}

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