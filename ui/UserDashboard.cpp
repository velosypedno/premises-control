#include "UserDashboard.h"
#include "UserEditProfile.h"
#include "UserChangePin.h"
#include "UserCardsView.h"
#include "UserPolicyView.h"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"

using namespace ftxui;

namespace UI {

void UserDashboard::setCurrentSession(Card card, User user) {
    currentCard_ = std::move(card);
    currentUser_ = std::move(user);

    if (editProfile_) editProfile_->loadUser(&currentUser_);
    if (changePin_) changePin_->loadSession(&currentUser_, &currentCard_);
    if (cardsView_) cardsView_->loadSession(&currentUser_, &currentCard_);
    if (policyView_) policyView_->loadSession(&currentUser_);
}

void UserDashboard::resetStatus() {
    selectedTab_ = 0;
    mainSelected_ = 0;
}

UserDashboard::UserDashboard(
    std::function<void()> onLogout,
    std::shared_ptr<UserRepo> userRepo,
    std::shared_ptr<CardRepo> cardRepo,
    std::shared_ptr<AccessPolicyRepo> policyRepo,
    std::shared_ptr<CryptoService> cryptoService
) : onLogout_(onLogout),
    userRepo_(userRepo),
    cardRepo_(cardRepo),
    policyRepo_(policyRepo),
    cryptoService_(cryptoService) {

    menuEntries_ = {
        "1. Edit My Profile (Name & Surname)",
        "2. Change Card PIN Code",
        "3. My Cards & Revocation",
        "4. My Access Schedule & Policy",
        "5. Logout"
    };

    mainMenu_ = Menu(&menuEntries_, &mainSelected_);

    auto goBack = [this]() { selectedTab_ = 0; };

    editProfile_ = std::make_shared<UserEditProfile>(userRepo_, &currentUser_, goBack);
    changePin_ = std::make_shared<UserChangePin>(cardRepo_, cryptoService_, &currentUser_, &currentCard_, goBack);
    cardsView_ = std::make_shared<UserCardsView>(cardRepo_, &currentUser_, &currentCard_, goBack);
    policyView_ = std::make_shared<UserPolicyView>(policyRepo_, &currentUser_, goBack);

    auto mainMenuHandler = CatchEvent(mainMenu_, [this](Event event) {
        if (event == Event::Return) {
            int sel = mainSelected_;
            if (sel == 4) {
                if (onLogout_) onLogout_();
            } else if (sel == 0) {
                if (editProfile_) editProfile_->loadUser(&currentUser_);
                selectedTab_ = 1;
            } else if (sel == 1) {
                if (changePin_) changePin_->loadSession(&currentUser_, &currentCard_);
                selectedTab_ = 2;
            } else if (sel == 2) {
                if (cardsView_) cardsView_->loadSession(&currentUser_, &currentCard_);
                selectedTab_ = 3;
            } else if (sel == 3) {
                if (policyView_) policyView_->loadSession(&currentUser_);
                selectedTab_ = 4;
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
        std::string welcome = "Welcome, " + currentUser_.getFullName();
        std::string cardInfo = "Session Card: " + std::string(currentCard_.getCardId());

        return vbox({
            text("=== USER CONTROL PANEL ===") | bold | color(Color::Cyan) | center,
            separator(),
            hbox({
                text(welcome) | bold | color(Color::Green),
                filler(),
                text(cardInfo) | dim,
            }),
            separator(),
            mainMenu_->Render() | border,
            separator(),
            text("[Enter] Select  |  [Esc] Logout") | dim | center,
        }) | border | size(WIDTH, GREATER_THAN, 62);
    });

    tabContainer_ = Container::Tab({
        mainMenuRenderer,
        editProfile_->getComponent(),
        changePin_->getComponent(),
        cardsView_->getComponent(),
        policyView_->getComponent(),
    }, &selectedTab_);

    component_ = tabContainer_;
}

} // namespace UI
