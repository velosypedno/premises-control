#include "UserCardsView.h"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"

using namespace ftxui;

namespace UI {

void UserCardsView::loadSession(User* user, Card* card) {
    currentUser_ = user;
    currentCard_ = card;
    revokeCardIdInput_.clear();
    statusMessage_.clear();
    selected_ = 0;

    if (currentUser_) {
        try {
            cachedCards_ = cardRepo_->listByUserId(std::string(currentUser_->getId()));
        } catch (const std::exception& e) {
            statusMessage_ = std::string("[Error]: ") + e.what();
        }
    } else {
        cachedCards_.clear();
    }
}

UserCardsView::UserCardsView(
    std::shared_ptr<CardRepo> cardRepo,
    User* currentUser,
    Card* currentCard,
    std::function<void()> onBack
) : cardRepo_(cardRepo),
    currentUser_(currentUser),
    currentCard_(currentCard),
    onBack_(onBack) {

    loadSession(currentUser_, currentCard_);

    revokeInput_ = Input(&revokeCardIdInput_, "Card ID to Revoke");
    menu_ = Menu(&entries_, &selected_);

    container_ = Container::Vertical({
        revokeInput_,
        menu_,
    });

    auto handler = CatchEvent(container_, [this](Event event) {
        if (event == Event::Return) {
            if (menu_->Focused()) {
                if (selected_ == 2) { // Back
                    if (onBack_) onBack_();
                    return true;
                }
                if (selected_ == 1) { // Refresh
                    if (currentUser_) {
                        try {
                            cachedCards_ = cardRepo_->listByUserId(std::string(currentUser_->getId()));
                            statusMessage_ = "[Info]: Card list refreshed.";
                        } catch (const std::exception& e) {
                            statusMessage_ = std::string("[Error]: ") + e.what();
                        }
                    }
                    return true;
                }
                if (selected_ == 0) { // Revoke Card
                    if (revokeCardIdInput_.empty()) {
                        statusMessage_ = "[Error]: Please enter a Card ID to revoke.";
                        return true;
                    }

                    if (!currentUser_) {
                        statusMessage_ = "[Error]: No active user session.";
                        return true;
                    }

                    auto cardOpt = cardRepo_->getByCardId(revokeCardIdInput_);
                    if (!cardOpt.has_value() || cardOpt->getUserId() != currentUser_->getId()) {
                        statusMessage_ = "[Error]: Card not found in your account.";
                        return true;
                    }

                    Card c = *cardOpt;
                    if (c.getStatus() == CardStatus::Revoked) {
                        statusMessage_ = "[Error]: This card is already revoked.";
                        return true;
                    }

                    try {
                        c.setStatus(CardStatus::Revoked);
                        cardRepo_->update(c);

                        if (currentCard_ && currentCard_->getCardId() == c.getCardId()) {
                            currentCard_->setStatus(CardStatus::Revoked);
                        }

                        cachedCards_ = cardRepo_->listByUserId(std::string(currentUser_->getId()));
                        statusMessage_ = "[Success]: Card " + revokeCardIdInput_ + " has been revoked.";
                        revokeCardIdInput_.clear();
                    } catch (const std::exception& e) {
                        statusMessage_ = std::string("[Error]: ") + e.what();
                    }
                    return true;
                }
            }
            return false;
        }
        if (event == Event::Escape) {
            if (onBack_) onBack_();
            return true;
        }
        return false;
    });

    component_ = Renderer(handler, [this]() {
        auto statusColor = (statusMessage_.rfind("[Success]", 0) == 0) ? Color::Green :
                          (statusMessage_.rfind("[Info]", 0) == 0) ? Color::Cyan : Color::Red;

        Elements elements;
        elements.push_back(text("=== MY ACCESS CARDS ===") | bold | color(Color::Cyan) | center);
        elements.push_back(separator());

        if (cachedCards_.empty()) {
            elements.push_back(text("No cards associated with your user account.") | dim | center);
        } else {
            for (const auto& c : cachedCards_) {
                Color statusBadgeColor = Color::Green;
                std::string statusText = "ACTIVE";

                if (c.getStatus() == CardStatus::Blocked) {
                    statusBadgeColor = Color::Yellow;
                    statusText = "BLOCKED (" + std::to_string(c.getFailedAttempts()) + "/3 attempts)";
                } else if (c.getStatus() == CardStatus::Revoked) {
                    statusBadgeColor = Color::Red;
                    statusText = "REVOKED";
                }

                bool isCurrent = (currentCard_ && currentCard_->getCardId() == c.getCardId());

                Elements cardRow;
                cardRow.push_back(text("Card: ") | bold);
                cardRow.push_back(text(std::string(c.getCardId())) | bold);
                if (isCurrent) {
                    cardRow.push_back(text("  [Current Session]") | color(Color::Cyan) | bold);
                }
                cardRow.push_back(filler());
                cardRow.push_back(text("Status: ") | dim);
                cardRow.push_back(text("[" + statusText + "]") | color(statusBadgeColor) | bold);

                elements.push_back(hbox(std::move(cardRow)) | border);
            }
        }

        elements.push_back(separator());
        elements.push_back(text("Revoke Card (Enter Card ID to deactivate permanently):") | bold);
        elements.push_back(revokeInput_->Render() | border);

        elements.push_back(separator());
        elements.push_back(menu_->Render() | border);

        if (!statusMessage_.empty()) {
            elements.push_back(separator());
            elements.push_back(text(statusMessage_) | color(statusColor) | center);
        }

        elements.push_back(separator());
        elements.push_back(text("[Enter] Select  |  [Esc] Back") | dim | center);

        return vbox(std::move(elements)) | border | size(WIDTH, GREATER_THAN, 65);
    });
}

} // namespace UI
