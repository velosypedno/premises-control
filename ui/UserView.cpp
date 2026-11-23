#include "UserView.h"
#include <cctype>

namespace UI {

namespace {
bool isValidHHMM(const std::string& t) {
    if (t.size() != 5 || t[2] != ':') return false;
    if (!std::isdigit(static_cast<unsigned char>(t[0])) ||
        !std::isdigit(static_cast<unsigned char>(t[1])) ||
        !std::isdigit(static_cast<unsigned char>(t[3])) ||
        !std::isdigit(static_cast<unsigned char>(t[4]))) {
        return false;
    }
    int h = (t[0] - '0') * 10 + (t[1] - '0');
    int m = (t[3] - '0') * 10 + (t[4] - '0');
    return h >= 0 && h <= 23 && m >= 0 && m <= 59;
}
}

UserView::UserView(std::shared_ptr<CardCheck> cardCheck,
                   std::function<void()> onBack,
                   std::function<void(const Card&, const User&)> onSuccess)
    : cardCheckService_(std::move(cardCheck)),
      onBack_(std::move(onBack)),
      onSuccess_(std::move(onSuccess))
{
    using namespace ftxui;

    InputOption passwordOption;
    passwordOption.password = true;

    cardIdBox_ = Input(&cardIdInput, "Card ID (16 digits)");
    cardPinBox_ = Input(&cardPinInput, "PIN Code", passwordOption);
    dayDropdown_ = Dropdown(&dayEntries, &daySelected);
    timeBox_ = Input(&timeInput, "10:00");
    systemTimeCheckbox_ = Checkbox("Use Current Live System Time", &useSystemTime);

    auto attemptLogin = [this]() {
        if (cardIdInput.empty() || cardPinInput.empty()) {
            statusMessage = "[Error]: Please enter both Card ID and PIN code.";
            return;
        }

        auto result = [&]() -> Services::Result<AuthSuccess, AuthError> {
            if (useSystemTime) {
                return cardCheckService_->authenticate(cardIdInput, cardPinInput);
            }
            if (daySelected < 0 || daySelected >= static_cast<int>(dayEntries.size())) {
                return Services::Err(AuthError::DayIsNotAllowed);
            }
            if (!isValidHHMM(timeInput)) {
                statusMessage = "[Error]: Time must be in HH:MM format (00:00-23:59).";
                return Services::Err(AuthError::TimeIsNotAllowed);
            }
            return cardCheckService_->authenticate(
                cardIdInput,
                cardPinInput,
                static_cast<DayOfWeek>(daySelected),
                timeInput
            );
        }();

        if (!useSystemTime && !isValidHHMM(timeInput)) {
            return;
        }

        if (result.is_ok()) {
            const auto& auth = result.value();
            statusMessage = "Access granted. Door unlocked [simulation].";
            if (onSuccess_) {
                onSuccess_(auth.card, auth.user);
            }
        } else {
            if (result.error() == AuthError::PersistFailed) {
                statusMessage = "Failed to save state. Access denied.";
            } else {
                statusMessage = "Access denied. " + toErrorMessage(result.error());
            }
        }
    };

    auto loginButton = Button("Log In", attemptLogin);

    auto slotRow = Container::Horizontal({
        dayDropdown_,
        timeBox_,
    });

    auto container = Container::Vertical({
        cardIdBox_,
        cardPinBox_,
        systemTimeCheckbox_,
        slotRow,
        loginButton,
    });

    auto eventHandler = CatchEvent(container, [this, attemptLogin](Event event) {
        if (event == Event::Escape) {
            cardCheckService_->logCancellation(cardIdInput);
            if (onBack_) {
                onBack_();
            }
            return true;
        }

        if (event == Event::Return) {
            if (systemTimeCheckbox_->Focused() || dayDropdown_->Focused()) {
                return false;
            }
            if (cardIdBox_->Focused() || cardPinBox_->Focused() || timeBox_->Focused()) {
                attemptLogin();
                return true;
            }
        }

        return false;
    });

    component_ = Renderer(eventHandler, [this, loginButton] {
        auto statusCol = (statusMessage.find("Access granted") != std::string::npos) ? Color::Green :
                         (statusMessage.find("Access denied") != std::string::npos ||
                          statusMessage.find("Failed") != std::string::npos ||
                          statusMessage.rfind("[Error]", 0) == 0) ? Color::Red : Color::Yellow;

        Elements children;
        children.push_back(text("=== ACCESS ATTEMPT ===") | bold | color(Color::Blue) | center);
        children.push_back(separator());

        children.push_back(text("Card ID:"));
        children.push_back(cardIdBox_->Render() | border);

        children.push_back(text("PIN Code:"));
        children.push_back(cardPinBox_->Render() | border);

        children.push_back(separatorLight());
        children.push_back(text("Schedule Verification:") | bold);
        children.push_back(systemTimeCheckbox_->Render());

        if (!useSystemTime) {
            children.push_back(hbox({
                text(" Day: ") | center,
                dayDropdown_->Render() | size(WIDTH, EQUAL, 22),
                text(" Time (HH:MM): ") | center,
                timeBox_->Render() | size(WIDTH, EQUAL, 10) | border,
            }));
        } else {
            children.push_back(text("  [Using live system clock for slot check]") | color(Color::Cyan) | dim);
        }

        children.push_back(separatorLight());
        children.push_back(loginButton->Render() | center);

        children.push_back(separator());
        children.push_back(text("[Space/Enter] Toggle/Select  |  [Enter on Input] Login  |  [Esc] Cancel") | dim | center);
        children.push_back(separator());
        children.push_back(text(statusMessage) | color(statusCol) | center);

        return vbox(std::move(children)) | border | size(WIDTH, GREATER_THAN, 62);
    });
}

}
