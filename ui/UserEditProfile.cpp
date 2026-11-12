#include "UserEditProfile.h"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"

using namespace ftxui;

namespace UI {

void UserEditProfile::loadUser(User* user) {
    currentUser_ = user;
    if (currentUser_) {
        firstNameVal_ = std::string(currentUser_->getFirstName());
        lastNameVal_ = std::string(currentUser_->getLastName());
    } else {
        firstNameVal_.clear();
        lastNameVal_.clear();
    }
    statusMessage_.clear();
    selected_ = 0;
}

UserEditProfile::UserEditProfile(
    std::shared_ptr<UserRepo> userRepo,
    User* currentUser,
    std::function<void()> onBack
) : userRepo_(userRepo),
    currentUser_(currentUser),
    onBack_(onBack) {

    loadUser(currentUser_);

    firstNameInput_ = Input(&firstNameVal_, "First Name");
    lastNameInput_ = Input(&lastNameVal_, "Last Name");
    menu_ = Menu(&entries_, &selected_);

    container_ = Container::Vertical({
        firstNameInput_,
        lastNameInput_,
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
                    statusMessage_ = "[Error]: No active user session.";
                    return true;
                }

                if (firstNameVal_.empty() || lastNameVal_.empty()) {
                    statusMessage_ = "[Error]: First name and Last name cannot be empty.";
                    return true;
                }

                currentUser_->setFirstName(firstNameVal_);
                currentUser_->setLastName(lastNameVal_);

                try {
                    userRepo_->update(*currentUser_);
                    statusMessage_ = "[Success]: Profile updated successfully!";
                } catch (const std::exception& e) {
                    statusMessage_ = std::string("[Error]: ") + e.what();
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

        Elements elements;
        elements.push_back(text("=== USER PROFILE: EDIT NAME ===") | bold | color(Color::Cyan) | center);
        elements.push_back(separator());

        if (currentUser_) {
            elements.push_back(hbox({
                text("User ID: ") | dim,
                text(std::string(currentUser_->getId())) | dim,
            }));
            elements.push_back(separatorLight());
        }

        elements.push_back(text("First Name:"));
        elements.push_back(firstNameInput_->Render() | border);
        elements.push_back(text("Last Name:"));
        elements.push_back(lastNameInput_->Render() | border);
        elements.push_back(separator());
        elements.push_back(menu_->Render() | border);

        if (!statusMessage_.empty()) {
            elements.push_back(separator());
            elements.push_back(text(statusMessage_) | color(statusColor) | center);
        }

        elements.push_back(separator());
        elements.push_back(text("[Enter] Save/Select  |  [Esc] Back") | dim | center);

        return vbox(std::move(elements)) | border | size(WIDTH, GREATER_THAN, 60);
    });
}

} // namespace UI
