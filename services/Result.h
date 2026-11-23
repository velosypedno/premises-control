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
            return "Користувача не знайдено.";
        case UserError::EmptyFields:
            return "Всі обов'язкові поля мають бути заповнені.";
        case UserError::UsernameAlreadyTaken:
            return "Це ім'я користувача (username) вже зайняте іншим користувачем.";
        case UserError::InvalidPassword:
            return "Неприпустимий формат пароля.";
        case UserError::PolicyNotFound:
            return "Вказану політику доступу не знайдено.";
        case UserError::DatabaseError:
            return "Помилка бази даних під час збереження користувача.";
    }
    return "Невідома помилка користувача.";
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
            return "Картку з таким номером не знайдено.";
        case CardError::UserNotFound:
            return "Користувача з таким ID не знайдено.";
        case CardError::CardAlreadyExists:
            return "Картка з таким номером вже існує в системі.";
        case CardError::InvalidCardFormat:
            return "Номер картки повинен містити рівно 16 цифр.";
        case CardError::InvalidPinFormat:
            return "PIN-код повинен містити рівно 4 цифри.";
        case CardError::OldPinsDoNotMatch:
            return "Введені поля старого PIN-коду не збігаються.";
        case CardError::OldPinIncorrect:
            return "Введено невірний старий PIN-код.";
        case CardError::NewPinsDoNotMatch:
            return "Новий PIN-код та його підтвердження не збігаються.";
        case CardError::CardAlreadyBlocked:
            return "Картка вже заблокована.";
        case CardError::CardAlreadyActive:
            return "Картка вже активна.";
        case CardError::CardRevoked:
            return "Картку відкликано (не підлягає відновленню).";
        case CardError::DatabaseError:
            return "Помилка бази даних при роботі з карткою.";
    }
    return "Невідома помилка картки.";
}

} // namespace Services
