#pragma once

#include <memory>
#include <functional>
#include <string>
#include <vector>
#include "ftxui/component/component.hpp"
#include "../models/User.h"
#include "../models/UserRepo.h"

namespace UI {

class UserEditProfile {
public:
    UserEditProfile(
        std::shared_ptr<UserRepo> userRepo,
        User* currentUser,
        std::function<void()> onBack
    );

    ftxui::Component getComponent() { return component_; }
    void loadUser(User* user);

private:
    ftxui::Component component_;

    std::shared_ptr<UserRepo> userRepo_;
    User* currentUser_ = nullptr;
    std::function<void()> onBack_;

    std::string firstNameVal_;
    std::string lastNameVal_;
    std::string statusMessage_;

    int selected_ = 0;
    std::vector<std::string> entries_ = {"Save Changes", "Back"};

    ftxui::Component firstNameInput_;
    ftxui::Component lastNameInput_;
    ftxui::Component menu_;
    ftxui::Component container_;
};

} // namespace UI
