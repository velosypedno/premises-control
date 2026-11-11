#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "ftxui/component/component.hpp"
#include "../models/UserRepo.h"
#include "../models/CardRepo.h"
#include "../models/AccessPolicyRepo.h"
#include "../models/AccessLogRecordRepo.h"
#include "../services/CryptoService.h"

namespace UI {
    class AdminEditProfile;
    class AdminManageUsers;
    class AdminManageCards;
    class AdminManagePolicies;
    class AdminAccessLog;

class AdminDashboard {
public:
    AdminDashboard(
        std::function<void()> onLogout,
        std::shared_ptr<UserRepo> userRepo,
        std::shared_ptr<CardRepo> cardRepo,
        std::shared_ptr<AccessPolicyRepo> policyRepo,
        std::shared_ptr<AccessLogRecordRepo> logRepo,
        std::shared_ptr<CryptoService> cryptoService
    );

    ftxui::Component getComponent() { return component_; }
    void setCurrentUser(User user);
    void resetStatus();

private:
    ftxui::Component component_;

    std::shared_ptr<UserRepo> userRepo_;
    std::shared_ptr<CardRepo> cardRepo_;
    std::shared_ptr<AccessPolicyRepo> policyRepo_;
    std::shared_ptr<AccessLogRecordRepo> logRepo_;
    std::shared_ptr<CryptoService> cryptoService_;
    std::function<void()> onLogout_;

    User currentUser_;
    int selectedTab_ = 0;
    int mainSelected_ = 0;

    std::vector<std::string> menuEntries_;
    ftxui::Component mainMenu_;
    ftxui::Component tabContainer_;

    std::shared_ptr<AdminEditProfile> editProfile_;
    std::shared_ptr<AdminManageUsers> manageUsers_;
    std::shared_ptr<AdminManageCards> manageCards_;
    std::shared_ptr<AdminManagePolicies> managePolicies_;
    std::shared_ptr<AdminAccessLog> accessLog_;
};

} // namespace UI
