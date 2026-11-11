#pragma once
#include <memory>
#include <functional>
#include <string>
#include <vector>
#include "ftxui/component/component.hpp"
#include "../models/UserRepo.h"
#include "../models/AccessPolicyRepo.h"
#include "../services/CryptoService.h"

namespace UI {

class AdminManageUsers {
public:
    AdminManageUsers(
        std::shared_ptr<UserRepo> userRepo,
        std::shared_ptr<AccessPolicyRepo> policyRepo,
        std::shared_ptr<CryptoService> cryptoService,
        std::function<void()> onBack
    );
    ftxui::Component getComponent() { return component_; }
    void reset();

private:
    ftxui::Component component_;

    std::shared_ptr<UserRepo> userRepo_;
    std::shared_ptr<AccessPolicyRepo> policyRepo_;
    std::shared_ptr<CryptoService> cryptoService_;
    std::function<void()> onBack_;

    int innerScreen_ = 0;
    std::string status_;

    // Submenu
    int subSel_ = 0;
    std::vector<std::string> subEntries_ = {
        "List All Users",
        "Create New User",
        "Assign Admin Role",
        "Assign Policy to User",
        "Back"
    };
    ftxui::Component subMenu_;

    // List Users
    int listActionSel_ = 0;
    std::vector<std::string> listActions_ = {"Back"};
    ftxui::Component listMenu_;

    // Create User
    std::string createFirstName_;
    std::string createLastName_;
    std::string createUsername_;
    std::string createPassword_;
    std::string createStatus_ = "Fill in details to create a new user.";
    int createActionSel_ = 0;
    std::vector<std::string> createActions_ = {"Create User", "Back"};
    ftxui::Component createFNInput_;
    ftxui::Component createLNInput_;
    ftxui::Component createUNInput_;
    ftxui::Component createPWInput_;
    ftxui::Component createActionMenu_;

    // Assign Role
    std::string assignRoleUserId_;
    std::string assignRoleStatus_ = "Enter the User ID to promote to Admin.";
    int assignRoleActionSel_ = 0;
    std::vector<std::string> assignRoleActions_ = {"Assign Admin Role", "Back"};
    ftxui::Component assignRoleInput_;
    ftxui::Component assignRoleMenu_;

    // Assign Policy
    std::string assignPolicyUserId_;
    std::string assignPolicyId_;
    std::string assignPolicyStatus_ = "Enter User ID and Policy ID to assign.";
    int assignPolicyActionSel_ = 0;
    std::vector<std::string> assignPolicyActions_ = {"Assign Policy", "Back"};
    ftxui::Component assignPolicyUserInput_;
    ftxui::Component assignPolicyInput_;
    ftxui::Component assignPolicyMenu_;

    ftxui::Component tabContainer_;
};

} // namespace UI
