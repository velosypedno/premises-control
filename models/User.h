#pragma once

#include <string>
#include <string_view>
#include <optional>

class User {
private:
    std::string id;
    std::string firstName;
    std::string lastName;
    std::optional<std::string> policyId;

public:
    User() = default;
    User(std::string id, std::string firstName, std::string lastName, std::optional<std::string> policyId = std::nullopt);
    User(std::string firstName, std::string lastName, std::optional<std::string> policyId = std::nullopt);

    User(const User&) = default;
    User& operator=(const User&) = default;
    User(User&&) noexcept = default;
    User& operator=(User&&) noexcept = default;

    ~User() = default;

    std::string_view getId() const noexcept;
    std::string_view getFirstName() const noexcept;
    std::string_view getLastName() const noexcept;
    std::optional<std::string_view> getPolicyId() const noexcept;
    const std::optional<std::string>& getPolicyIdOpt() const noexcept;
    std::string getFullName() const;

    void setFirstName(std::string newFirstName);
    void setLastName(std::string newLastName);
    void setPolicyId(std::optional<std::string> newPolicyId);
};