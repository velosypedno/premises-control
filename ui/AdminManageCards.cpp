#include "AdminManageCards.h"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

using namespace ftxui;

namespace UI {

namespace {
std::string nowTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}
} // namespace

void AdminManageCards::reset() {
    innerScreen_ = 0;
    subSel_ = 0;
    addUserId_.clear();
    addCardNum_.clear();
    addPin_.clear();
    addStatus_ = "Fill in details to add a card.";
    blockCardNum_.clear();
    blockStatus_ = "Enter card number to block.";
    unblockCardNum_.clear();
    unblockStatus_ = "Enter card number to unblock.";
    revokeCardNum_.clear();
    revokeStatus_ = "Enter card number to revoke.";
    revokeConfirmPending_ = false;
    changePinCardNum_.clear();
    changePinNew_.clear();
    changePinStatus_ = "Enter card number and new PIN.";
}

AdminManageCards::AdminManageCards(
    std::shared_ptr<CardService> cardService,
    std::function<void()> onBack
) : cardService_(std::move(cardService)),
    onBack_(std::move(onBack)) {

    subMenu_ = Menu(&subEntries_, &subSel_);

    auto subMenuHandler = CatchEvent(subMenu_, [this](Event event) {
        if (event == Event::Return) {
            int sel = subSel_;
            if (sel == 6) {
                if (onBack_) onBack_();
            } else {
                innerScreen_ = sel + 1;
            }
            return true;
        }
        if (event == Event::Escape) {
            if (onBack_) onBack_();
            return true;
        }
        return false;
    });

    auto subMenuRenderer = Renderer(subMenuHandler, [this]() {
        return vbox({
            text("=== MANAGE CARDS ===") | bold | color(Color::Green) | center,
            separator(),
            subMenu_->Render() | border,
            separator(),
            text("[Enter] Select  |  [Esc] Back") | dim | center,
        }) | border | size(WIDTH, GREATER_THAN, 60);
    });

    listMenu_ = Menu(&listActions_, &listActionSel_);
    auto listHandler = CatchEvent(listMenu_, [this](Event event) {
        if (event == Event::Return || event == Event::Escape) {
            innerScreen_ = 0;
            return true;
        }
        return false;
    });

    auto listRenderer = Renderer(listHandler, [this]() {
        Elements rows;
        rows.push_back(text("=== LIST ALL CARDS ===") | bold | color(Color::Green) | center);
        rows.push_back(separator());
        rows.push_back(text("CardID | UserID | Status | FailedAttempts") | bold);
        rows.push_back(separator());
        try {
            auto cards = cardService_->listAllCards();
            if (cards.empty()) {
                rows.push_back(text("No cards found.") | dim | center);
            } else {
                for (auto& c : cards) {
                    std::string line =
                        std::string(c.getCardId()) + " | " +
                        std::string(c.getUserId()) + " | " +
                        Card::statusToString(c.getStatus()) + " | " +
                        std::to_string(c.getFailedAttempts());
                    rows.push_back(text(line));
                }
            }
        } catch (const std::exception& e) {
            rows.push_back(text(std::string("[Error] ") + e.what()) | color(Color::Red));
        }
        rows.push_back(separator());
        rows.push_back(listMenu_->Render() | border);
        rows.push_back(separator());
        rows.push_back(text("[Enter] Select  |  [Esc] Back") | dim | center);
        return vbox(std::move(rows)) | border | size(WIDTH, GREATER_THAN, 60);
    });

    InputOption pinOpt;
    pinOpt.password = true;

    addUserIdInput_ = Input(&addUserId_, "User ID");
    addCardNumInput_ = Input(&addCardNum_, "Card Number");
    addPinInput_ = Input(&addPin_, "PIN", pinOpt);
    addActionMenu_ = Menu(&addActions_, &addActionSel_);

    auto addContainer = Container::Vertical({
        addUserIdInput_,
        addCardNumInput_,
        addPinInput_,
        addActionMenu_,
    });

    auto addHandler = CatchEvent(addContainer, [this](Event event) {
        if (event == Event::Return) {
            if (addActionMenu_->Focused() && addActionSel_ == 1) {
                innerScreen_ = 0;
                return true;
            }
            if ((addActionMenu_->Focused() && addActionSel_ == 0) || !addActionMenu_->Focused()) {
                auto res = cardService_->addCard(addUserId_, addCardNum_, addPin_);
                if (res.is_ok()) {
                    addStatus_ = "[Success] Card added.";
                    addUserId_.clear();
                    addCardNum_.clear();
                    addPin_.clear();
                } else {
                    addStatus_ = "[Error] " + Services::toErrorMessage(res.error());
                }
                return true;
            }
        }
        if (event == Event::Escape) {
            innerScreen_ = 0;
            return true;
        }
        return false;
    });

    auto addRenderer = Renderer(addHandler, [this]() {
        auto statusCol = (addStatus_.rfind("[Success]", 0) == 0) ? Color::Green :
                         (addStatus_.rfind("[Error]", 0) == 0) ? Color::Red : Color::Yellow;
        return vbox({
            text("=== ADD CARD TO USER ===") | bold | color(Color::Green) | center,
            separator(),
            text("User ID:"),
            addUserIdInput_->Render() | border,
            text("Card Number:"),
            addCardNumInput_->Render() | border,
            text("PIN:"),
            addPinInput_->Render() | border,
            separator(),
            addActionMenu_->Render() | border,
            separator(),
            text(addStatus_) | color(statusCol) | center,
            separator(),
            text("[Esc] Back") | dim | center,
        }) | border | size(WIDTH, GREATER_THAN, 60);
    });

    blockCardInput_ = Input(&blockCardNum_, "Card Number");
    blockMenu_ = Menu(&blockActions_, &blockActionSel_);
    auto blockContainer = Container::Vertical({blockCardInput_, blockMenu_});

    auto blockHandler = CatchEvent(blockContainer, [this](Event event) {
        if (event == Event::Return) {
            if (blockMenu_->Focused() && blockActionSel_ == 1) {
                innerScreen_ = 0;
                return true;
            }
            if ((blockMenu_->Focused() && blockActionSel_ == 0) || !blockMenu_->Focused()) {
                auto res = cardService_->blockCard(blockCardNum_);
                if (res.is_ok()) {
                    blockStatus_ = "[Success] Card blocked.";
                    blockCardNum_.clear();
                } else {
                    blockStatus_ = "[Error] " + Services::toErrorMessage(res.error());
                }
                return true;
            }
        }
        if (event == Event::Escape) {
            innerScreen_ = 0;
            return true;
        }
        return false;
    });

    auto blockRenderer = Renderer(blockHandler, [this]() {
        auto statusCol = (blockStatus_.rfind("[Success]", 0) == 0) ? Color::Green :
                         (blockStatus_.rfind("[Error]", 0) == 0) ? Color::Red : Color::Yellow;
        return vbox({
            text("=== BLOCK CARD ===") | bold | color(Color::Green) | center,
            separator(),
            text("Card Number:"),
            blockCardInput_->Render() | border,
            separator(),
            blockMenu_->Render() | border,
            separator(),
            text(blockStatus_) | color(statusCol) | center,
            separator(),
            text("[Esc] Back") | dim | center,
        }) | border | size(WIDTH, GREATER_THAN, 60);
    });

    unblockCardInput_ = Input(&unblockCardNum_, "Card Number");
    unblockMenu_ = Menu(&unblockActions_, &unblockActionSel_);
    auto unblockContainer = Container::Vertical({unblockCardInput_, unblockMenu_});

    auto unblockHandler = CatchEvent(unblockContainer, [this](Event event) {
        if (event == Event::Return) {
            if (unblockMenu_->Focused() && unblockActionSel_ == 1) {
                innerScreen_ = 0;
                return true;
            }
            if ((unblockMenu_->Focused() && unblockActionSel_ == 0) || !unblockMenu_->Focused()) {
                auto res = cardService_->unblockCard(unblockCardNum_);
                if (res.is_ok()) {
                    unblockStatus_ = "[Success] Card unblocked and failed attempts reset.";
                    unblockCardNum_.clear();
                } else {
                    unblockStatus_ = "[Error] " + Services::toErrorMessage(res.error());
                }
                return true;
            }
        }
        if (event == Event::Escape) {
            innerScreen_ = 0;
            return true;
        }
        return false;
    });

    auto unblockRenderer = Renderer(unblockHandler, [this]() {
        auto statusCol = (unblockStatus_.rfind("[Success]", 0) == 0) ? Color::Green :
                         (unblockStatus_.rfind("[Error]", 0) == 0) ? Color::Red : Color::Yellow;
        return vbox({
            text("=== UNBLOCK CARD ===") | bold | color(Color::Green) | center,
            separator(),
            text("Card Number:"),
            unblockCardInput_->Render() | border,
            separator(),
            unblockMenu_->Render() | border,
            separator(),
            text(unblockStatus_) | color(statusCol) | center,
            separator(),
            text("[Esc] Back") | dim | center,
        }) | border | size(WIDTH, GREATER_THAN, 60);
    });

    revokeCardInput_ = Input(&revokeCardNum_, "Card Number");
    revokeMenu_ = Menu(&revokeActions_, &revokeActionSel_);
    revokeConfirmMenu_ = Menu(&revokeConfirmActions_, &revokeActionSel_);
    auto revokeContainer = Container::Vertical({revokeCardInput_, revokeMenu_, revokeConfirmMenu_});

    auto revokeHandler = CatchEvent(revokeContainer, [this](Event event) {
        if (event == Event::Return) {
            if (!revokeConfirmPending_) {
                if (revokeMenu_->Focused() && revokeActionSel_ == 1) {
                    innerScreen_ = 0;
                    return true;
                }
                if ((revokeMenu_->Focused() && revokeActionSel_ == 0) || !revokeMenu_->Focused()) {
                    if (revokeCardNum_.empty()) {
                        revokeStatus_ = "[Error] Card number cannot be empty.";
                        return true;
                    }
                    auto cardOpt = cardService_->getCardByNumber(revokeCardNum_);
                    if (!cardOpt.is_ok()) {
                        revokeStatus_ = "[Error] " + Services::toErrorMessage(cardOpt.error());
                        return true;
                    }
                    revokeConfirmPending_ = true;
                    revokeActionSel_ = 0;
                    revokeStatus_ = "Confirm revoke for " + revokeCardNum_ + "?";
                    return true;
                }
            } else {
                if (revokeActionSel_ == 1) {
                    revokeConfirmPending_ = false;
                    revokeStatus_ = "Revoke cancelled.";
                    return true;
                }
                auto res = cardService_->revokeCard(revokeCardNum_);
                if (res.is_ok()) {
                    revokeStatus_ = "[Success] Card revoked.";
                    revokeCardNum_.clear();
                } else {
                    revokeStatus_ = "[Error] " + Services::toErrorMessage(res.error());
                }
                revokeConfirmPending_ = false;
                return true;
            }
        }
        if (event == Event::Escape) {
            if (revokeConfirmPending_) {
                revokeConfirmPending_ = false;
                revokeStatus_ = "Enter card number to revoke.";
                return true;
            }
            innerScreen_ = 0;
            return true;
        }
        return false;
    });

    auto revokeRenderer = Renderer(revokeHandler, [this]() {
        auto statusCol = (revokeStatus_.rfind("[Success]", 0) == 0) ? Color::Green :
                         (revokeStatus_.rfind("[Error]", 0) == 0) ? Color::Red : Color::Yellow;
        Elements children;
        children.push_back(text("=== REVOKE CARD ===") | bold | color(Color::Green) | center);
        children.push_back(separator());
        children.push_back(text("Card Number:"));
        children.push_back(revokeCardInput_->Render() | border);
        children.push_back(separator());
        if (revokeConfirmPending_) {
            children.push_back(text("Підтвердіть відкликання картки:") | bold | color(Color::Red));
            children.push_back(revokeConfirmMenu_->Render() | border);
        } else {
            children.push_back(revokeMenu_->Render() | border);
        }
        children.push_back(separator());
        children.push_back(text(revokeStatus_) | color(statusCol) | center);
        children.push_back(separator());
        children.push_back(text("[Esc] Back") | dim | center);
        return vbox(std::move(children)) | border | size(WIDTH, GREATER_THAN, 60);
    });

    changePinCardInput_ = Input(&changePinCardNum_, "Card Number");
    changePinInput_ = Input(&changePinNew_, "New PIN", pinOpt);
    changePinMenu_ = Menu(&changePinActions_, &changePinActionSel_);
    auto changePinContainer = Container::Vertical({changePinCardInput_, changePinInput_, changePinMenu_});

    auto changePinHandler = CatchEvent(changePinContainer, [this](Event event) {
        if (event == Event::Return) {
            if (changePinMenu_->Focused() && changePinActionSel_ == 1) {
                innerScreen_ = 0;
                return true;
            }
            if ((changePinMenu_->Focused() && changePinActionSel_ == 0) || !changePinMenu_->Focused()) {
                auto res = cardService_->changePinByAdmin(changePinCardNum_, changePinNew_);
                if (res.is_ok()) {
                    changePinStatus_ = "[Success] PIN changed.";
                    changePinCardNum_.clear();
                    changePinNew_.clear();
                } else {
                    changePinStatus_ = "[Error] " + Services::toErrorMessage(res.error());
                }
                return true;
            }
        }
        if (event == Event::Escape) {
            innerScreen_ = 0;
            return true;
        }
        return false;
    });

    auto changePinRenderer = Renderer(changePinHandler, [this]() {
        auto statusCol = (changePinStatus_.rfind("[Success]", 0) == 0) ? Color::Green :
                         (changePinStatus_.rfind("[Error]", 0) == 0) ? Color::Red : Color::Yellow;
        return vbox({
            text("=== CHANGE CARD PIN ===") | bold | color(Color::Green) | center,
            separator(),
            text("Card Number:"),
            changePinCardInput_->Render() | border,
            text("New PIN:"),
            changePinInput_->Render() | border,
            separator(),
            changePinMenu_->Render() | border,
            separator(),
            text(changePinStatus_) | color(statusCol) | center,
            separator(),
            text("[Esc] Back") | dim | center,
        }) | border | size(WIDTH, GREATER_THAN, 60);
    });

    tabContainer_ = Container::Tab({
        subMenuRenderer,
        listRenderer,
        addRenderer,
        blockRenderer,
        unblockRenderer,
        revokeRenderer,
        changePinRenderer,
    }, &innerScreen_);

    component_ = tabContainer_;
}

} // namespace UI
