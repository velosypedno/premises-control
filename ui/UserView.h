#pragma once

#include <memory>
#include <string>
#include <functional>
#include "ftxui/component/component.hpp"
#include "../services/CardCheck.h"

namespace UI {
    class UserView {
    private:
        std::string cardIdInput = "";
        std::string cardPinInput = "";
        std::string statusMessage = "Enter card ID and PIN code.";

        CardCheck cardCheckService;
        std::function<void()> onBack;

        ftxui::Component component;

    public:
        UserView(std::shared_ptr<CardRepo> cardRepo,
                 std::shared_ptr<UserRepo> userRepo,
                 std::shared_ptr<AccessPolicyRepo> policyRepo,
                 std::shared_ptr<AccessLogRecordRepo> logRepo,
                 std::shared_ptr<CryptoService> cryptoService,
                 std::function<void()> onBack);

        ftxui::Component getComponent() { return component; }

        void resetStatus() {
            statusMessage = "Enter card ID and PIN code.";
            cardIdInput = "";
            cardPinInput = "";
        }

        CardCheck& getCardCheckService() { return cardCheckService; }
    };
}
