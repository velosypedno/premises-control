#include "User.h"
#include "uuid.h"

User::User(std::string id, std::string firstName, std::string lastName)
    : id(std::move(id)), firstName(std::move(firstName)), lastName(std::move(lastName)) {}

User::User(std::string firstName, std::string lastName)
    : id(uuid::generate()), firstName(std::move(firstName)), lastName(std::move(lastName)) {}

std::string_view User::getId() const noexcept {
    return id;
}

std::string_view User::getFirstName() const noexcept {
    return firstName;
}

std::string_view User::getLastName() const noexcept {
    return lastName;
}

std::string User::getFullName() const {
    std::string fullName;
    fullName.reserve(firstName.size() + lastName.size() + 1);
    fullName = firstName;
    fullName += " ";
    fullName += lastName;
    return fullName;
}
