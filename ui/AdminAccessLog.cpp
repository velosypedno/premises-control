#include "AdminAccessLog.h"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include <fstream>

using namespace ftxui;

namespace UI {


void AdminAccessLog::loadLogs() {
    try {
        cachedRecords_ = logRepo_->list();
        errorMessage_.clear();
    } catch (const std::exception& e) {
        errorMessage_ = e.what();
    }
}

AdminAccessLog::AdminAccessLog(
    std::shared_ptr<AccessLogRecordRepo> logRepo,
    std::function<void()> onBack
) : logRepo_(logRepo), onBack_(onBack) {

    loadLogs();

    menu_ = Menu(&entries_, &selected_);

    auto actionHandler = CatchEvent(menu_, [&](Event event) {
        if (event == Event::Return) {
            if (selected_ == 0) {
                loadLogs();
                return true;
            }
            if (selected_ == 1) {
                onBack_();
                return true;
            }
        }
        if (event == Event::Escape) {
            onBack_();
            return true;
        }
        return false;
    });

    auto renderer = Renderer(actionHandler, [&]() {
        Elements rows;
        rows.push_back(text("=== ACCESS LOG ===") | bold | color(Color::Green) | center);
        rows.push_back(separator());
        rows.push_back(text("Timestamp | CardID | Event") | bold);
        rows.push_back(separator());

        if (!errorMessage_.empty()) {
            rows.push_back(text("[Error] " + errorMessage_) | color(Color::Red));
        } else if (cachedRecords_.empty()) {
            rows.push_back(text("No log records found.") | dim | center);
        } else {
            for (auto& r : cachedRecords_) {
                std::string line =
                    std::string(r.getTimestamp()) + " | " +
                    std::string(r.getCardId()) + " | " +
                    AccessLogRecord::eventTypeToString(r.getEventType());
                rows.push_back(text(line));
            }
        }

        rows.push_back(separator());
        rows.push_back(menu_->Render() | border);
        rows.push_back(separator());
        rows.push_back(text("[Esc] Back") | dim | center);

        return vbox(rows) | border | size(WIDTH, GREATER_THAN, 60);
    });

    component_ = renderer;
}

} // namespace UI
