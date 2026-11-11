#pragma once
#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <array>
#include "ftxui/component/component.hpp"
#include "../models/AccessPolicyRepo.h"
#include "../models/UserRepo.h"

namespace UI {

class AdminManagePolicies {
public:
    AdminManagePolicies(
        std::shared_ptr<AccessPolicyRepo> policyRepo,
        std::shared_ptr<UserRepo> userRepo,
        std::function<void()> onBack
    );
    ftxui::Component getComponent() { return component_; }
    void reset();

private:
    ftxui::Component component_;

    std::shared_ptr<AccessPolicyRepo> policyRepo_;
    std::shared_ptr<UserRepo> userRepo_;
    std::function<void()> onBack_;

    int innerScreen_ = 0;

    // Submenu
    int subSel_ = 0;
    std::vector<std::string> subEntries_ = {
        "List All Policies",
        "Create Policy",
        "Delete Policy",
        "Back"
    };
    ftxui::Component subMenu_;

    // List Policies
    int listActionSel_ = 0;
    std::vector<std::string> listActions_ = {"Back"};
    ftxui::Component listMenu_;

    // Create Policy Form
    std::string createName_;
    std::string createStatus_ = "Configure policy name and weekly access windows.";

    std::array<bool, 7> dayEnabled_ = {true, true, true, true, true, false, false};
    std::array<std::string, 7> dayFrom_ = {"09:00", "09:00", "09:00", "09:00", "09:00", "09:00", "09:00"};
    std::array<std::string, 7> dayTo_ = {"18:00", "18:00", "18:00", "18:00", "18:00", "18:00", "18:00"};
    const std::array<std::string, 7> dayNames_ = {
        "Monday   (MON)",
        "Tuesday  (TUE)",
        "Wednesday(WED)",
        "Thursday (THU)",
        "Friday   (FRI)",
        "Saturday (SAT)",
        "Sunday   (SUN)"
    };

    ftxui::Component createNameInput_;
    std::array<ftxui::Component, 7> dayCheckboxes_;
    std::array<ftxui::Component, 7> dayFromInputs_;
    std::array<ftxui::Component, 7> dayToInputs_;

    int createActionSel_ = 0;
    std::vector<std::string> createActions_ = {
        "Save Policy",
        "Preset: Mon-Fri (09:00-18:00)",
        "Preset: 24/7 (All Days)",
        "Preset: Clear All",
        "Back"
    };
    ftxui::Component createActionMenu_;

    // Delete Policy
    std::string deletePolicyId_;
    std::string deleteStatus_ = "Enter the Policy ID to delete.";
    int deleteActionSel_ = 0;
    std::vector<std::string> deleteActions_ = {"Delete Policy", "Back"};
    ftxui::Component deletePolicyIdInput_;
    ftxui::Component deleteActionMenu_;

    ftxui::Component tabContainer_;
};

} // namespace UI
