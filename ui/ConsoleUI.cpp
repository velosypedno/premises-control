#include "ConsoleUI.h"
#include "AdminView.h"
#include "UserView.h"
#include "AdminDashboard.h"
#include "UserDashboard.h"

#include <iostream>
#include <vector>
#include <string>
#include <memory>

#include "../infra/Database.h"
#include "../infra/Config.h"
#include "../services/CryptoService.h"

#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"

namespace UI {

void runConsoleUI(
    std::shared_ptr<UserRepo> userRepo,
    std::shared_ptr<CardRepo> cardRepo,
    std::shared_ptr<AccessPolicyRepo> policyRepo,
    std::shared_ptr<AccessLogRecordRepo> logRepo,
    std::shared_ptr<CryptoService> cryptoService
) {
    using namespace ftxui;

    int currentScreen = 0;

    AdminDashboard adminDashboard(
        [&]() { currentScreen = 0; },
        userRepo,
        cardRepo,
        policyRepo,
        logRepo,
        cryptoService
    );

    UserDashboard userDashboard(
        [&]() { currentScreen = 0; },
        userRepo,
        cardRepo,
        policyRepo,
        cryptoService
    );

    AdminView adminView(userRepo, cryptoService,
        [&]() { currentScreen = 0; },
        [&]() {
            auto userOpt = userRepo->getByUsername(adminView.getUsername());
            if (userOpt) {
                adminDashboard.setCurrentUser(*userOpt);
            }
            adminDashboard.resetStatus();
            currentScreen = 3;
        }
    );

    UserView userView(cardRepo, userRepo, policyRepo, logRepo, cryptoService,
        [&]() { currentScreen = 0; },
        [&](const Card& card, const User& user) {
            userDashboard.setCurrentSession(card, user);
            userDashboard.resetStatus();
            currentScreen = 4;
        }
    );

    int mainSelected = 0;
    std::vector<std::string> mainEntries = {
        "1. Admin Login (AdminCheck)",
        "2. User Login / Card Check (UserDashboard)",
        "3. Exit"
    };
    auto mainMenu = Menu(&mainEntries, &mainSelected);
    auto screen = ScreenInteractive::TerminalOutput();

    std::string mainStatus = "Use Arrow keys to navigate and Enter to select.";

    auto mainComponent = CatchEvent(mainMenu, [&](Event event) {
        if (event == Event::Return) {
            if (mainSelected == 0) {
                currentScreen = 1;
                adminView.resetStatus();
            } else if (mainSelected == 1) {
                currentScreen = 2;
                userView.resetStatus();
            } else if (mainSelected == 2) {
                screen.Exit();
            }
            return true;
        }
        return false;
    });

    auto mainRenderer = Renderer(mainComponent, [&] {
        Elements children;
        children.push_back(text("=== PREMISES CONTROL SYSTEM ===") | bold | color(Color::Cyan) | center);
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

} // namespace UI
