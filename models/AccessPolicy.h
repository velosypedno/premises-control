#pragma once

#include <string>
#include <string_view>
#include <vector>

enum class DayOfWeek {
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday,
    Sunday
};

class AccessWindow {
private:
    std::string id;
    std::string policyId;
    DayOfWeek day;
    std::string fromTime;   // "HH:MM"
    std::string toTime;     // "HH:MM"

    static bool isValidTimeFormat(std::string_view time);

public:
    AccessWindow() = default;

    AccessWindow(std::string id, std::string policyId, DayOfWeek day, std::string fromTime, std::string toTime);
    AccessWindow(std::string policyId, DayOfWeek day, std::string fromTime, std::string toTime);

    std::string_view getId() const noexcept;
    std::string_view getPolicyId() const noexcept;
    DayOfWeek getDay() const noexcept;
    std::string_view getFromTime() const noexcept;
    std::string_view getToTime() const noexcept;

    void setDay(DayOfWeek newDay) noexcept;
    void setFromTime(std::string newFromTime);
    void setToTime(std::string newToTime);

    bool isTimeAllowed(std::string_view time) const;

    static std::string dayToString(DayOfWeek day);
    static DayOfWeek stringToDay(std::string_view str);
};

class AccessPolicy {
private:
    std::string id;
    std::string name;
    std::vector<AccessWindow> windows;

public:
    AccessPolicy() = default;
    AccessPolicy(std::string id, std::string name, std::vector<AccessWindow> windows = {});
    explicit AccessPolicy(std::string name);

    std::string_view getId() const noexcept;
    std::string_view getName() const noexcept;
    const std::vector<AccessWindow>& getWindows() const noexcept;

    void setName(std::string newName);

    void addWindow(AccessWindow window);
    void setWindows(std::vector<AccessWindow> newWindows);
    void clearWindows() noexcept;

    bool isAllowed(DayOfWeek day, std::string_view time) const;
};