#pragma once

#include <string>
#include <string_view>

class User {
private:
    std::string id;
    std::string firstName;
    std::string lastName;

public:
    User() = default;
    User(std::string id, std::string firstName, std::string lastName);
    User(std::string firstName, std::string lastName);

    User(const User&) = default;
    User& operator=(const User&) = default;
    User(User&&) noexcept = default;
    User& operator=(User&&) noexcept = default;

    ~User() = default;

    std::string_view getId() const noexcept;
    std::string_view getFirstName() const noexcept;
    std::string_view getLastName() const noexcept;
    std::string getFullName() const;

    void setId(std::string newId);
    void setFirstName(std::string newFirstName);
    void setLastName(std::string newLastName);

private:
    static std::string generateUuid();
};