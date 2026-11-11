#pragma once

#include <memory>
#include <string>
#include <functional>
#include "ftxui/component/component.hpp"
#include "../services/AdminCheck.h"
#include "../models/UserRepo.h"
#include "../services/CryptoService.h"

namespace UI {
    class AdminView {
    private:
        std::string username = "root";
        std::string password = "";
        std::string statusMessage = "Enter administrator credentials.";

        AdminCheck adminCheck;
        std::string rootPass;
        std::function<void()> onBack;
        std::function<void()> onSuccess;

        ftxui::Component component;

    public:
        AdminView(std::shared_ptr<UserRepo> userRepo,
                  std::shared_ptr<CryptoService> cryptoService,
                  const std::string& rootPass,
                  std::function<void()> onBack,
                  std::function<void()> onSuccess);

        ftxui::Component getComponent() { return component; }
        const std::string& getUsername() const { return username; }

        void resetStatus() {
            statusMessage = "Enter administrator credentials.";
            password = "";
        }
    };
}
