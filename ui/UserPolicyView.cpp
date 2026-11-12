#include "UserPolicyView.h"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"

using namespace ftxui;

namespace UI {

void UserPolicyView::loadSession(User* user) {
    currentUser_ = user;
    cachedPolicy_ = std::nullopt;
    errorMessage_.clear();
    selected_ = 0;

    if (currentUser_ && currentUser_->getPolicyIdOpt().has_value()) {
        try {
            cachedPolicy_ = policyRepo_->getById(*currentUser_->getPolicyIdOpt());
            if (!cachedPolicy_.has_value()) {
                errorMessage_ = "Assigned policy (" + *currentUser_->getPolicyIdOpt() + ") was not found in database.";
            }
        } catch (const std::exception& e) {
            errorMessage_ = e.what();
        }
    }
}

UserPolicyView::UserPolicyView(
    std::shared_ptr<AccessPolicyRepo> policyRepo,
    User* currentUser,
    std::function<void()> onBack
) : policyRepo_(policyRepo),
    currentUser_(currentUser),
    onBack_(onBack) {

    loadSession(currentUser_);

    menu_ = Menu(&entries_, &selected_);

    auto handler = CatchEvent(menu_, [this](Event event) {
        if (event == Event::Return || event == Event::Escape) {
            if (onBack_) onBack_();
            return true;
        }
        return false;
    });

    component_ = Renderer(handler, [this]() {
        Elements elements;
        elements.push_back(text("=== MY ACCESS SCHEDULE & POLICY ===") | bold | color(Color::Cyan) | center);
        elements.push_back(separator());

        if (!errorMessage_.empty()) {
            elements.push_back(text("[Error]: " + errorMessage_) | color(Color::Red));
        } else if (!cachedPolicy_.has_value()) {
            elements.push_back(vbox({
                text("No Access Policy is assigned to your account.") | bold | color(Color::Yellow) | center,
                separatorLight(),
                text("Your card access to premises is currently DENIED.") | color(Color::Red) | center,
                text("Please contact an administrator to assign an access schedule.") | dim | center,
            }) | border);
        } else {
            const auto& policy = *cachedPolicy_;
            elements.push_back(hbox({
                text("Assigned Policy: ") | bold,
                text(std::string(policy.getName())) | bold | color(Color::Cyan),
                filler(),
                text("[" + std::to_string(policy.getWindows().size()) + " access window(s)]") | color(Color::Yellow),
            }));
            elements.push_back(text("Policy ID: " + std::string(policy.getId())) | dim);
            elements.push_back(separatorLight());

            if (policy.getWindows().empty()) {
                elements.push_back(text("  No access windows defined in this policy (Access DENIED 24/7).") | color(Color::Red));
            } else {
                elements.push_back(text("Weekly Schedule (Allowed Access Hours):") | bold);
                for (const auto& w : policy.getWindows()) {
                    std::string dayStr = AccessWindow::dayToString(w.getDay());
                    std::string dayName = (dayStr == "MON") ? "Monday   " :
                                          (dayStr == "TUE") ? "Tuesday  " :
                                          (dayStr == "WED") ? "Wednesday" :
                                          (dayStr == "THU") ? "Thursday " :
                                          (dayStr == "FRI") ? "Friday   " :
                                          (dayStr == "SAT") ? "Saturday " : "Sunday   ";

                    elements.push_back(hbox({
                        text("  • " + dayName + " (" + dayStr + "): ") | color(Color::White),
                        text(std::string(w.getFromTime()) + "  ➔  " + std::string(w.getToTime())) | color(Color::Green) | bold,
                    }));
                }
            }
        }

        elements.push_back(separator());
        elements.push_back(menu_->Render() | border);
        elements.push_back(separator());
        elements.push_back(text("[Enter] Back  |  [Esc] Back") | dim | center);

        return vbox(std::move(elements)) | border | size(WIDTH, GREATER_THAN, 65);
    });
}

} // namespace UI
