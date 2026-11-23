#include "ConsoleUI.h"
#include "AdminView.h"
#include "UserView.h"
#include "AdminDashboard.h"
#include "UserDashboard.h"

#include <iostream>
#include <vector>
#include <string>
#include <memory>


#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"

namespace UI {

void runConsoleUI(const AppDependencies& deps) {
    using namespace ftxui;

    int currentScreen = 0;

    AdminDashboard adminDashboard(
        [&]() { currentScreen = 0; },
        deps.userService,
        deps.cardService,
        deps.policyRepo,
        deps.userRepo,
        deps.logRepo
    );

    UserDashboard userDashboard(
        [&]() { currentScreen = 0; },
        deps.userService,
        deps.cardService,
        deps.cardRepo,
        deps.policyRepo
    );

    AdminView adminView(deps.adminCheck,
        [&]() { currentScreen = 0; },
        [&](const User& user) {
            adminDashboard.setCurrentUser(user);
            adminDashboard.resetStatus();
            currentScreen = 3;
        }
    );

    UserView userView(deps.cardCheck, deps.cardRepo, deps.userRepo,
        [&]() { currentScreen = 0; },
        [&](const Card& card, const User& user) {
            userDashboard.setCurrentSession(card, user);
            userDashboard.resetStatus();
            currentScreen = 4;
        }
    );

    int mainSelected = 0;
    std::vector<std::string> mainEntries = {
        "1. Access Attempt",
        "2. Administrator Mode",
        "0. Exit"
    };
    auto mainMenu = Menu(&mainEntries, &mainSelected);
    auto screen = ScreenInteractive::TerminalOutput();

    std::string mainStatus = "Select an action (Arrow keys + Enter).";

    auto mainComponent = CatchEvent(mainMenu, [&](Event event) {
        if (event == Event::Return) {
            if (mainSelected == 0) {
                currentScreen = 2;
                userView.resetStatus();
            } else if (mainSelected == 1) {
                currentScreen = 1;
                adminView.resetStatus();
            } else if (mainSelected == 2) {
                screen.Exit();
            }
            return true;
        }
        return false;
    });

    auto mainRenderer = Renderer(mainComponent, [&] {
        Elements children;
        children.push_back(text("ACCESS CONTROL TERMINAL") | bold | color(Color::Cyan) | center);
        children.push_back(separator());
        children.push_back(mainMenu->Render() | border | color(Color::Green));
        children.push_back(separator());
        children.push_back(text(mainStatus) | color(Color::Yellow) | center);
        return vbox(children) | border;
    });

    Components screens = {
        mainRenderer,
        adminView.getComponent(),
        userView.getComponent(),
        adminDashboard.getComponent(),
        userDashboard.getComponent()
    };

    auto container = Container::Tab(screens, &currentScreen);

    std::cout << "\nStarting Interactive Console UI...\n";
    screen.Loop(container);
}

}
