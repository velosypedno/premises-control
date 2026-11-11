#pragma once

#include <memory>
#include <functional>
#include <string>
#include <vector>
#include "ftxui/component/component.hpp"
#include "../models/UserRepo.h"
#include "../services/CryptoService.h"

namespace UI {

class AdminEditProfile {
public:
    AdminEditProfile(
        std::shared_ptr<UserRepo> userRepo,
        std::shared_ptr<CryptoService> cryptoService,
        User* currentUser,
        std::function<void()> onBack
    );

    ftxui::Component getComponent() { return component_; }
    void loadUser();

private:
    ftxui::Component component_;

    std::shared_ptr<UserRepo> userRepo_;
    std::shared_ptr<CryptoService> cryptoService_;
    User* currentUser_ = nullptr;
    std::function<void()> onBack_;

    std::string firstNameVal_;
    std::string lastNameVal_;
    std::string usernameVal_;
    std::string passwordVal_;
    std::string statusMessage_;

    int selected_ = 0;
    std::vector<std::string> entries_ = {"Save", "Back"};

    ftxui::Component firstNameInput_;
    ftxui::Component lastNameInput_;
    ftxui::Component usernameInput_;
    ftxui::Component passwordInput_;
    ftxui::Component menu_;
    ftxui::Component container_;
};

} // namespace UI
