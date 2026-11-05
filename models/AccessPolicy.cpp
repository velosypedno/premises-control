#include "AccessPolicy.h"
#include "uuid.h"
#include <algorithm>
#include <stdexcept>
#include <regex>


bool AccessWindow::isValidTimeFormat(std::string_view time) {
    if (time.size() != 5) return false;
    if (time[2] != ':') return false;

    if (!std::isdigit(time[0]) || !std::isdigit(time[1]) ||
        !std::isdigit(time[3]) || !std::isdigit(time[4])) {
        return false;
    }

    int hours = (time[0] - '0') * 10 + (time[1] - '0');
    int minutes = (time[3] - '0') * 10 + (time[4] - '0');

    return hours >= 0 && hours <= 23 && minutes >= 0 && minutes <= 59;
}

AccessWindow::AccessWindow(std::string id, std::string policyId, DayOfWeek day, std::string fromTime, std::string toTime)
    : id(std::move(id)), policyId(std::move(policyId)), day(day) {

    setFromTime(fromTime);
    setToTime(toTime);
}

AccessWindow::AccessWindow(std::string policyId, DayOfWeek day, std::string fromTime, std::string toTime)
    : id(uuid::generate()), policyId(std::move(policyId)), day(day) {

    setFromTime(fromTime);
    setToTime(toTime);
}

std::string_view AccessWindow::getId() const noexcept {
    return id;
}

std::string_view AccessWindow::getPolicyId() const noexcept {
    return policyId;
}

DayOfWeek AccessWindow::getDay() const noexcept {
    return day;
}

std::string_view AccessWindow::getFromTime() const noexcept {
    return fromTime;
}

std::string_view AccessWindow::getToTime() const noexcept {
    return toTime;
}

void AccessWindow::setDay(DayOfWeek newDay) noexcept {
    day = newDay;
}

void AccessWindow::setFromTime(std::string newFromTime) {
    if (!isValidTimeFormat(newFromTime)) {
        throw std::invalid_argument("Invalid 'fromTime' format. Use HH:MM (e.g., 09:00).");
    }
    if (!toTime.empty() && newFromTime >= toTime) {
        throw std::invalid_argument("Start time ('fromTime') must be earlier than end time ('toTime').");
    }
    fromTime = std::move(newFromTime);
}

void AccessWindow::setToTime(std::string newToTime) {
    if (!isValidTimeFormat(newToTime)) {
        throw std::invalid_argument("Invalid 'toTime' format. Use HH:MM (e.g., 18:00).");
    }
    if (!fromTime.empty() && newToTime <= fromTime) {
        throw std::invalid_argument("End time ('toTime') must be later than start time ('fromTime').");
    }
    toTime = std::move(newToTime);
}

bool AccessWindow::isTimeAllowed(std::string_view time) const {
    if (!isValidTimeFormat(time)) {
        return false;
    }
    return time >= fromTime && time < toTime;
}

std::string AccessWindow::dayToString(DayOfWeek day) {
    switch (day) {
        case DayOfWeek::Monday:    return "MON";
        case DayOfWeek::Tuesday:   return "TUE";
        case DayOfWeek::Wednesday: return "WED";
        case DayOfWeek::Thursday:  return "THU";
        case DayOfWeek::Friday:    return "FRI";
        case DayOfWeek::Saturday:  return "SAT";
        case DayOfWeek::Sunday:    return "SUN";
    }
    return "MON";
}

DayOfWeek AccessWindow::stringToDay(std::string_view str) {
    if (str == "TUE") return DayOfWeek::Tuesday;
    if (str == "WED") return DayOfWeek::Wednesday;
    if (str == "THU") return DayOfWeek::Thursday;
    if (str == "FRI") return DayOfWeek::Friday;
    if (str == "SAT") return DayOfWeek::Saturday;
    if (str == "SUN") return DayOfWeek::Sunday;
    return DayOfWeek::Monday;
}

AccessPolicy::AccessPolicy(std::string id, std::string name, std::vector<AccessWindow> windows)
    : id(std::move(id)), name(std::move(name)), windows(std::move(windows)) {}

AccessPolicy::AccessPolicy(std::string name)
    : id(uuid::generate()), name(std::move(name)) {}

std::string_view AccessPolicy::getId() const noexcept {
    return id;
}

std::string_view AccessPolicy::getName() const noexcept {
    return name;
}

const std::vector<AccessWindow>& AccessPolicy::getWindows() const noexcept {
    return windows;
}

void AccessPolicy::setName(std::string newName) {
    if (newName.empty()) {
        throw std::invalid_argument("Policy name cannot be empty.");
    }
    name = std::move(newName);
}

void AccessPolicy::addWindow(AccessWindow window) {
    windows.push_back(std::move(window));
}

void AccessPolicy::setWindows(std::vector<AccessWindow> newWindows) {
    windows = std::move(newWindows);
}

void AccessPolicy::clearWindows() noexcept {
    windows.clear();
}

bool AccessPolicy::isAllowed(DayOfWeek day, std::string_view time) const {
    return std::any_of(windows.begin(), windows.end(), [day, time](const AccessWindow& window) {
        return window.getDay() == day && window.isTimeAllowed(time);
    });
}