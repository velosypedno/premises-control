#pragma once

#include <string>
#include <memory>
#include <optional>
#include "../models/CardRepo.h"
#include "../models/UserRepo.h"
#include "../models/AccessPolicyRepo.h"
#include "../models/AccessLogRecordRepo.h"
#include "CryptoService.h"

class CardCheck {
private:
    std::shared_ptr<CardRepo> cardRepo;
    std::shared_ptr<UserRepo> userRepo;
    std::shared_ptr<AccessPolicyRepo> policyRepo;
    std::shared_ptr<AccessLogRecordRepo> logRepo;
    std::shared_ptr<CryptoService> cryptoService;

    int maxFailedAttempts;
    std::optional<LogEventType> lastEventType = std::nullopt;

    static std::string getCurrentTimestamp();
    void logEvent(const std::string& cardId, LogEventType eventType);

public:
    CardCheck(
        std::shared_ptr<CardRepo> cardRepo,
        std::shared_ptr<UserRepo> userRepo,
        std::shared_ptr<AccessPolicyRepo> policyRepo,
        std::shared_ptr<AccessLogRecordRepo> logRepo,
        std::shared_ptr<CryptoService> cryptoService,
        int maxFailedAttempts = 3
    );

    bool authenticate(const std::string& cardId, const std::string& pin, DayOfWeek currentDay, const std::string& currentTime);

    std::optional<LogEventType> getLastEventType() const noexcept { return lastEventType; }
    static std::string eventTypeToMessage(LogEventType event);
};
