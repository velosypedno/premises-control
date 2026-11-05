#pragma once

#include <string>
#include <string_view>

enum class CardStatus {
    Active,
    Blocked,
    Revoked
};

class Card {
private:
    std::string id;
    std::string userId;
    std::string cardId;
    std::string pinHash;
    CardStatus status;
    int failedAttempts;

public:
    Card() = default;

    Card(std::string id, std::string userId, std::string cardId, 
         std::string pinHash, CardStatus status, int failedAttempts);

    Card(std::string userId, std::string cardId, std::string pinHash);

    Card(const Card&) = default;
    Card& operator=(const Card&) = default;
    Card(Card&&) noexcept = default;
    Card& operator=(Card&&) noexcept = default;

    ~Card() = default;

    std::string_view getId() const noexcept;
    std::string_view getUserId() const noexcept;
    std::string_view getCardId() const noexcept;
    std::string_view getPinHash() const noexcept;
    CardStatus getStatus() const noexcept;
    int getFailedAttempts() const noexcept;

    void setStatus(CardStatus newStatus) noexcept;
    void setPinHash(std::string newPinHash);

    void incrementFailedAttempts() noexcept;
    void resetFailedAttempts() noexcept;

    static std::string statusToString(CardStatus status);
    static CardStatus stringToStatus(std::string_view str);
};