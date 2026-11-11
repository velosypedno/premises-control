#include "UserView.h"

namespace UI {

UserView::UserView(std::shared_ptr<CardRepo> cardRepo,
                   std::shared_ptr<UserRepo> userRepo,
                   std::shared_ptr<AccessPolicyRepo> policyRepo,
                   std::shared_ptr<AccessLogRecordRepo> logRepo,
                   std::shared_ptr<CryptoService> cryptoService,
                   std::function<void()> onBack)
    : cardCheckService(cardRepo, userRepo, policyRepo, logRepo, cryptoService, 3), onBack(onBack)
{
    using namespace ftxui;

    InputOption passwordOption;
    passwordOption.password = true;

    auto cardIdBox = Input(&cardIdInput, "Card ID (16 digits)");
    auto cardPinBox = Input(&cardPinInput, "PIN Code", passwordOption);

    auto container = Container::Vertical({
        cardIdBox,
        cardPinBox,
    });

    auto eventHandler = CatchEvent(container, [this, onBack](Event event) {
        if (event == Event::Return) {
            bool ok = cardCheckService.authenticate(cardIdInput, cardPinInput, DayOfWeek::Monday, "10:00");
            statusMessage = ok ? "[Access]: Access granted!" : "[Access]: Access denied (or card blocked)!";
            return true;
        }
        if (event == Event::Escape) {
            if (onBack) {
                onBack();
            }
            return true;
        }
        return false;
    });

    component = Renderer(eventHandler, [this, cardIdBox, cardPinBox] {
        Elements children;
        children.push_back(text("=== PANEL: USER ACCESS CHECK ===") | bold | color(Color::Blue) | center);
        children.push_back(separator());
        children.push_back(text("Card ID:"));
        children.push_back(cardIdBox->Render() | border);
        children.push_back(text("PIN Code:"));
        children.push_back(cardPinBox->Render() | border);
        children.push_back(separator());
        children.push_back(text("[Enter] - Check  |  [Esc] - Back") | dim | center);
        children.push_back(separator());
        children.push_back(text(statusMessage) | color(Color::Yellow) | center);

        return vbox(children) | border | size(WIDTH, GREATER_THAN, 50);
    });
}

}
