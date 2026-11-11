#include "AdminDashboard.h"

namespace UI {

AdminDashboard::AdminDashboard(std::function<void()> onLogout)
    : onLogout(onLogout)
{
    using namespace ftxui;

    auto menu = Menu(&menuEntries, &selectedMenu);

    auto eventHandler = CatchEvent(menu, [this, onLogout](Event event) {
        if (event == Event::Return) {
            if (selectedMenu == 0) {
                statusMessage = "[Action]: Logs view is not implemented yet.";
            } else if (selectedMenu == 1) {
                statusMessage = "[Action]: User management is not implemented yet.";
            } else if (selectedMenu == 2) {
                if (onLogout) {
                    onLogout();
                }
            }
            return true;
        }
        if (event == Event::Escape) {
            if (onLogout) {
                onLogout();
            }
            return true;
        }
        return false;
    });

    component = Renderer(eventHandler, [this, menu] {
        Elements children;
        children.push_back(text("=== ADMIN CONTROL PANEL ===") | bold | color(Color::Green) | center);
        children.push_back(separator());
        children.push_back(menu->Render() | border);
        children.push_back(separator());
        children.push_back(text("[Enter] - Select  |  [Esc] - Logout") | dim | center);
        children.push_back(separator());
        children.push_back(text(statusMessage) | color(Color::Yellow) | center);

        return vbox(children) | border | size(WIDTH, GREATER_THAN, 60);
    });
}

}
