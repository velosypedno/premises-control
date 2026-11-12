#pragma once

#include <memory>
#include <functional>
#include <string>
#include <vector>
#include "ftxui/component/component.hpp"
#include "../models/User.h"
#include "../models/Card.h"
#include "../models/CardRepo.h"
#include "../services/CryptoService.h"

namespace UI {

class UserChangePin {
public:
    UserChangePin(
        std::shared_ptr<CardRepo> cardRepo,
        std::shared_ptr<CryptoService> cryptoService,
        User* currentUser,
        Card* currentCard,
        std::function<void()> onBack
    );

    ftxui::Component getComponent() { return component_; }
    void loadSession(User* user, Card* card);

private:
    ftxui::Component component_;

    std::shared_ptr<CardRepo> cardRepo_;
    std::shared_ptr<CryptoService> cryptoService_;
    User* currentUser_ = nullptr;
    Card* currentCard_ = nullptr;
    std::function<void()> onBack_;

    std::string oldPin1_;
    std::string oldPin2_;
    std::string newPin_;
    std::string confirmNewPin_;
    std::string statusMessage_;

    int selected_ = 0;
    std::vector<std::string> entries_ = {"Change PIN", "Back"};

    ftxui::Component oldPinInput1_;
    ftxui::Component oldPinInput2_;
    ftxui::Component newPinInput_;
    ftxui::Component confirmNewPinInput_;
    ftxui::Component menu_;
    ftxui::Component container_;
};

} // namespace UI
