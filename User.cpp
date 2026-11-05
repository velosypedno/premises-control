#include "User.h"
#include <random>
#include <sstream>

std::string User::generateUuid() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);

    std::stringstream ss;
    int i = 0;
    ss << std::hex;
    for (i = 0; i < 8; i++) ss << dis(gen);
    ss << "-";
    for (i = 0; i < 4; i++) ss << dis(gen);
    ss << "-4";
    for (i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    ss << dis2(gen);
    for (i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    for (i = 0; i < 12; i++) ss << dis(gen);
    return ss.str();
}

User::User(std::string id, std::string firstName, std::string lastName)
    : id(std::move(id)), firstName(std::move(firstName)), lastName(std::move(lastName)) {}

User::User(std::string firstName, std::string lastName)
    : id(generateUuid()), firstName(std::move(firstName)), lastName(std::move(lastName)) {}

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

void User::setId(std::string newId) {
    id = std::move(newId);
}

void User::setFirstName(std::string newFirstName) {
    firstName = std::move(newFirstName);
}

void User::setLastName(std::string newLastName) {
    lastName = std::move(newLastName);
}