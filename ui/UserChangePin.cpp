#include "UserChangePin.h"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include <algorithm>
#include <cctype>

using namespace ftxui;

namespace UI {

void UserChangePin::loadSession(User* user, Card* card) {
    currentUser_ = user;
    currentCard_ = card;
    oldPin1_.clear();
    oldPin2_.clear();
    newPin_.clear();
    confirmNewPin_.clear();
    statusMessage_.clear();
    selected_ = 0;
}

UserChangePin::UserChangePin(
    std::shared_ptr<CardRepo> cardRepo,
    std::shared_ptr<CryptoService> cryptoService,
    User* currentUser,
    Card* currentCard,
    std::function<void()> onBack
) : cardRepo_(cardRepo),
    cryptoService_(cryptoService),
    currentUser_(currentUser),
    currentCard_(currentCard),
    onBack_(onBack) {

    loadSession(currentUser_, currentCard_);

    InputOption passOpt;
    passOpt.password = true;

    oldPinInput1_ = Input(&oldPin1_, "Current PIN (1st entry)", passOpt);
    oldPinInput2_ = Input(&oldPin2_, "Current PIN (confirm)", passOpt);
    newPinInput_ = Input(&newPin_, "New PIN (4-8 digits)", passOpt);
    confirmNewPinInput_ = Input(&confirmNewPin_, "Confirm New PIN", passOpt);

    menu_ = Menu(&entries_, &selected_);

    container_ = Container::Vertical({
        oldPinInput1_,
        oldPinInput2_,
        newPinInput_,
        confirmNewPinInput_,
        menu_,
    });

    auto handler = CatchEvent(container_, [this](Event event) {
        if (event == Event::Return) {
            if (menu_->Focused() && selected_ == 1) {
                if (onBack_) onBack_();
                return true;
            }

            if ((menu_->Focused() && selected_ == 0) || !menu_->Focused()) {
                if (!currentCard_) {
                    statusMessage_ = "[Error]: No active card session.";
                    return true;
                }

                if (oldPin1_.empty() || oldPin2_.empty() || newPin_.empty() || confirmNewPin_.empty()) {
                    statusMessage_ = "[Error]: All PIN fields are required.";
                    return true;
                }

                if (oldPin1_ != oldPin2_) {
                    statusMessage_ = "[Error]: The two old PIN entries do not match.";
                    return true;
                }

                std::string oldHash = cryptoService_->hashPin(oldPin1_);
                if (oldHash != currentCard_->getPinHash()) {
                    statusMessage_ = "[Error]: Current PIN code is incorrect.";
                    return true;
                }

                bool allDigits = std::all_of(newPin_.begin(), newPin_.end(), [](unsigned char c) {
                    return std::isdigit(c);
                });

                if (!allDigits || newPin_.size() < 4 || newPin_.size() > 8) {
                    statusMessage_ = "[Error]: New PIN must be between 4 and 8 digits (numeric only).";
                    return true;
                }

                if (newPin_ == oldPin1_) {
                    statusMessage_ = "[Error]: New PIN must be different from the current PIN.";
                    return true;
                }

                if (newPin_ != confirmNewPin_) {
                    statusMessage_ = "[Error]: New PIN and confirmation do not match.";
                    return true;
                }

                try {
                    std::string newHash = cryptoService_->hashPin(newPin_);
                    currentCard_->setPinHash(newHash);
                    cardRepo_->update(*currentCard_);

                    oldPin1_.clear();
                    oldPin2_.clear();
                    newPin_.clear();
                    confirmNewPin_.clear();
                    statusMessage_ = "[Success]: Card PIN code changed successfully!";
                } catch (const std::exception& e) {
                    statusMessage_ = std::string("[Error]: ") + e.what();
                }
                return true;
            }
        }
        if (event == Event::Escape) {
            if (onBack_) onBack_();
            return true;
        }
        return false;
    });

    component_ = Renderer(handler, [this]() {
        auto statusColor = (statusMessage_.rfind("[Success]", 0) == 0) ? Color::Green : Color::Red;

        Elements elements;
        elements.push_back(text("=== CHANGE CARD PIN CODE ===") | bold | color(Color::Cyan) | center);
        elements.push_back(separator());

        if (currentCard_) {
            elements.push_back(hbox({
                text("Active Card ID: ") | bold,
                text(std::string(currentCard_->getCardId())) | color(Color::Green) | bold,
            }));
            elements.push_back(separatorLight());
        }

        elements.push_back(text("Step 1: Verify Current PIN (Enter twice):") | bold);
        elements.push_back(oldPinInput1_->Render() | border);
        elements.push_back(oldPinInput2_->Render() | border);
        elements.push_back(separatorLight());

        elements.push_back(text("Step 2: Enter New PIN (4 - 8 digits):") | bold);
        elements.push_back(newPinInput_->Render() | border);
        elements.push_back(text("Confirm New PIN:") | bold);
        elements.push_back(confirmNewPinInput_->Render() | border);

        elements.push_back(separator());
        elements.push_back(menu_->Render() | border);

        if (!statusMessage_.empty()) {
            elements.push_back(separator());
            elements.push_back(text(statusMessage_) | color(statusColor) | center);
        }

        elements.push_back(separator());
        elements.push_back(text("[Enter] Submit/Select  |  [Esc] Back") | dim | center);

        return vbox(std::move(elements)) | border | size(WIDTH, GREATER_THAN, 62);
    });
}

} // namespace UI
