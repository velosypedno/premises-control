#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include "ftxui/component/component.hpp"
#include "../services/CardCheck.h"
#include "../services/CryptoService.h"
#include "../models/CardRepo.h"
#include "../models/UserRepo.h"
#include "../models/AccessPolicyRepo.h"
#include "../models/AccessLogRecordRepo.h"
#include "../models/User.h"
#include "../models/Card.h"

namespace UI {

class UserView {
private:
    std::string cardIdInput = "";
    std::string cardPinInput = "";
    std::string timeInput = "10:00";
    int daySelected = 0; // Monday
    std::vector<std::string> dayEntries = {
        "Monday (MON)",
        "Tuesday (TUE)",
        "Wednesday (WED)",
        "Thursday (THU)",
        "Friday (FRI)",
        "Saturday (SAT)",
        "Sunday (SUN)"
    };
    bool useSystemTime = false;
    std::string statusMessage = "Enter card ID and PIN to authenticate.";

    std::shared_ptr<CardRepo> cardRepo_;
    std::shared_ptr<UserRepo> userRepo_;
    std::shared_ptr<AccessPolicyRepo> policyRepo_;
    std::shared_ptr<AccessLogRecordRepo> logRepo_;
    std::shared_ptr<CryptoService> cryptoService_;

    CardCheck cardCheckService_;
    std::function<void()> onBack_;
    std::function<void(const Card&, const User&)> onSuccess_;

    ftxui::Component cardIdBox_;
    ftxui::Component cardPinBox_;
    ftxui::Component dayDropdown_;
    ftxui::Component timeBox_;
    ftxui::Component systemTimeCheckbox_;
    ftxui::Component component;

public:
    UserView(std::shared_ptr<CardRepo> cardRepo,
             std::shared_ptr<UserRepo> userRepo,
             std::shared_ptr<AccessPolicyRepo> policyRepo,
             std::shared_ptr<AccessLogRecordRepo> logRepo,
             std::shared_ptr<CryptoService> cryptoService,
             std::function<void()> onBack,
             std::function<void(const Card&, const User&)> onSuccess = nullptr);

    ftxui::Component getComponent() { return component; }

    void resetStatus() {
        statusMessage = "Enter card ID and PIN to authenticate.";
        cardIdInput = "";
        cardPinInput = "";
        timeInput = "10:00";
        daySelected = 0;
        useSystemTime = false;
    }

    CardCheck& getCardCheckService() { return cardCheckService_; }
};

} // namespace UI
