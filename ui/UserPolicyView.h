#pragma once

#include <memory>
#include <functional>
#include <string>
#include <vector>
#include "ftxui/component/component.hpp"
#include "../models/User.h"
#include "../models/AccessPolicy.h"
#include "../models/AccessPolicyRepo.h"

namespace UI {

class UserPolicyView {
public:
    UserPolicyView(
        std::shared_ptr<AccessPolicyRepo> policyRepo,
        User* currentUser,
        std::function<void()> onBack
    );

    ftxui::Component getComponent() { return component_; }
    void loadSession(User* user);

private:
    ftxui::Component component_;

    std::shared_ptr<AccessPolicyRepo> policyRepo_;
    User* currentUser_ = nullptr;
    std::function<void()> onBack_;

    std::optional<AccessPolicy> cachedPolicy_;
    std::string errorMessage_;

    int selected_ = 0;
    std::vector<std::string> entries_ = {"Back"};

    ftxui::Component menu_;
    ftxui::Component container_;
};

} // namespace UI
