#pragma once

#include <variant>
#include <utility>
#include <stdexcept>
#include <type_traits>
#include <string>

namespace Services {

template <typename E>
struct ErrTag {
    E error;
};

template <typename E>
ErrTag<std::decay_t<E>> Err(E&& err) {
    return ErrTag<std::decay_t<E>>{std::forward<E>(err)};
}

template <typename T>
struct OkTag {
    T value;
};

template <typename T>
OkTag<std::decay_t<T>> Ok(T&& val) {
    return OkTag<std::decay_t<T>>{std::forward<T>(val)};
}

inline OkTag<std::monostate> Ok() {
    return OkTag<std::monostate>{std::monostate{}};
}

template <typename T, typename E>
class Result {
private:
    std::variant<T, E> data_;

public:
    Result(T val) : data_(std::in_place_index<0>, std::move(val)) {}
    Result(OkTag<T> ok) : data_(std::in_place_index<0>, std::move(ok.value)) {}
    Result(ErrTag<E> err) : data_(std::in_place_index<1>, std::move(err.error)) {}

    static Result ok(T val) { return Result(OkTag<T>{std::move(val)}); }
    static Result err(E error) { return Result(ErrTag<E>{std::move(error)}); }

    [[nodiscard]] bool is_ok() const noexcept { return std::holds_alternative<T>(data_); }
    [[nodiscard]] bool is_err() const noexcept { return std::holds_alternative<E>(data_); }

    explicit operator bool() const noexcept { return is_ok(); }

    const T& value() const {
        if (is_err()) {
            throw std::logic_error("Attempted to access value of an Err Result");
        }
        return std::get<0>(data_);
    }

    T& value() {
        if (is_err()) {
            throw std::logic_error("Attempted to access value of an Err Result");
        }
        return std::get<0>(data_);
    }

    const E& error() const {
        if (is_ok()) {
            throw std::logic_error("Attempted to access error of an Ok Result");
        }
        return std::get<1>(data_);
    }

    E& error() {
        if (is_ok()) {
            throw std::logic_error("Attempted to access error of an Ok Result");
        }
        return std::get<1>(data_);
    }

    T value_or(T default_val) const {
        return is_ok() ? std::get<0>(data_) : default_val;
    }
};

template <typename E>
class Result<void, E> {
private:
    std::variant<std::monostate, E> data_;

public:
    Result() : data_(std::in_place_index<0>, std::monostate{}) {}
    Result(OkTag<std::monostate>) : data_(std::in_place_index<0>, std::monostate{}) {}
    Result(ErrTag<E> err) : data_(std::in_place_index<1>, std::move(err.error)) {}

    static Result ok() { return Result(); }
    static Result err(E error) { return Result(ErrTag<E>{std::move(error)}); }

    [[nodiscard]] bool is_ok() const noexcept { return std::holds_alternative<std::monostate>(data_); }
    [[nodiscard]] bool is_err() const noexcept { return std::holds_alternative<E>(data_); }

    explicit operator bool() const noexcept { return is_ok(); }

    const E& error() const {
        if (is_ok()) {
            throw std::logic_error("Attempted to access error of an Ok Result");
        }
        return std::get<1>(data_);
    }

    E& error() {
        if (is_ok()) {
            throw std::logic_error("Attempted to access error of an Ok Result");
        }
        return std::get<1>(data_);
    }
};

// =================== Domain Error Types ===================

enum class UserError {
    UserNotFound,
    EmptyFields,
    UsernameAlreadyTaken,
    InvalidPassword,
    PolicyNotFound,
    DatabaseError
};

inline std::string toErrorMessage(UserError err) {
    switch (err) {
        case UserError::UserNotFound:
            return "User not found.";
        case UserError::EmptyFields:
            return "All required fields must be filled out.";
        case UserError::UsernameAlreadyTaken:
            return "This username is already taken by another user.";
        case UserError::InvalidPassword:
            return "Invalid password format.";
        case UserError::PolicyNotFound:
            return "The specified access policy was not found.";
        case UserError::DatabaseError:
            return "Database error while saving the user.";
    }
    return "Unknown user error.";
}

enum class CardError {
    CardNotFound,
    UserNotFound,
    CardAlreadyExists,
    InvalidCardFormat,
    InvalidPinFormat,
    OldPinsDoNotMatch,
    OldPinIncorrect,
    NewPinsDoNotMatch,
    CardAlreadyBlocked,
    CardAlreadyActive,
    CardRevoked,
    DatabaseError
};

inline std::string toErrorMessage(CardError err) {
    switch (err) {
        case CardError::CardNotFound:
            return "Card with this number was not found.";
        case CardError::UserNotFound:
            return "User with this ID was not found.";
        case CardError::CardAlreadyExists:
            return "A card with this number already exists in the system.";
        case CardError::InvalidCardFormat:
            return "Card number must contain exactly 16 digits.";
        case CardError::InvalidPinFormat:
            return "PIN code must contain exactly 4 digits.";
        case CardError::OldPinsDoNotMatch:
            return "The entered old PIN fields do not match.";
        case CardError::OldPinIncorrect:
            return "The entered old PIN is incorrect.";
        case CardError::NewPinsDoNotMatch:
            return "The new PIN and its confirmation do not match.";
        case CardError::CardAlreadyBlocked:
            return "The card is already blocked.";
        case CardError::CardAlreadyActive:
            return "The card is already active.";
        case CardError::CardRevoked:
            return "The card has been revoked (cannot be restored).";
        case CardError::DatabaseError:
            return "Database error while processing the card.";
    }
    return "Unknown card error.";
}

} // namespace Services
