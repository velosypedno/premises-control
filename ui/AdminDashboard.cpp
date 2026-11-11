#include "AdminDashboard.h"
#include "AdminEditProfile.h"
#include "AdminManageUsers.h"
#include "AdminManageCards.h"
#include "AdminManagePolicies.h"
#include "AdminAccessLog.h"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"

using namespace ftxui;

namespace UI {

void AdminDashboard::setCurrentUser(User user) {
    currentUser_ = std::move(user);
    if (editProfile_) {
        editProfile_->loadUser();
    }
}

void AdminDashboard::resetStatus() {
    selectedTab_ = 0;
    mainSelected_ = 0;
    if (manageUsers_) manageUsers_->reset();
    if (manageCards_) manageCards_->reset();
    if (managePolicies_) managePolicies_->reset();
}

AdminDashboard::AdminDashboard(
    std::function<void()> onLogout,
    std::shared_ptr<UserRepo> userRepo,
    std::shared_ptr<CardRepo> cardRepo,
    std::shared_ptr<AccessPolicyRepo> policyRepo,
    std::shared_ptr<AccessLogRecordRepo> logRepo,
    std::shared_ptr<CryptoService> cryptoService
) : onLogout_(onLogout),
    userRepo_(userRepo),
    cardRepo_(cardRepo),
    policyRepo_(policyRepo),
    logRepo_(logRepo),
    cryptoService_(cryptoService)
{
    menuEntries_ = {
        "1. Edit My Profile",
        "2. Manage Users",
        "3. Manage Cards",
        "4. Manage Access Policies",
        "5. System Access Logs",
        "6. Logout"
    };

    mainMenu_ = Menu(&menuEntries_, &mainSelected_);

    auto goBack = [this]() { selectedTab_ = 0; };

    editProfile_ = std::make_shared<AdminEditProfile>(userRepo_, cryptoService_, &currentUser_, goBack);
    manageUsers_ = std::make_shared<AdminManageUsers>(userRepo_, policyRepo_, cryptoService_, goBack);
    manageCards_ = std::make_shared<AdminManageCards>(cardRepo_, userRepo_, cryptoService_, goBack);
    managePolicies_ = std::make_shared<AdminManagePolicies>(policyRepo_, userRepo_, goBack);
    accessLog_ = std::make_shared<AdminAccessLog>(logRepo_, goBack);

    auto mainMenuHandler = CatchEvent(mainMenu_, [this](Event event) {
        if (event == Event::Return) {
            int sel = mainSelected_;
            if (sel == 5) {
                if (onLogout_) onLogout_();
            } else if (sel == 0) {
                if (editProfile_) {
                    editProfile_->loadUser();
                }
                selectedTab_ = 1; // Edit My Profile
            } else if (sel == 1) {
                if (manageUsers_) manageUsers_->reset();
                selectedTab_ = 2; // Manage Users
            } else if (sel == 2) {
                if (manageCards_) manageCards_->reset();
                selectedTab_ = 3; // Manage Cards
            } else if (sel == 3) {
                if (managePolicies_) managePolicies_->reset();
                selectedTab_ = 4; // Manage Policies
            } else if (sel == 4) {
                if (accessLog_) accessLog_->loadLogs();
                selectedTab_ = 5; // Access Log
            }
            return true;
        }
        if (event == Event::Escape) {
            if (onLogout_) onLogout_();
            return true;
        }
        return false;
    });

    auto mainMenuRenderer = Renderer(mainMenuHandler, [this]() {
        return vbox({
            text("=== ADMIN CONTROL PANEL ===") | bold | color(Color::Green) | center,
            separator(),
            mainMenu_->Render() | border,
            separator(),
            text("[Enter] - Select  |  [Esc] - Logout") | dim | center,
        }) | border | size(WIDTH, GREATER_THAN, 60);
    });

    tabContainer_ = Container::Tab({
        mainMenuRenderer,
        editProfile_->getComponent(),
        manageUsers_->getComponent(),
        manageCards_->getComponent(),
        managePolicies_->getComponent(),
        accessLog_->getComponent(),
    }, &selectedTab_);

    component_ = tabContainer_;
}

} // namespace UI
