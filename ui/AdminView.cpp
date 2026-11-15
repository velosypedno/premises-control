#include "AdminView.h"

namespace UI {

AdminView::AdminView(std::shared_ptr<UserRepo> userRepo,
                     std::shared_ptr<CardRepo> cardRepo,
                     std::shared_ptr<CryptoService> cryptoService,
                     std::function<void()> onBack,
                     std::function<void(const User&)> onSuccess)
    : adminCheck(userRepo, cardRepo, cryptoService), onBack(onBack), onSuccess(onSuccess)
{
    using namespace ftxui;

    InputOption passwordOption;
    passwordOption.password = true;

    auto cardBox = Input(&cardIdInput, "Card ID (16 digits)");
    auto pinBox = Input(&pinInput, "PIN", passwordOption);

    auto container = Container::Vertical({
        cardBox,
        pinBox,
    });

    auto eventHandler = CatchEvent(container, [this](Event event) {
        if (event == Event::Return) {
            if (cardIdInput.empty() || pinInput.empty()) {
                statusMessage = "[Error]: Enter both card number and PIN.";
                return true;
            }

            auto userOpt = adminCheck.authenticateByCard(cardIdInput, pinInput);
            if (userOpt.has_value()) {
                authenticatedUser_ = userOpt;
                statusMessage = "[Success]: Administrator authenticated!";
                if (this->onSuccess) {
                    this->onSuccess(*userOpt);
                }
            } else {
                statusMessage = "[Error]: Admin access denied (need ADMIN role card + PIN).";
            }
            return true;
        }
        if (event == Event::Escape) {
            if (this->onBack) {
                this->onBack();
            }
            return true;
        }
        return false;
    });

    component = Renderer(eventHandler, [this, cardBox, pinBox] {
        Elements children;
        children.push_back(text("=== РЕЖИМ АДМІНІСТРАТОРА ===") | bold | color(Color::Red) | center);
        children.push_back(separator());
        children.push_back(text("Card ID:"));
        children.push_back(cardBox->Render() | border);
        children.push_back(text("PIN:"));
        children.push_back(pinBox->Render() | border);
        children.push_back(separator());
        children.push_back(text("[Enter] - Login  |  [Esc] - Back") | dim | center);
        children.push_back(separator());
        children.push_back(text(statusMessage) | color(Color::Yellow) | center);

        return vbox(children) | border | size(WIDTH, GREATER_THAN, 50);
    });
}

} // namespace UI
