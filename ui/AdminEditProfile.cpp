#include "AdminEditProfile.h"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"

using namespace ftxui;

namespace UI {

void AdminEditProfile::loadUser() {
    if (currentUser_ && !currentUser_->getId().empty()) {
        auto res = userService_->getUserById(std::string(currentUser_->getId()));
        if (res.is_ok()) {
            *currentUser_ = res.value();
        }
        firstNameVal_ = std::string(currentUser_->getFirstName());
        lastNameVal_ = std::string(currentUser_->getLastName());
        usernameVal_ = currentUser_->getUsernameOpt().value_or("");
    }
    passwordVal_.clear();
    statusMessage_.clear();
    selected_ = 0;
}

AdminEditProfile::AdminEditProfile(
    std::shared_ptr<UserService> userService,
    User* currentUser,
    std::function<void()> onBack
) : userService_(std::move(userService)),
    currentUser_(currentUser),
    onBack_(std::move(onBack)) {

    loadUser();

    InputOption passwordOption;
    passwordOption.password = true;

    firstNameInput_ = Input(&firstNameVal_, "First Name");
    lastNameInput_ = Input(&lastNameVal_, "Last Name");
    usernameInput_ = Input(&usernameVal_, "Username");
    passwordInput_ = Input(&passwordVal_, "New Password (optional)", passwordOption);

    menu_ = Menu(&entries_, &selected_);

    container_ = Container::Vertical({
        firstNameInput_,
        lastNameInput_,
        usernameInput_,
        passwordInput_,
        menu_,
    });

    auto handler = CatchEvent(container_, [this](Event event) {
        if (event == Event::Return) {
            if (menu_->Focused() && selected_ == 1) {
                if (onBack_) onBack_();
                return true;
            }

            if ((menu_->Focused() && selected_ == 0) || !menu_->Focused()) {
                if (!currentUser_) {
                    statusMessage_ = "[Error]: No user loaded.";
                    return true;
                }

                auto res = userService_->updateProfile(
                    std::string(currentUser_->getId()),
                    firstNameVal_,
                    lastNameVal_,
                    usernameVal_,
                    passwordVal_.empty() ? std::nullopt : std::make_optional(passwordVal_)
                );

                if (res.is_ok()) {
                    *currentUser_ = res.value();
                    passwordVal_.clear();
                    statusMessage_ = "[Success]: Profile updated successfully!";
                } else {
                    statusMessage_ = "[Error]: " + Services::toErrorMessage(res.error());
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

        Elements rows;
        rows.push_back(text("=== EDIT PROFILE ===") | bold | color(Color::Green) | center);
        rows.push_back(separator());
        rows.push_back(text("First Name:"));
        rows.push_back(firstNameInput_->Render() | border);
        rows.push_back(text("Last Name:"));
        rows.push_back(lastNameInput_->Render() | border);
        rows.push_back(text("Username:"));
        rows.push_back(usernameInput_->Render() | border);
        rows.push_back(text("New Password (leave empty to keep current):"));
        rows.push_back(passwordInput_->Render() | border);
        rows.push_back(separator());
        rows.push_back(menu_->Render() | border);

        if (!statusMessage_.empty()) {
            rows.push_back(separator());
            rows.push_back(text(statusMessage_) | color(statusColor) | center);
        }

        rows.push_back(separator());
        rows.push_back(text("[Enter] Submit/Select  |  [Esc] Back") | dim | center);

        return vbox(std::move(rows)) | border | size(WIDTH, GREATER_THAN, 60);
    });
}

}
