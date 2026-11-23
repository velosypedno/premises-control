#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include "ftxui/component/component.hpp"
#include "../services/CardCheck.h"
#include "../models/User.h"
#include "../models/Card.h"

namespace UI {

class UserView {
private:
    std::string cardIdInput;
    std::string cardPinInput;
    std::string timeInput = "10:00";
    int daySelected = 0;
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
    std::string statusMessage = "Enter card number and PIN.";

    std::shared_ptr<CardCheck> cardCheckService_;
    std::function<void()> onBack_;
    std::function<void(const Card&, const User&)> onSuccess_;

    ftxui::Component cardIdBox_;
    ftxui::Component cardPinBox_;
    ftxui::Component dayDropdown_;
    ftxui::Component timeBox_;
    ftxui::Component systemTimeCheckbox_;
    ftxui::Component component_;

public:
    UserView(std::shared_ptr<CardCheck> cardCheck,
             std::function<void()> onBack,
             std::function<void(const Card&, const User&)> onSuccess = nullptr);

    ftxui::Component getComponent() { return component_; }

    void resetStatus() {
        statusMessage = "Enter card number and PIN.";
        cardIdInput.clear();
        cardPinInput.clear();
        timeInput = "10:00";
        daySelected = 0;
        useSystemTime = false;
    }
};

}
