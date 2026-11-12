#pragma once

#include <memory>
#include <functional>
#include <string>
#include <vector>
#include "ftxui/component/component.hpp"
#include "../models/User.h"
#include "../models/Card.h"
#include "../models/CardRepo.h"

namespace UI {

class UserCardsView {
public:
    UserCardsView(
        std::shared_ptr<CardRepo> cardRepo,
        User* currentUser,
        Card* currentCard,
        std::function<void()> onBack
    );

    ftxui::Component getComponent() { return component_; }
    void loadSession(User* user, Card* card);

private:
    ftxui::Component component_;

    std::shared_ptr<CardRepo> cardRepo_;
    User* currentUser_ = nullptr;
    Card* currentCard_ = nullptr;
    std::function<void()> onBack_;

    std::vector<Card> cachedCards_;
    std::string revokeCardIdInput_;
    std::string statusMessage_;

    int selected_ = 0;
    std::vector<std::string> entries_ = {"Revoke Card", "Refresh", "Back"};

    ftxui::Component revokeInput_;
    ftxui::Component menu_;
    ftxui::Component container_;
};

} // namespace UI
