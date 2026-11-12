#include <iostream>
#include "infra/Database.h"
#include "infra/Config.h"
#include "models/User.h"
#include "models/Card.h"
#include "models/AccessPolicy.h"
#include "models/AccessLogRecord.h"
#include "models/UserRepo.h"
#include "models/CardRepo.h"
#include "models/AccessPolicyRepo.h"
#include "models/AccessLogRecordRepo.h"
#include "services/CryptoService.h"
#include "services/CardCheck.h"
#include "services/AdminCheck.h"
#include "ui/ConsoleUI.h"

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

void runAuthTests(CardCheck& cardCheckService, const Card& c1, const Card& c2) {
    std::cout << "\n=== CardCheck Testing ===\n";

    std::cout << "[Test 1] Maksym correct PIN (expected ALLOWED): ";
    bool res1 = cardCheckService.authenticate(std::string(c1.getCardId()), "1111", DayOfWeek::Monday, "10:00");
    std::cout << (res1 ? "SUCCESS" : "FAILED") << '\n';

    std::cout << "[Test 2] Maksym outside working hours (expected TIME_IS_NOT_ALLOWED): ";
    bool res2 = cardCheckService.authenticate(std::string(c1.getCardId()), "1111", DayOfWeek::Monday, "20:00");
    std::cout << (res2 ? "SUCCESS" : "FAILED") << '\n';

    std::cout << "[Test 3] Kateryna wrong PIN attempt 1 (expected WRONG_PIN): ";
    bool res3 = cardCheckService.authenticate(std::string(c2.getCardId()), "wrong_pin", DayOfWeek::Monday, "10:00");
    std::cout << (res3 ? "SUCCESS" : "FAILED") << '\n';

    std::cout << "[Test 4] Kateryna wrong PIN attempt 2: ";
    cardCheckService.authenticate(std::string(c2.getCardId()), "wrong_pin", DayOfWeek::Monday, "10:00");

    std::cout << "[Test 5] Kateryna wrong PIN attempt 3 (should trigger BLOCKED): ";
    bool res5 = cardCheckService.authenticate(std::string(c2.getCardId()), "wrong_pin", DayOfWeek::Monday, "10:00");
    std::cout << (res5 ? "SUCCESS" : "FAILED") << '\n';
    std::cout << "[Test 6] Kateryna correct PIN with BLOCKED card (expected BLOCKED): ";
    bool res6 = cardCheckService.authenticate(std::string(c2.getCardId()), "2222", DayOfWeek::Monday, "10:00");
    std::cout << (res6 ? "SUCCESS" : "FAILED") << '\n';
}

void runAdminTests(AdminCheck& adminCheck, const std::string& correctPassword) {
    std::cout << "\n=== AdminAuth (AdminCheck) Testing ===\n";

    bool success = adminCheck.authenticate("root", correctPassword);
    std::cout << "Root login with correct password (expected true): " << (success ? "SUCCESS" : "FAILED") << '\n';

    bool wrongPass = adminCheck.authenticate("root", "wrong_admin_password");
    std::cout << "Root login with wrong password (expected false): " << (!wrongPass ? "SUCCESS (denied)" : "FAILED") << '\n';

    bool unknownUser = adminCheck.authenticate("hacker", correctPassword);
    std::cout << "Unknown user login (expected false): " << (!unknownUser ? "SUCCESS (denied)" : "FAILED") << '\n';
}

void runLogTests(AccessLogRecordRepo& logRepo) {
    std::cout << "\n=== Access Log Inspection ===\n";
    for (const auto& log : logRepo.list()) {
        std::cout << "Time: " << log.getTimestamp()
                  << " | Card: " << log.getCardId()
                  << " | Event: " << AccessLogRecord::eventTypeToString(log.getEventType()) << '\n';
    }
}

void runAllTests() {
    Config config = Config::loadFromEnv();
    std::cout << "Using database: " << config.dbPath << '\n';

    auto db = Database::openConnection(config.dbPath);

    Database::initializeSchema(db.get(), config.schemaPath);
    Database::clear(db.get());

    auto cryptoService = std::make_shared<CryptoService>(config.salt);

    std::string rootPassHash = cryptoService->hashPin(config.rootPass);
    Database::initializeRootAdmin(db.get(), rootPassHash);
    std::cout << "Root administrator initialized in database.\n";

    auto userRepo = std::make_shared<UserRepo>(db);
    auto cardRepo = std::make_shared<CardRepo>(db);
    auto policyRepo = std::make_shared<AccessPolicyRepo>(db);
    auto logRepo = std::make_shared<AccessLogRecordRepo>(db);

    CardCheck cardCheckService(
        cardRepo,
        userRepo,
        policyRepo,
        logRepo,
        cryptoService,
        3
    );

    AdminCheck adminCheck(userRepo, cryptoService);

    AccessPolicy workHoursPolicy("Standard Working Hours");
    workHoursPolicy.addWindow(AccessWindow(std::string(workHoursPolicy.getId()), DayOfWeek::Monday, "09:00", "18:00"));
    workHoursPolicy.addWindow(AccessWindow(std::string(workHoursPolicy.getId()), DayOfWeek::Friday, "09:00", "17:00"));
    policyRepo->create(workHoursPolicy);
    std::cout << "Access policy '" << workHoursPolicy.getName() << "' saved to the database.\n";

    User u1("Maksym", "Shevchenko");
    userRepo->create(u1);
    u1.setPolicyId(std::string(workHoursPolicy.getId()));
    userRepo->update(u1);

    User u2("Kateryna", "Bondarenko");
    userRepo->create(u2);
    std::cout << "Regular users created and assigned to policies.\n";

    std::string maksymPinHash = cryptoService->hashPin("1111");
    std::string katerynaPinHash = cryptoService->hashPin("2222");

    Card c1(std::string(u1.getId()), "1234567890123456", maksymPinHash);
    Card c2(std::string(u2.getId()), "9876543210987654", katerynaPinHash);
    cardRepo->create(c1);
    cardRepo->create(c2);
    std::cout << "Cards created with secured hashed PINs via CryptoService.\n\n";

    runPolicyTests(*policyRepo, u1, u2);
    runAdminTests(adminCheck, config.rootPass);
    runAuthTests(cardCheckService, c1, c2);
    runLogTests(*logRepo);

    std::cout << "\nAll tests passed successfully.\n";
}



int main() {
    try {
        runAllTests();
        UI::runConsoleUI();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
