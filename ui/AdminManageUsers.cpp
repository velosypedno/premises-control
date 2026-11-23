#include "AdminManageUsers.h"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include <string>
#include <vector>

using namespace ftxui;

namespace UI {

void AdminManageUsers::reset() {
    innerScreen_ = 0;
    subSel_ = 0;
    createFirstName_.clear();
    createLastName_.clear();
    createUsername_.clear();
    createPassword_.clear();
    createStatus_ = "Fill in details to create a new user.";
    assignRoleUserId_.clear();
    assignRoleStatus_ = "Enter the User ID to promote to Admin.";
    assignPolicyUserId_.clear();
    assignPolicyId_.clear();
    assignPolicyStatus_ = "Enter User ID and Policy ID to assign.";
    toggleActiveUserId_.clear();
    toggleActiveStatus_ = "Enter User ID to toggle active flag.";
}

AdminManageUsers::AdminManageUsers(
    std::shared_ptr<UserService> userService,
    std::function<void()> onBack
) : userService_(std::move(userService)),
    onBack_(std::move(onBack)) {

    subMenu_ = Menu(&subEntries_, &subSel_);

    auto subMenuHandler = CatchEvent(subMenu_, [this](Event event) {
        if (event == Event::Return) {
            int sel = subSel_;
            if (sel == 5) {
                if (onBack_) onBack_();
            } else {
                innerScreen_ = sel + 1;
                status_.clear();
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
            text("=== MANAGE USERS ===") | bold | color(Color::Green) | center,
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
        rows.push_back(text("=== LIST ALL USERS ===") | bold | color(Color::Green) | center);
        rows.push_back(separator());
        rows.push_back(text("ID | Name | Role | Active | Username | PolicyID") | bold);
        rows.push_back(separator());
        try {
            auto users = userService_->listAllUsers();
            if (users.empty()) {
                rows.push_back(text("No users found.") | dim | center);
            } else {
                for (auto& u : users) {
                    std::string line =
                        std::string(u.getId()) + " | " +
                        u.getFullName() + " | " +
                        User::roleToString(u.getRole()) + " | " +
                        (u.isActive() ? "YES" : "NO") + " | " +
                        (u.getUsernameOpt().has_value() ? std::string(*u.getUsernameOpt()) : "(none)") + " | " +
                        (u.getPolicyIdOpt().has_value() ? std::string(*u.getPolicyIdOpt()) : "(none)");
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

    InputOption passOpt;
    passOpt.password = true;

    createFNInput_ = Input(&createFirstName_, "First Name");
    createLNInput_ = Input(&createLastName_, "Last Name");
    createUNInput_ = Input(&createUsername_, "Username");
    createPWInput_ = Input(&createPassword_, "Password", passOpt);
    createActionMenu_ = Menu(&createActions_, &createActionSel_);

    auto createContainer = Container::Vertical({
        createFNInput_,
        createLNInput_,
        createUNInput_,
        createPWInput_,
        createActionMenu_,
    });

    auto createHandler = CatchEvent(createContainer, [this](Event event) {
        if (event == Event::Return) {
            if (createActionMenu_->Focused() && createActionSel_ == 1) {
                innerScreen_ = 0;
                return true;
            }
            if ((createActionMenu_->Focused() && createActionSel_ == 0) || !createActionMenu_->Focused()) {
                auto res = userService_->createUser(createFirstName_, createLastName_, createUsername_, createPassword_);
                if (res.is_ok()) {
                    createStatus_ = "[Success] User created.";
                    createFirstName_.clear();
                    createLastName_.clear();
                    createUsername_.clear();
                    createPassword_.clear();
                } else {
                    createStatus_ = "[Error] " + Services::toErrorMessage(res.error());
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

    auto createRenderer = Renderer(createHandler, [this]() {
        auto statusCol = (createStatus_.rfind("[Success]", 0) == 0) ? Color::Green :
                         (createStatus_.rfind("[Error]", 0) == 0) ? Color::Red : Color::Yellow;
        return vbox({
            text("=== CREATE NEW USER ===") | bold | color(Color::Green) | center,
            separator(),
            text("First Name:"),
            createFNInput_->Render() | border,
            text("Last Name:"),
            createLNInput_->Render() | border,
            text("Username:"),
            createUNInput_->Render() | border,
            text("Password:"),
            createPWInput_->Render() | border,
            separator(),
            createActionMenu_->Render() | border,
            separator(),
            text(createStatus_) | color(statusCol) | center,
            separator(),
            text("[Esc] Back") | dim | center,
        }) | border | size(WIDTH, GREATER_THAN, 60);
    });

    assignRoleInput_ = Input(&assignRoleUserId_, "User ID");
    assignRoleMenu_ = Menu(&assignRoleActions_, &assignRoleActionSel_);
    auto assignRoleContainer = Container::Vertical({assignRoleInput_, assignRoleMenu_});

    auto assignRoleHandler = CatchEvent(assignRoleContainer, [this](Event event) {
        if (event == Event::Return) {
            if (assignRoleMenu_->Focused() && assignRoleActionSel_ == 1) {
                innerScreen_ = 0;
                return true;
            }
            if ((assignRoleMenu_->Focused() && assignRoleActionSel_ == 0) || !assignRoleMenu_->Focused()) {
                auto res = userService_->assignAdminRole(assignRoleUserId_);
                if (res.is_ok()) {
                    assignRoleStatus_ = "[Success] User promoted to Admin.";
                    assignRoleUserId_.clear();
                } else {
                    assignRoleStatus_ = "[Error] " + Services::toErrorMessage(res.error());
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

    auto assignRoleRenderer = Renderer(assignRoleHandler, [this]() {
        auto statusCol = (assignRoleStatus_.rfind("[Success]", 0) == 0) ? Color::Green :
                         (assignRoleStatus_.rfind("[Error]", 0) == 0) ? Color::Red : Color::Yellow;
        return vbox({
            text("=== ASSIGN ADMIN ROLE ===") | bold | color(Color::Green) | center,
            separator(),
            text("User ID:"),
            assignRoleInput_->Render() | border,
            separator(),
            assignRoleMenu_->Render() | border,
            separator(),
            text(assignRoleStatus_) | color(statusCol) | center,
            separator(),
            text("[Esc] Back") | dim | center,
        }) | border | size(WIDTH, GREATER_THAN, 60);
    });

    assignPolicyUserInput_ = Input(&assignPolicyUserId_, "User ID");
    assignPolicyInput_ = Input(&assignPolicyId_, "Policy ID");
    assignPolicyMenu_ = Menu(&assignPolicyActions_, &assignPolicyActionSel_);
    auto assignPolicyContainer = Container::Vertical({assignPolicyUserInput_, assignPolicyInput_, assignPolicyMenu_});

    auto assignPolicyHandler = CatchEvent(assignPolicyContainer, [this](Event event) {
        if (event == Event::Return) {
            if (assignPolicyMenu_->Focused() && assignPolicyActionSel_ == 1) {
                innerScreen_ = 0;
                return true;
            }
            if ((assignPolicyMenu_->Focused() && assignPolicyActionSel_ == 0) || !assignPolicyMenu_->Focused()) {
                auto res = userService_->assignPolicy(assignPolicyUserId_, assignPolicyId_);
                if (res.is_ok()) {
                    assignPolicyStatus_ = "[Success] Policy assigned to user.";
                    assignPolicyUserId_.clear();
                    assignPolicyId_.clear();
                } else {
                    assignPolicyStatus_ = "[Error] " + Services::toErrorMessage(res.error());
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

    auto assignPolicyRenderer = Renderer(assignPolicyHandler, [this]() {
        auto statusCol = (assignPolicyStatus_.rfind("[Success]", 0) == 0) ? Color::Green :
                         (assignPolicyStatus_.rfind("[Error]", 0) == 0) ? Color::Red : Color::Yellow;
        return vbox({
            text("=== ASSIGN POLICY TO USER ===") | bold | color(Color::Green) | center,
            separator(),
            text("User ID:"),
            assignPolicyUserInput_->Render() | border,
            text("Policy ID:"),
            assignPolicyInput_->Render() | border,
            separator(),
            assignPolicyMenu_->Render() | border,
            separator(),
            text(assignPolicyStatus_) | color(statusCol) | center,
            separator(),
            text("[Esc] Back") | dim | center,
        }) | border | size(WIDTH, GREATER_THAN, 60);
    });

    toggleActiveInput_ = Input(&toggleActiveUserId_, "User ID");
    toggleActiveMenu_ = Menu(&toggleActiveActions_, &toggleActiveActionSel_);
    auto toggleActiveContainer = Container::Vertical({toggleActiveInput_, toggleActiveMenu_});

    auto toggleActiveHandler = CatchEvent(toggleActiveContainer, [this](Event event) {
        if (event == Event::Return) {
            if (toggleActiveMenu_->Focused() && toggleActiveActionSel_ == 1) {
                innerScreen_ = 0;
                return true;
            }
            if ((toggleActiveMenu_->Focused() && toggleActiveActionSel_ == 0) || !toggleActiveMenu_->Focused()) {
                auto res = userService_->toggleUserActive(toggleActiveUserId_);
                if (res.is_ok()) {
                    toggleActiveStatus_ = std::string("[Success] User is now ") +
                                          (res.value() ? "ACTIVE." : "INACTIVE.");
                    toggleActiveUserId_.clear();
                } else {
                    toggleActiveStatus_ = "[Error] " + Services::toErrorMessage(res.error());
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

    auto toggleActiveRenderer = Renderer(toggleActiveHandler, [this]() {
        auto statusCol = (toggleActiveStatus_.rfind("[Success]", 0) == 0) ? Color::Green :
                         (toggleActiveStatus_.rfind("[Error]", 0) == 0) ? Color::Red : Color::Yellow;
        return vbox({
            text("=== TOGGLE USER ACTIVE ===") | bold | color(Color::Green) | center,
            separator(),
            text("User ID:"),
            toggleActiveInput_->Render() | border,
            separator(),
            toggleActiveMenu_->Render() | border,
            separator(),
            text(toggleActiveStatus_) | color(statusCol) | center,
            separator(),
            text("[Esc] Back") | dim | center,
        }) | border | size(WIDTH, GREATER_THAN, 60);
    });

    tabContainer_ = Container::Tab({
        subMenuRenderer,
        listRenderer,
        createRenderer,
        assignRoleRenderer,
        assignPolicyRenderer,
        toggleActiveRenderer,
    }, &innerScreen_);

    component_ = tabContainer_;
}

} // namespace UI
