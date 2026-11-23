#pragma once

#include <memory>

class UserRepo;
class CardRepo;
class AccessPolicyRepo;
class AccessLogRecordRepo;
class AdminCheck;
class CardCheck;
class UserService;
class CardService;

namespace UI {

    struct AppDependencies {
        std::shared_ptr<UserService> userService;
        std::shared_ptr<CardService> cardService;
        std::shared_ptr<AdminCheck> adminCheck;
        std::shared_ptr<CardCheck> cardCheck;
        std::shared_ptr<UserRepo> userRepo;
        std::shared_ptr<CardRepo> cardRepo;
        std::shared_ptr<AccessPolicyRepo> policyRepo;
        std::shared_ptr<AccessLogRecordRepo> logRepo;
    };

    void runConsoleUI(const AppDependencies& deps);

} // namespace UI
