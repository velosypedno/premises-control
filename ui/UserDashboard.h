#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "ftxui/component/component.hpp"
#include "../models/User.h"
#include "../models/Card.h"
#include "../models/UserRepo.h"
#include "../models/CardRepo.h"
#include "../models/AccessPolicyRepo.h"
#include "../services/CryptoService.h"

namespace UI {

class UserEditProfile;
class UserChangePin;
class UserCardsView;
class UserPolicyView;

class UserDashboard {
public:
    UserDashboard(
        std::function<void()> onLogout,
        std::shared_ptr<UserRepo> userRepo,
        std::shared_ptr<CardRepo> cardRepo,
        std::shared_ptr<AccessPolicyRepo> policyRepo,
        std::shared_ptr<CryptoService> cryptoService
    );

    ftxui::Component getComponent() { return component_; }
    void setCurrentSession(Card card, User user);
    void resetStatus();

private:
    ftxui::Component component_;

    std::shared_ptr<UserRepo> userRepo_;
    std::shared_ptr<CardRepo> cardRepo_;
    std::shared_ptr<AccessPolicyRepo> policyRepo_;
    std::shared_ptr<CryptoService> cryptoService_;
    std::function<void()> onLogout_;

    User currentUser_;
    Card currentCard_;
    int selectedTab_ = 0;
    int mainSelected_ = 0;

    std::vector<std::string> menuEntries_;
    ftxui::Component mainMenu_;
    ftxui::Component tabContainer_;

    std::shared_ptr<UserEditProfile> editProfile_;
    std::shared_ptr<UserChangePin> changePin_;
    std::shared_ptr<UserCardsView> cardsView_;
    std::shared_ptr<UserPolicyView> policyView_;
};

} // namespace UI
