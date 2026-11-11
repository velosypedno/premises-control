#include "AdminManagePolicies.h"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include <string>
#include <vector>
#include <cctype>

using namespace ftxui;

namespace UI {

static bool validateTime(const std::string& t) {
    if (t.size() != 5 || t[2] != ':') return false;
    if (!std::isdigit(t[0]) || !std::isdigit(t[1]) || !std::isdigit(t[3]) || !std::isdigit(t[4])) return false;
    int h = (t[0] - '0') * 10 + (t[1] - '0');
    int m = (t[3] - '0') * 10 + (t[4] - '0');
    return h >= 0 && h <= 23 && m >= 0 && m <= 59;
}

void AdminManagePolicies::reset() {
    innerScreen_ = 0;
    subSel_ = 0;
    createName_.clear();
    createStatus_ = "Configure policy name and weekly access windows.";
    dayEnabled_ = {true, true, true, true, true, false, false};
    for (int i = 0; i < 7; ++i) {
        dayFrom_[i] = "09:00";
        dayTo_[i] = "18:00";
    }
    createActionSel_ = 0;
    deletePolicyId_.clear();
    deleteStatus_ = "Enter the Policy ID to delete.";
    deleteActionSel_ = 0;
    listActionSel_ = 0;
}

AdminManagePolicies::AdminManagePolicies(
    std::shared_ptr<AccessPolicyRepo> policyRepo,
    std::shared_ptr<UserRepo> userRepo,
    std::function<void()> onBack
) : policyRepo_(policyRepo),
    userRepo_(userRepo),
    onBack_(onBack) {

    subMenu_ = Menu(&subEntries_, &subSel_);

    auto subMenuHandler = CatchEvent(subMenu_, [this](Event event) {
        if (event == Event::Return) {
            int sel = subSel_;
            if (sel == 3) {
                if (onBack_) onBack_();
            } else {
                innerScreen_ = sel + 1;
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
            text("=== MANAGE ACCESS POLICIES ===") | bold | color(Color::Green) | center,
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
        rows.push_back(text("=== ACCESS POLICIES & WEEKLY SCHEDULES ===") | bold | color(Color::Green) | center);
        rows.push_back(separator());

        try {
            auto policies = policyRepo_->list();
            if (policies.empty()) {
                rows.push_back(text("No policies found in the system.") | dim | center);
            } else {
                for (auto& p : policies) {
                    Elements card;
                    card.push_back(hbox({
                        text("Policy: ") | bold,
                        text(std::string(p.getName())) | bold | color(Color::Cyan),
                        filler(),
                        text("[" + std::to_string(p.getWindows().size()) + " window(s)]") | color(Color::Yellow),
                    }));
                    card.push_back(text("ID: " + std::string(p.getId())) | dim);
                    card.push_back(separatorLight());

                    if (p.getWindows().empty()) {
                        card.push_back(text("  No access windows defined (Access is DENIED 24/7)") | color(Color::Red));
                    } else {
                        for (const auto& w : p.getWindows()) {
                            std::string dayStr = AccessWindow::dayToString(w.getDay());
                            std::string dayName = (dayStr == "MON") ? "Monday   " :
                                                  (dayStr == "TUE") ? "Tuesday  " :
                                                  (dayStr == "WED") ? "Wednesday" :
                                                  (dayStr == "THU") ? "Thursday " :
                                                  (dayStr == "FRI") ? "Friday   " :
                                                  (dayStr == "SAT") ? "Saturday " : "Sunday   ";

                            card.push_back(hbox({
                                text("  • " + dayName + " (" + dayStr + "): ") | color(Color::White),
                                text(std::string(w.getFromTime()) + "  ➔  " + std::string(w.getToTime())) | color(Color::Green) | bold,
                            }));
                        }
                    }
                    rows.push_back(vbox(std::move(card)) | border);
                }
            }
        } catch (const std::exception& e) {
            rows.push_back(text(std::string("[Error] ") + e.what()) | color(Color::Red));
        }

        rows.push_back(separator());
        rows.push_back(listMenu_->Render() | border);
        rows.push_back(separator());
        rows.push_back(text("[Enter] Select  |  [Esc] Back") | dim | center);
        return vbox(std::move(rows)) | border | size(WIDTH, GREATER_THAN, 68);
    });

    createNameInput_ = Input(&createName_, "e.g. Standard Office Hours");

    for (int i = 0; i < 7; ++i) {
        dayCheckboxes_[i] = Checkbox(dayNames_[i], &dayEnabled_[i]);
        dayFromInputs_[i] = Input(&dayFrom_[i], "09:00");
        dayToInputs_[i] = Input(&dayTo_[i], "18:00");
    }

    createActionMenu_ = Menu(&createActions_, &createActionSel_);

    Components createRows;
    createRows.push_back(createNameInput_);
    for (int i = 0; i < 7; ++i) {
        createRows.push_back(Container::Horizontal({
            dayCheckboxes_[i],
            dayFromInputs_[i],
            dayToInputs_[i],
        }));
    }
    createRows.push_back(createActionMenu_);

    auto createContainer = Container::Vertical(std::move(createRows));

    auto createHandler = CatchEvent(createContainer, [this](Event event) {
        if (event == Event::Return) {
            if (createActionMenu_->Focused()) {
                if (createActionSel_ == 4) { // Back
                    innerScreen_ = 0;
                    return true;
                }
                if (createActionSel_ == 1) { // Preset: Mon-Fri
                    for (int i = 0; i < 5; ++i) {
                        dayEnabled_[i] = true;
                        dayFrom_[i] = "09:00";
                        dayTo_[i] = "18:00";
                    }
                    dayEnabled_[5] = false;
                    dayEnabled_[6] = false;
                    createStatus_ = "[Info] Applied Mon-Fri (09:00 - 18:00) preset.";
                    return true;
                }
                if (createActionSel_ == 2) { // Preset: 24/7
                    for (int i = 0; i < 7; ++i) {
                        dayEnabled_[i] = true;
                        dayFrom_[i] = "00:00";
                        dayTo_[i] = "23:59";
                    }
                    createStatus_ = "[Info] Applied 24/7 (All Days) preset.";
                    return true;
                }
                if (createActionSel_ == 3) { // Preset: Clear All
                    for (int i = 0; i < 7; ++i) {
                        dayEnabled_[i] = false;
                    }
                    createStatus_ = "[Info] Cleared all days.";
                    return true;
                }
                if (createActionSel_ == 0) { // Save Policy
                    if (createName_.empty()) {
                        createStatus_ = "[Error] Policy name cannot be empty.";
                        return true;
                    }

                    int enabledCount = 0;
                    for (int i = 0; i < 7; ++i) {
                        if (dayEnabled_[i]) {
                            enabledCount++;
                            if (!validateTime(dayFrom_[i]) || !validateTime(dayTo_[i])) {
                                createStatus_ = "[Error] " + dayNames_[i] + ": Time must be HH:MM format (00:00-23:59).";
                                return true;
                            }
                            if (dayFrom_[i] >= dayTo_[i]) {
                                createStatus_ = "[Error] " + dayNames_[i] + ": 'From' (" + dayFrom_[i] + ") must be earlier than 'To' (" + dayTo_[i] + ").";
                                return true;
                            }
                        }
                    }

                    if (enabledCount == 0) {
                        createStatus_ = "[Error] Select at least one active day for access.";
                        return true;
                    }

                    try {
                        AccessPolicy policy(createName_);
                        for (int i = 0; i < 7; ++i) {
                            if (dayEnabled_[i]) {
                                DayOfWeek day = static_cast<DayOfWeek>(i);
                                policy.addWindow(AccessWindow(std::string(policy.getId()), day, dayFrom_[i], dayTo_[i]));
                            }
                        }

                        policyRepo_->create(policy);
                        createStatus_ = "[Success] Policy '" + createName_ + "' created with " + std::to_string(enabledCount) + " windows!";
                        createName_.clear();
                    } catch (const std::exception& e) {
                        createStatus_ = std::string("[Error] ") + e.what();
                    }
                    return true;
                }
            }
            return false;
        }
        if (event == Event::Escape) {
            innerScreen_ = 0;
            return true;
        }
        return false;
    });

    auto createRenderer = Renderer(createHandler, [this]() {
        auto statusCol = (createStatus_.rfind("[Success]", 0) == 0) ? Color::Green :
                         (createStatus_.rfind("[Error]", 0) == 0) ? Color::Red :
                         (createStatus_.rfind("[Info]", 0) == 0) ? Color::Cyan : Color::Yellow;

        Elements formElements;
        formElements.push_back(text("=== CREATE ACCESS POLICY ===") | bold | color(Color::Green) | center);
        formElements.push_back(separator());

        formElements.push_back(text("Policy Name:"));
        formElements.push_back(createNameInput_->Render() | border);
        formElements.push_back(separator());

        formElements.push_back(text("Weekly Access Windows (Day / Allowed Hours):") | bold);
        formElements.push_back(text("Tip: Press [Space] or [Enter] on a day to toggle active, [Arrows] to edit From/To hours.") | dim);
        formElements.push_back(separatorLight());

        for (int i = 0; i < 7; ++i) {
            auto dayColor = dayEnabled_[i] ? Color::Green : Color::GrayDark;
            formElements.push_back(hbox({
                dayCheckboxes_[i]->Render() | size(WIDTH, EQUAL, 20),
                separatorLight(),
                text(" From: ") | (dayEnabled_[i] ? color(Color::White) : dim),
                dayFromInputs_[i]->Render() | size(WIDTH, EQUAL, 9) | border,
                text(" To: ") | (dayEnabled_[i] ? color(Color::White) : dim),
                dayToInputs_[i]->Render() | size(WIDTH, EQUAL, 9) | border,
                filler(),
                text(dayEnabled_[i] ? " [ACTIVE] " : " [CLOSED] ") | color(dayColor) | bold,
            }));
        }

        formElements.push_back(separator());
        formElements.push_back(createActionMenu_->Render() | border);
        formElements.push_back(separator());
        formElements.push_back(text(createStatus_) | color(statusCol) | center);
        formElements.push_back(separator());
        formElements.push_back(text("[Enter] Select / Toggle  |  [Esc] Back") | dim | center);

        return vbox(std::move(formElements)) | border | size(WIDTH, GREATER_THAN, 68);
    });

    deletePolicyIdInput_ = Input(&deletePolicyId_, "Policy ID");
    deleteActionMenu_ = Menu(&deleteActions_, &deleteActionSel_);
    auto deleteContainer = Container::Vertical({deletePolicyIdInput_, deleteActionMenu_});

    auto deleteHandler = CatchEvent(deleteContainer, [this](Event event) {
        if (event == Event::Return) {
            if (deleteActionMenu_->Focused() && deleteActionSel_ == 1) {
                innerScreen_ = 0;
                return true;
            }
            if ((deleteActionMenu_->Focused() && deleteActionSel_ == 0) || !deleteActionMenu_->Focused()) {
                if (deletePolicyId_.empty()) {
                    deleteStatus_ = "[Error] Policy ID cannot be empty.";
                    return true;
                }
                try {
                    auto policyOpt = policyRepo_->getById(deletePolicyId_);
                    if (!policyOpt.has_value()) {
                        deleteStatus_ = "[Error] Policy not found.";
                        return true;
                    }
                    policyRepo_->remove(deletePolicyId_);
                    deleteStatus_ = "[Success] Policy deleted.";
                    deletePolicyId_.clear();
                } catch (const std::exception& e) {
                    deleteStatus_ = std::string("[Error] ") + e.what();
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

    auto deleteRenderer = Renderer(deleteHandler, [this]() {
        auto statusCol = (deleteStatus_.rfind("[Success]", 0) == 0) ? Color::Green :
                         (deleteStatus_.rfind("[Error]", 0) == 0) ? Color::Red : Color::Yellow;
        return vbox({
            text("=== DELETE POLICY ===") | bold | color(Color::Green) | center,
            separator(),
            text("Policy ID:"),
            deletePolicyIdInput_->Render() | border,
            separator(),
            deleteActionMenu_->Render() | border,
            separator(),
            text(deleteStatus_) | color(statusCol) | center,
            separator(),
            text("[Esc] Back") | dim | center,
        }) | border | size(WIDTH, GREATER_THAN, 60);
    });

    tabContainer_ = Container::Tab({
        subMenuRenderer,
        listRenderer,
        createRenderer,
        deleteRenderer,
    }, &innerScreen_);

    component_ = tabContainer_;
}

} // namespace UI
