#pragma once
#include <memory>
#include <functional>
#include <string>
#include <vector>
#include "ftxui/component/component.hpp"
#include "../models/CardRepo.h"
#include "../models/UserRepo.h"
#include "../services/CryptoService.h"

namespace UI {

class AdminManageCards {
public:
    AdminManageCards(
        std::shared_ptr<CardRepo> cardRepo,
        std::shared_ptr<UserRepo> userRepo,
        std::shared_ptr<CryptoService> cryptoService,
        std::function<void()> onBack
    );
    ftxui::Component getComponent() { return component_; }
    void reset();

private:
    ftxui::Component component_;

    std::shared_ptr<CardRepo> cardRepo_;
    std::shared_ptr<UserRepo> userRepo_;
    std::shared_ptr<CryptoService> cryptoService_;
    std::function<void()> onBack_;

    int innerScreen_ = 0;

    // Submenu
    int subSel_ = 0;
    std::vector<std::string> subEntries_ = {
        "List All Cards",
        "Add Card to User",
        "Block Card",
        "Unblock Card",
        "Back"
    };
    ftxui::Component subMenu_;

    // List Cards
    int listActionSel_ = 0;
    std::vector<std::string> listActions_ = {"Back"};
    ftxui::Component listMenu_;

    // Add Card
    std::string addUserId_;
    std::string addCardNum_;
    std::string addPin_;
    std::string addStatus_ = "Fill in details to add a card.";
    int addActionSel_ = 0;
    std::vector<std::string> addActions_ = {"Add Card", "Back"};
    ftxui::Component addUserIdInput_;
    ftxui::Component addCardNumInput_;
    ftxui::Component addPinInput_;
    ftxui::Component addActionMenu_;

    // Block Card
    std::string blockCardNum_;
    std::string blockStatus_ = "Enter card number to block.";
    int blockActionSel_ = 0;
    std::vector<std::string> blockActions_ = {"Block Card", "Back"};
    ftxui::Component blockCardInput_;
    ftxui::Component blockMenu_;

    // Unblock Card
    std::string unblockCardNum_;
    std::string unblockStatus_ = "Enter card number to unblock.";
    int unblockActionSel_ = 0;
    std::vector<std::string> unblockActions_ = {"Unblock Card", "Back"};
    ftxui::Component unblockCardInput_;
    ftxui::Component unblockMenu_;

    ftxui::Component tabContainer_;
};

} // namespace UI
