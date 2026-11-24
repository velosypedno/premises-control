#pragma once

#include <memory>
#include <string>
#include <functional>
#include "ftxui/component/component.hpp"
#include "../services/AdminCheck.h"
#include "../models/User.h"

namespace UI {

class AdminView {
private:
    std::string usernameInput_;
    std::string passwordInput_;
    std::string statusMessage_ = "Enter administrator username and password.";

    std::shared_ptr<AdminCheck> adminCheck_;
    std::function<void()> onBack_;
    std::function<void(const User&)> onSuccess_;

    ftxui::Component usernameBox_;
    ftxui::Component passwordBox_;
    ftxui::Component component_;

public:
    AdminView(std::shared_ptr<AdminCheck> adminCheck,
              std::function<void()> onBack,
              std::function<void(const User&)> onSuccess);

    ftxui::Component getComponent() { return component_; }

    void resetStatus() {
        statusMessage_ = "Enter administrator username and password.";
        usernameInput_.clear();
        passwordInput_.clear();
    }
};

}
