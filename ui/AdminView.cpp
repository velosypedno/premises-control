#include "AdminView.h"

namespace UI {

AdminView::AdminView(std::shared_ptr<UserRepo> userRepo,
                     std::shared_ptr<CryptoService> cryptoService,
                     const std::string& rootPass,
                     std::function<void()> onBack,
                     std::function<void()> onSuccess)
    : adminCheck(userRepo, cryptoService), rootPass(rootPass), onBack(onBack), onSuccess(onSuccess)
{
    using namespace ftxui;

    InputOption passwordOption;
    passwordOption.password = true;

    auto userBox = Input(&username, "Username");
    auto passBox = Input(&password, "Password", passwordOption);

    auto container = Container::Vertical({
        userBox,
        passBox,
    });

    auto eventHandler = CatchEvent(container, [this, onBack, onSuccess](Event event) {
        if (event == Event::Return) {
            bool ok = adminCheck.authenticate(username, password);
            if (ok) {
                statusMessage = "[Success]: Administrator authenticated!";
                if (onSuccess) {
                    onSuccess();
                }
            } else {
                statusMessage = "[Error]: Invalid username or password!";
            }
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

    component = Renderer(eventHandler, [this, userBox, passBox] {
        Elements children;
        children.push_back(text("=== PANEL: ADMIN LOGIN ===") | bold | color(Color::Red) | center);
        children.push_back(separator());
        children.push_back(text("Username:"));
        children.push_back(userBox->Render() | border);
        children.push_back(text("Password:"));
        children.push_back(passBox->Render() | border);
        children.push_back(separator());
        children.push_back(text("[Enter] - Login  |  [Esc] - Back") | dim | center);
        children.push_back(separator());
        children.push_back(text(statusMessage) | color(Color::Yellow) | center);

        return vbox(children) | border | size(WIDTH, GREATER_THAN, 50);
    });
}

} // namespace UI
