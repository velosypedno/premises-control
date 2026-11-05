#include <iostream>
#include "Database.h"
#include "models/User.h"
#include "models/Card.h"
#include "models/AccessPolicy.h"
#include "models/AccessLogRecord.h"
#include "models/UserRepo.h"
#include "models/CardRepo.h"
#include "models/AccessPolicyRepo.h"
#include "models/AccessLogRecordRepo.h"

void runPolicyTests(AccessPolicyRepo& policyRepo, const User& u1, const User& u2) {
    std::cout << "--- Checking policy search by user ID (Maksym) ---:\n";
    if (auto foundPolicy = policyRepo.getByUserId(std::string(u1.getId()))) {
        std::cout << "Policy found: " << foundPolicy->getName() << " (ID: " << foundPolicy->getId() << ")\n";
        std::cout << "Number of access windows: " << foundPolicy->getWindows().size() << '\n';

        bool allowedMon10 = foundPolicy->isAllowed(DayOfWeek::Monday, "10:00");
        bool allowedMon19 = foundPolicy->isAllowed(DayOfWeek::Monday, "19:00");
        std::cout << "Access on Monday at 10:00 (expected true): " << (allowedMon10 ? "ALLOWED" : "DENIED") << '\n';
        std::cout << "Access on Monday at 19:00 (expected false): " << (allowedMon19 ? "ALLOWED" : "DENIED") << '\n';
    }

    std::cout << "\n--- Checking policy search for Kateryna (has no policy) ---:\n";
    auto noPolicy = policyRepo.getByUserId(std::string(u2.getId()));
    std::cout << "Policy found: " << (noPolicy.has_value() ? "Yes" : "No") << '\n';

    std::cout << "\n--- List of all policies in the system ---:\n";
    for (const auto& pol : policyRepo.list()) {
        std::cout << "Policy: " << pol.getName() << " | Windows: " << pol.getWindows().size() << '\n';
    }
}

void runLogTests(AccessLogRepo& logRepo, const Card& c1, const Card& c2) {
    std::cout << "\n=== Access Log Testing ===\n";

    logRepo.create(AccessLogRecord("2026-10-12 09:10:00", std::string(c1.getCardId()), LogEventType::Allowed));
    logRepo.create(AccessLogRecord("2026-10-12 09:12:00", std::string(c2.getCardId()), LogEventType::WrongPin));
    logRepo.create(AccessLogRecord("2026-10-12 09:13:00", std::string(c2.getCardId()), LogEventType::Blocked));
    logRepo.create(AccessLogRecord("2026-10-12 14:30:00", std::string(c1.getCardId()), LogEventType::TimeIsNotAllowed));

    std::cout << "Test log records successfully created.\n";

    std::string targetCardId(c2.getCardId());
    std::cout << "\n--- Log records for card ID: " << targetCardId << " ---:\n";
    auto c2Logs = logRepo.getByCardId(targetCardId);
    for (const auto& log : c2Logs) {
        std::cout << "Time: " << log.getTimestamp()
                  << " | Card: " << log.getCardId()
                  << " | Event: " << AccessLogRecord::eventTypeToString(log.getEventType()) << '\n';
    }

    std::cout << "\n--- Full access log list (all records) ---:\n";
    for (const auto& log : logRepo.list()) {
        std::cout << "Time: " << log.getTimestamp()
                  << " | Card: " << log.getCardId()
                  << " | Event: " << AccessLogRecord::eventTypeToString(log.getEventType()) << '\n';
    }
}

int main() {
    try {
        Config config = Config::loadFromEnv();
        std::cout << "Using database: " << config.dbPath << '\n';

        auto db = Database::openConnection(config.dbPath);

        Database::initializeSchema(db.get(), config.schemaPath);
        Database::clear(db.get());

        UserRepo userRepo(db);
        CardRepo cardRepo(db);
        AccessPolicyRepo policyRepo(db);
        AccessLogRepo logRepo(db);

        AccessPolicy workHoursPolicy("Standard Working Hours");
        workHoursPolicy.addWindow(AccessWindow(std::string(workHoursPolicy.getId()), DayOfWeek::Monday, "09:00", "18:00"));
        workHoursPolicy.addWindow(AccessWindow(std::string(workHoursPolicy.getId()), DayOfWeek::Friday, "09:00", "17:00"));
        policyRepo.create(workHoursPolicy);
        std::cout << "Access policy '" << workHoursPolicy.getName() << "' saved to the database.\n";

        User u1("Maksym", "Shevchenko");
        userRepo.create(u1);
        u1.setPolicyId(std::string(workHoursPolicy.getId()));
        userRepo.update(u1);

        User u2("Kateryna", "Bondarenko");
        userRepo.create(u2);
        std::cout << "Users created and assigned to policies.\n";

        Card c1(std::string(u1.getId()), "1234567890123456", "hashed_pin_1");
        Card c2(std::string(u2.getId()), "9876543210987654", "hashed_pin_2");
        cardRepo.create(c1);
        cardRepo.create(c2);
        std::cout << "Cards created.\n\n";

        runPolicyTests(policyRepo, u1, u2);

        runLogTests(logRepo, c1, c2);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}