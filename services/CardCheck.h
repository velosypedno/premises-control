#pragma once

#include <string>
#include <memory>
#include "../models/CardRepo.h"
#include "../models/UserRepo.h"
#include "../models/AccessPolicyRepo.h"
#include "../models/AccessLogRecordRepo.h"
#include "CryptoService.h"
#include "Result.h"

struct AuthSuccess {
    Card card;
    User user;
};

enum class AuthError {
    WrongPin,
    Blocked,
    Alarm,
    RevokedCard,
    CardDoesNotExist,
    PolicyNotSet,
    TimeIsNotAllowed,
    DayIsNotAllowed,
    UserInactive,
    PersistFailed
};

std::string toErrorMessage(AuthError err);

class CardCheck {
private:
    std::shared_ptr<CardRepo> cardRepo_;
    std::shared_ptr<UserRepo> userRepo_;
    std::shared_ptr<AccessPolicyRepo> policyRepo_;
    std::shared_ptr<AccessLogRecordRepo> logRepo_;
    std::shared_ptr<CryptoService> cryptoService_;
    int maxFailedAttempts_;

    void recordAudit(const std::string& timestamp, const std::string& cardId, LogEventType eventType) const;

    Services::Result<AuthSuccess, AuthError> authenticateAt(
        const std::string& cardId,
        const std::string& pin,
        DayOfWeek day,
        const std::string& time,
        const std::string& auditTimestamp
    ) const;

public:
    CardCheck(
        std::shared_ptr<CardRepo> cardRepo,
        std::shared_ptr<UserRepo> userRepo,
        std::shared_ptr<AccessPolicyRepo> policyRepo,
        std::shared_ptr<AccessLogRecordRepo> logRepo,
        std::shared_ptr<CryptoService> cryptoService,
        int maxFailedAttempts = 3
    );

    Services::Result<AuthSuccess, AuthError> authenticate(
        const std::string& cardId,
        const std::string& pin
    ) const;

    Services::Result<AuthSuccess, AuthError> authenticate(
        const std::string& cardId,
        const std::string& pin,
        DayOfWeek day,
        const std::string& time
    ) const;

    void logCancellation(const std::string& cardId) const;
};

using CardCheckService = CardCheck;
