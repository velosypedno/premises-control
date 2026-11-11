#pragma once

#include <string>
#include <vector>
#include <functional>
#include "ftxui/component/component.hpp"

namespace UI {
    class AdminDashboard {
    private:
        int selectedMenu = 0;
        std::vector<std::string> menuEntries = {
            "1. View System Access Logs",
            "2. Manage Users & Cards",
            "3. Logout (Return to Main Menu)"
        };
        std::string statusMessage = "Welcome to the Administrator Dashboard.";
        std::function<void()> onLogout;

        ftxui::Component component;

    public:
        AdminDashboard(std::function<void()> onLogout);

        ftxui::Component getComponent() { return component; }

        void resetStatus() {
            statusMessage = "Welcome to the Administrator Dashboard.";
            selectedMenu = 0;
        }
    };
}
