#pragma once

#include <string>
#include <string_view>
#include <optional>

enum class UserRole {
    User,
    Admin
};

class User {
private:
    std::string id;
    std::string firstName;
    std::string lastName;
    std::optional<std::string> policyId;
    UserRole role;
    std::optional<std::string> username;
    std::optional<std::string> passwordHash;

public:
    User() = default;

    User(std::string id,
         std::string firstName,
         std::string lastName,
         std::optional<std::string> policyId,
         UserRole role,
         std::optional<std::string> username = std::nullopt,
         std::optional<std::string> passwordHash = std::nullopt);

    User(std::string firstName,
         std::string lastName,
         std::optional<std::string> policyId = std::nullopt,
         UserRole role = UserRole::User,
         std::optional<std::string> username = std::nullopt,
         std::optional<std::string> passwordHash = std::nullopt);

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

    UserRole getRole() const noexcept;

    std::optional<std::string_view> getUsername() const noexcept;
    const std::optional<std::string>& getUsernameOpt() const noexcept;

    std::optional<std::string_view> getPasswordHash() const noexcept;
    const std::optional<std::string>& getPasswordHashOpt() const noexcept;

    std::string getFullName() const;

    void setFirstName(std::string newFirstName);
    void setLastName(std::string newLastName);
    void setPolicyId(std::optional<std::string> newPolicyId);
    void setRole(UserRole newRole) noexcept;
    void setUsername(std::optional<std::string> newUsername);
    void setPasswordHash(std::optional<std::string> newPasswordHash);

    static std::string roleToString(UserRole role);
    static UserRole stringToRole(std::string_view str);
};
