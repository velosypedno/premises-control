#include <iostream>
#include <memory>
#include "infra/Database.h"
#include "infra/Config.h"
#include "models/User.h"
#include "models/Card.h"
#include "models/AccessPolicy.h"
#include "models/UserRepo.h"
#include "models/CardRepo.h"
#include "models/AccessPolicyRepo.h"
#include "models/AccessLogRecordRepo.h"
#include "services/CryptoService.h"
#include "ui/ConsoleUI.h"

int main() {
    try {
        Config config = Config::loadFromEnv();
        std::cout << "Using database: " << config.dbPath << '\n';

        auto db = Database::openConnection(config.dbPath);

        Database::initializeSchema(db.get(), config.schemaPath);
        Database::clear(db.get());

        auto cryptoService = std::make_shared<CryptoService>(config.salt);

        std::string rootPassHash = cryptoService->hashPin(config.rootPass);
        Database::initializeRootAdmin(db.get(), rootPassHash);

        auto userRepo = std::make_shared<UserRepo>(db);
        auto cardRepo = std::make_shared<CardRepo>(db);
        auto policyRepo = std::make_shared<AccessPolicyRepo>(db);
        auto logRepo = std::make_shared<AccessLogRecordRepo>(db);

        AccessPolicy workHoursPolicy("Standard Working Hours");
        workHoursPolicy.addWindow(AccessWindow(std::string(workHoursPolicy.getId()), DayOfWeek::Monday, "09:00", "18:00"));
        workHoursPolicy.addWindow(AccessWindow(std::string(workHoursPolicy.getId()), DayOfWeek::Friday, "09:00", "17:00"));
        policyRepo->create(workHoursPolicy);

        User u1("Maksym", "Shevchenko");
        userRepo->create(u1);
        u1.setPolicyId(std::string(workHoursPolicy.getId()));
        userRepo->update(u1);

        User u2("Kateryna", "Bondarenko");
        userRepo->create(u2);

        std::string maksymPinHash = cryptoService->hashPin("1111");
        std::string katerynaPinHash = cryptoService->hashPin("2222");

        Card c1(std::string(u1.getId()), "1234567890123456", maksymPinHash);
        Card c2(std::string(u2.getId()), "9876543210987654", katerynaPinHash);
        cardRepo->create(c1);
        cardRepo->create(c2);

        std::cout << "Database successfully initialized with default seed data.\n";

        UI::runConsoleUI(userRepo, cardRepo, policyRepo, logRepo, cryptoService);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
