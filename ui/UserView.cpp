#include "UserView.h"
#include <chrono>
#include <ctime>
#include <cstdio>

namespace UI {

UserView::UserView(std::shared_ptr<CardRepo> cardRepo,
                   std::shared_ptr<UserRepo> userRepo,
                   std::shared_ptr<AccessPolicyRepo> policyRepo,
                   std::shared_ptr<AccessLogRecordRepo> logRepo,
                   std::shared_ptr<CryptoService> cryptoService,
                   std::function<void()> onBack,
                   std::function<void(const Card&, const User&)> onSuccess)
    : cardRepo_(cardRepo),
      userRepo_(userRepo),
      policyRepo_(policyRepo),
      logRepo_(logRepo),
      cryptoService_(cryptoService),
      cardCheckService_(cardRepo, userRepo, policyRepo, logRepo, cryptoService, 3),
      onBack_(onBack),
      onSuccess_(onSuccess)
{
    using namespace ftxui;

    InputOption passwordOption;
    passwordOption.password = true;

    cardIdBox_ = Input(&cardIdInput, "Card ID (16 digits)");
    cardPinBox_ = Input(&cardPinInput, "PIN Code", passwordOption);
    dayDropdown_ = Dropdown(&dayEntries, &daySelected);
    timeBox_ = Input(&timeInput, "10:00");
    systemTimeCheckbox_ = Checkbox("Use Current Live System Time", &useSystemTime);

    auto slotRow = Container::Horizontal({
        dayDropdown_,
        timeBox_,
    });

    auto container = Container::Vertical({
        cardIdBox_,
        cardPinBox_,
        systemTimeCheckbox_,
        slotRow,
    });

    auto eventHandler = CatchEvent(container, [this](Event event) {
        if (event == Event::Return) {
            if (cardIdInput.empty() || cardPinInput.empty()) {
                statusMessage = "[Error]: Please enter both Card ID and PIN code.";
                return true;
            }

            DayOfWeek checkDay;
            std::string checkTime;

            if (useSystemTime) {
                auto now = std::chrono::system_clock::now();
                std::time_t t = std::chrono::system_clock::to_time_t(now);
                std::tm tm = *std::localtime(&t);

                switch (tm.tm_wday) {
                    case 1: checkDay = DayOfWeek::Monday; break;
                    case 2: checkDay = DayOfWeek::Tuesday; break;
                    case 3: checkDay = DayOfWeek::Wednesday; break;
                    case 4: checkDay = DayOfWeek::Thursday; break;
                    case 5: checkDay = DayOfWeek::Friday; break;
                    case 6: checkDay = DayOfWeek::Saturday; break;
                    case 0:
                    default: checkDay = DayOfWeek::Sunday; break;
                }

                char buf[6];
                std::snprintf(buf, sizeof(buf), "%02d:%02d", tm.tm_hour, tm.tm_min);
                checkTime = std::string(buf);
            } else {
                checkDay = static_cast<DayOfWeek>(daySelected);
                checkTime = timeInput;
            }

            bool ok = cardCheckService_.authenticate(cardIdInput, cardPinInput, checkDay, checkTime);

            if (ok) {
                auto cardOpt = cardRepo_->getByCardId(cardIdInput);
                auto userOpt = userRepo_->getById(std::string(cardOpt->getUserId()));
                statusMessage = "Доступ дозволено. Двері відкрито [симуляція].";
                if (onSuccess_ && cardOpt && userOpt) {
                    onSuccess_(*cardOpt, *userOpt);
                }
            } else {
                auto lastEv = cardCheckService_.getLastEventType();
                if (lastEv.has_value()) {
                    if (*lastEv == LogEventType::PersistFailed) {
                        statusMessage = "Помилка збереження. Доступ не надано.";
                    } else {
                        statusMessage = "Доступ відхилено. " + CardCheck::eventTypeToMessage(*lastEv);
                    }
                } else {
                    statusMessage = "Доступ відхилено.";
                }
            }
            return true;
        }

        if (event == Event::Escape) {
            cardCheckService_.logCancellation(cardIdInput);
            if (onBack_) {
                onBack_();
            }
            return true;
        }
        return false;
    });

    component = Renderer(eventHandler, [this] {
        auto statusCol = (statusMessage.find("Доступ дозволено") != std::string::npos) ? Color::Green :
                         (statusMessage.find("Доступ відхилено") != std::string::npos ||
                          statusMessage.find("Помилка") != std::string::npos ||
                          statusMessage.rfind("[Error]", 0) == 0) ? Color::Red : Color::Yellow;

        Elements children;
        children.push_back(text("=== СПРОБА ВХОДУ ===") | bold | color(Color::Blue) | center);
        children.push_back(separator());

        children.push_back(text("Card ID:"));
        children.push_back(cardIdBox_->Render() | border);

        children.push_back(text("PIN Code:"));
        children.push_back(cardPinBox_->Render() | border);

        children.push_back(separatorLight());
        children.push_back(text("Перевірка розкладу:") | bold);
        children.push_back(systemTimeCheckbox_->Render());

        if (!useSystemTime) {
            children.push_back(hbox({
                text(" Day: ") | center,
                dayDropdown_->Render() | size(WIDTH, EQUAL, 20),
                text(" Time (HH:MM): ") | center,
                timeBox_->Render() | size(WIDTH, EQUAL, 10) | border,
            }));
        } else {
            children.push_back(text("  [Using live system clock for slot check]") | color(Color::Cyan) | dim);
        }

        children.push_back(separator());
        children.push_back(text("[Enter] - Спроба входу  |  [Esc] - Скасувати") | dim | center);
        children.push_back(separator());
        children.push_back(text(statusMessage) | color(statusCol) | center);

        return vbox(std::move(children)) | border | size(WIDTH, GREATER_THAN, 62);
    });
}

} // namespace UI
