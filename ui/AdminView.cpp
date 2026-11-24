#include "AdminView.h"

namespace UI {

AdminView::AdminView(std::shared_ptr<AdminCheck> adminCheck,
                     std::function<void()> onBack,
                     std::function<void(const User&)> onSuccess)
    : adminCheck_(std::move(adminCheck)),
      onBack_(std::move(onBack)),
      onSuccess_(std::move(onSuccess))
{
    using namespace ftxui;

    InputOption passwordOption;
    passwordOption.password = true;

    usernameBox_ = Input(&usernameInput_, "Username");
    passwordBox_ = Input(&passwordInput_, "Password", passwordOption);

    auto attemptLogin = [this]() {
        if (usernameInput_.empty() || passwordInput_.empty()) {
            statusMessage_ = "[Error]: Enter both username and password.";
            return;
        }

        auto result = adminCheck_->authenticate(usernameInput_, passwordInput_);
        if (result.is_ok()) {
            statusMessage_ = "[Success]: Administrator authenticated!";
            if (onSuccess_) {
                onSuccess_(result.value());
            }
        } else {
            statusMessage_ = "[Error]: " + toErrorMessage(result.error());
        }
    };

    auto loginButton = Button("Log In", attemptLogin);

    auto container = Container::Vertical({
        usernameBox_,
        passwordBox_,
        loginButton,
    });

    auto eventHandler = CatchEvent(container, [this, attemptLogin](Event event) {
        if (event == Event::Escape) {
            if (onBack_) {
                onBack_();
            }
            return true;
        }
        if (event == Event::Return) {
            if (usernameBox_->Focused() || passwordBox_->Focused()) {
                attemptLogin();
                return true;
            }
        }
        return false;
    });

    component_ = Renderer(eventHandler, [this, loginButton] {
        auto statusCol = (statusMessage_.rfind("[Success]", 0) == 0) ? Color::Green :
                         (statusMessage_.rfind("[Error]", 0) == 0) ? Color::Red : Color::Yellow;

        Elements children;
        children.push_back(text("=== ADMINISTRATOR MODE ===") | bold | color(Color::Red) | center);
        children.push_back(separator());
        children.push_back(text("Username:"));
        children.push_back(usernameBox_->Render() | border);
        children.push_back(text("Password:"));
        children.push_back(passwordBox_->Render() | border);
        children.push_back(separatorLight());
        children.push_back(loginButton->Render() | center);
        children.push_back(separator());
        children.push_back(text("[Enter] - Login  |  [Esc] - Back") | dim | center);
        children.push_back(separator());
        children.push_back(text(statusMessage_) | color(statusCol) | center);

        return vbox(std::move(children)) | border | size(WIDTH, GREATER_THAN, 50);
    });
}

}
