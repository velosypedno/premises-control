#include "ConsoleUI.h"
#include "AdminView.h"
#include "UserView.h"

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

void runConsoleUI() {
    Config config = Config::loadFromEnv();
    auto db = Database::openConnection(config.dbPath);
    auto cryptoService = std::make_shared<CryptoService>(config.salt);

    auto userRepo = std::make_shared<UserRepo>(db);
    auto cardRepo = std::make_shared<CardRepo>(db);
    auto policyRepo = std::make_shared<AccessPolicyRepo>(db);
    auto logRepo = std::make_shared<AccessLogRecordRepo>(db);

    using namespace ftxui;

    int currentScreen = 0;

    AdminView adminView(userRepo, cryptoService, config.rootPass, [&]() {
        currentScreen = 0;
    });
    UserView userView(cardRepo, userRepo, policyRepo, logRepo, cryptoService, [&]() {
        currentScreen = 0;
    });

    int mainSelected = 0;
    std::vector<std::string> mainEntries = {
        "1. Admin Login (AdminCheck)",
        "2. User Access Check (CardCheck)",
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
    };

    auto container = Container::Tab(screens, &currentScreen);

    std::cout << "\nStarting Interactive Console UI...\n";
    screen.Loop(container);
}

}
