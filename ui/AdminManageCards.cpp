#include "AdminManageCards.h"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include <string>
#include <vector>

using namespace ftxui;

namespace UI {

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
}

AdminManageCards::AdminManageCards(
    std::shared_ptr<CardRepo> cardRepo,
    std::shared_ptr<UserRepo> userRepo,
    std::shared_ptr<CryptoService> cryptoService,
    std::function<void()> onBack
) : cardRepo_(cardRepo),
    userRepo_(userRepo),
    cryptoService_(cryptoService),
    onBack_(onBack) {

    subMenu_ = Menu(&subEntries_, &subSel_);

    auto subMenuHandler = CatchEvent(subMenu_, [this](Event event) {
        if (event == Event::Return) {
            int sel = subSel_;
            if (sel == 4) {
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
            auto cards = cardRepo_->list();
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
                if (addUserId_.empty() || addCardNum_.empty() || addPin_.empty()) {
                    addStatus_ = "[Error] All fields are required.";
                    return true;
                }
                try {
                    std::string hash = cryptoService_->hashPin(addPin_);
                    Card card(addUserId_, addCardNum_, hash);
                    cardRepo_->create(card);
                    addStatus_ = "[Success] Card added.";
                    addUserId_.clear();
                    addCardNum_.clear();
                    addPin_.clear();
                } catch (const std::exception& e) {
                    addStatus_ = std::string("[Error] ") + e.what();
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
                if (blockCardNum_.empty()) {
                    blockStatus_ = "[Error] Card number cannot be empty.";
                    return true;
                }
                try {
                    auto cardOpt = cardRepo_->getByCardId(blockCardNum_);
                    if (!cardOpt.has_value()) {
                        blockStatus_ = "[Error] Card not found.";
                        return true;
                    }
                    Card c = *cardOpt;
                    c.setStatus(CardStatus::Blocked);
                    cardRepo_->update(c);
                    blockStatus_ = "[Success] Card blocked.";
                    blockCardNum_.clear();
                } catch (const std::exception& e) {
                    blockStatus_ = std::string("[Error] ") + e.what();
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
                if (unblockCardNum_.empty()) {
                    unblockStatus_ = "[Error] Card number cannot be empty.";
                    return true;
                }
                try {
                    auto cardOpt = cardRepo_->getByCardId(unblockCardNum_);
                    if (!cardOpt.has_value()) {
                        unblockStatus_ = "[Error] Card not found.";
                        return true;
                    }
                    Card c = *cardOpt;
                    c.setStatus(CardStatus::Active);
                    c.resetFailedAttempts();
                    cardRepo_->update(c);
                    unblockStatus_ = "[Success] Card unblocked and failed attempts reset.";
                    unblockCardNum_.clear();
                } catch (const std::exception& e) {
                    unblockStatus_ = std::string("[Error] ") + e.what();
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

    tabContainer_ = Container::Tab({
        subMenuRenderer,
        listRenderer,
        addRenderer,
        blockRenderer,
        unblockRenderer,
    }, &innerScreen_);

    component_ = tabContainer_;
}

} // namespace UI
