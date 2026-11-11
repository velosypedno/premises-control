#pragma once

#include <memory>
#include <functional>
#include <vector>
#include <string>
#include "ftxui/component/component.hpp"
#include "../models/AccessLogRecordRepo.h"

namespace UI {

class AdminAccessLog {
public:
    AdminAccessLog(
        std::shared_ptr<AccessLogRecordRepo> logRepo,
        std::function<void()> onBack
    );
    ftxui::Component getComponent() { return component_; }
    void loadLogs();

private:

    std::shared_ptr<AccessLogRecordRepo> logRepo_;
    std::function<void()> onBack_;

    int selected_ = 0;
    std::vector<std::string> entries_ = {"Refresh", "Back"};
    std::vector<AccessLogRecord> cachedRecords_;
    std::string errorMessage_;

    ftxui::Component menu_;
    ftxui::Component component_;
};

} // namespace UI
