#pragma once

#include <memory>
#include <string>
#include <vector>
#include <optional>

#include "Result.h"
#include "../models/Card.h"
#include "../models/CardRepo.h"
#include "../models/UserRepo.h"
#include "../models/AccessLogRecordRepo.h"
#include "CryptoService.h"

class CardService {
private:
    std::shared_ptr<CardRepo> cardRepo_;
    std::shared_ptr<UserRepo> userRepo_;
    std::shared_ptr<AccessLogRecordRepo> logRepo_;
    std::shared_ptr<CryptoService> cryptoService_;

    static std::string getCurrentTimestamp();
    void logEvent(const std::string& cardId, LogEventType eventType);

public:
    CardService(
        std::shared_ptr<CardRepo> cardRepo,
        std::shared_ptr<UserRepo> userRepo,
        std::shared_ptr<AccessLogRecordRepo> logRepo,
        std::shared_ptr<CryptoService> cryptoService
    );

    Services::Result<Card, Services::CardError> getCardByNumber(const std::string& cardNumber);

    std::vector<Card> listAllCards();

    std::vector<Card> getCardsByUserId(const std::string& userId);

    Services::Result<Card, Services::CardError> addCard(
        const std::string& userId,
        const std::string& cardNumber,
        const std::string& pin
    );

    Services::Result<void, Services::CardError> blockCard(const std::string& cardNumber);

    Services::Result<void, Services::CardError> unblockCard(const std::string& cardNumber);

    Services::Result<void, Services::CardError> revokeCard(const std::string& cardNumber);

    Services::Result<void, Services::CardError> changePinByAdmin(
        const std::string& cardNumber,
        const std::string& newPin
    );

    Services::Result<void, Services::CardError> changePinByUser(
        const std::string& cardNumber,
        const std::string& oldPin1,
        const std::string& oldPin2,
        const std::string& newPin,
        const std::string& confirmNewPin
    );
};
