#include "AdminEditProfile.h"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include <fstream>

using namespace ftxui;

namespace UI {

void AdminEditProfile::loadUser() {
    if (currentUser_) {
        if (!currentUser_->getId().empty()) {
            auto fresh = userRepo_->getById(std::string(currentUser_->getId()));
            if (fresh.has_value()) {
                *currentUser_ = *fresh;
            }
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
    std::shared_ptr<UserRepo> userRepo,
    std::shared_ptr<CryptoService> cryptoService,
    User* currentUser,
    std::function<void()> onBack
) : userRepo_(userRepo),
    cryptoService_(cryptoService),
    currentUser_(currentUser),
    onBack_(onBack) {

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

                if (firstNameVal_.empty() || lastNameVal_.empty() || usernameVal_.empty()) {
                    statusMessage_ = "[Error]: First name, last name, and username cannot be empty.";
                    return true;
                }

                auto existing = userRepo_->getByUsername(usernameVal_);
                if (existing.has_value() && existing->getId() != currentUser_->getId()) {
                    statusMessage_ = "[Error]: Username is already taken by another user.";
                    return true;
                }

                currentUser_->setFirstName(firstNameVal_);
                currentUser_->setLastName(lastNameVal_);
                currentUser_->setUsername(usernameVal_);

                if (!passwordVal_.empty()) {
                    std::string newHash = cryptoService_->hashPin(passwordVal_);
                    currentUser_->setPasswordHash(newHash);
                }

                try {
                    userRepo_->update(*currentUser_);
                    passwordVal_.clear();
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

} // namespace UI
