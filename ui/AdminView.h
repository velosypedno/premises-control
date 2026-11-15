#pragma once

#include <memory>
#include <string>
#include <functional>
#include <optional>
#include "ftxui/component/component.hpp"
#include "../services/AdminCheck.h"
#include "../models/UserRepo.h"
#include "../models/CardRepo.h"
#include "../models/User.h"
#include "../services/CryptoService.h"

namespace UI {
    class AdminView {
    private:
        std::string cardIdInput = "";
        std::string pinInput = "";
        std::string statusMessage = "Enter ADMIN card number and PIN.";
        std::optional<User> authenticatedUser_;

        AdminCheck adminCheck;
        std::function<void()> onBack;
        std::function<void(const User&)> onSuccess;

        ftxui::Component component;

    public:
        AdminView(std::shared_ptr<UserRepo> userRepo,
                  std::shared_ptr<CardRepo> cardRepo,
                  std::shared_ptr<CryptoService> cryptoService,
                  std::function<void()> onBack,
                  std::function<void(const User&)> onSuccess);

        ftxui::Component getComponent() { return component; }
        const std::optional<User>& getAuthenticatedUser() const { return authenticatedUser_; }

        void resetStatus() {
            statusMessage = "Enter ADMIN card number and PIN.";
            cardIdInput = "";
            pinInput = "";
            authenticatedUser_.reset();
        }
    };
}
