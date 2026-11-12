#pragma once

#include <memory>

class UserRepo;
class CardRepo;
class AccessPolicyRepo;
class AccessLogRecordRepo;
class CryptoService;

namespace UI {
    void runConsoleUI(
        std::shared_ptr<UserRepo> userRepo,
        std::shared_ptr<CardRepo> cardRepo,
        std::shared_ptr<AccessPolicyRepo> policyRepo,
        std::shared_ptr<AccessLogRecordRepo> logRepo,
        std::shared_ptr<CryptoService> cryptoService
    );
}
