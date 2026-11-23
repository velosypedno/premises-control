#pragma once
#include <memory>
#include <functional>
#include <string>
#include <vector>
#include "ftxui/component/component.hpp"
#include "../services/CardService.h"

namespace UI {

class AdminManageCards {
public:
    AdminManageCards(
        std::shared_ptr<CardService> cardService,
        std::function<void()> onBack
    );
    ftxui::Component getComponent() { return component_; }
    void reset();

private:
    ftxui::Component component_;

    std::shared_ptr<CardService> cardService_;
    std::function<void()> onBack_;

    int innerScreen_ = 0;

    // Submenu
    int subSel_ = 0;
    std::vector<std::string> subEntries_ = {
        "List All Cards",
        "Add Card to User",
        "Block Card",
        "Unblock Card",
        "Revoke Card",
        "Change PIN",
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

    // Revoke Card
    std::string revokeCardNum_;
    std::string revokeStatus_ = "Enter card number to revoke.";
    bool revokeConfirmPending_ = false;
    int revokeActionSel_ = 0;
    std::vector<std::string> revokeActions_ = {"Revoke Card", "Back"};
    std::vector<std::string> revokeConfirmActions_ = {"Confirm Revoke", "Cancel"};
    ftxui::Component revokeCardInput_;
    ftxui::Component revokeMenu_;
    ftxui::Component revokeConfirmMenu_;

    // Change PIN
    std::string changePinCardNum_;
    std::string changePinNew_;
    std::string changePinStatus_ = "Enter card number and new PIN.";
    int changePinActionSel_ = 0;
    std::vector<std::string> changePinActions_ = {"Change PIN", "Back"};
    ftxui::Component changePinCardInput_;
    ftxui::Component changePinInput_;
    ftxui::Component changePinMenu_;

    ftxui::Component tabContainer_;
};

} // namespace UI
