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
#include "services/AdminCheck.h"
#include "services/CardCheck.h"
#include "services/UserService.h"
#include "services/CardService.h"
#include "ui/ConsoleUI.h"

namespace {

void seedDefaultData(
    sqlite3* rawDb,
    const std::shared_ptr<UserRepo>& userRepo,
    const std::shared_ptr<CardRepo>& cardRepo,
    const std::shared_ptr<AccessPolicyRepo>& policyRepo,
    const std::shared_ptr<CryptoService>& cryptoService,
    const std::string& rootPassHash
) {
    Database::initializeRootAdmin(rawDb, rootPassHash);

    AccessPolicy workHoursPolicy("Standard Working Hours");
    for (DayOfWeek day : {
             DayOfWeek::Monday,
             DayOfWeek::Tuesday,
             DayOfWeek::Wednesday,
             DayOfWeek::Thursday,
             DayOfWeek::Friday}) {
        workHoursPolicy.addWindow(
            AccessWindow(std::string(workHoursPolicy.getId()), day, "09:00", "18:00"));
    }
    policyRepo->create(workHoursPolicy);

    auto rootOpt = userRepo->getByUsername("root");
    if (rootOpt.has_value()) {
        User root = *rootOpt;
        root.setPolicyId(std::string(workHoursPolicy.getId()));
        userRepo->update(root);

        std::string adminPinHash = cryptoService->hashPin("0000");
        Card adminCard(std::string(root.getId()), "0000000000000001", adminPinHash);
        cardRepo->create(adminCard);
    }

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

    std::cout << "Database seeded with default data.\n";
    std::cout << "  Admin card: 0000000000000001 / PIN 0000\n";
    std::cout << "  User card:  1234567890123456 / PIN 1111 (Mon-Fri 09:00-18:00)\n";
    std::cout << "  User card:  9876543210987654 / PIN 2222 (no policy)\n";
}

}

int main() {
    try {
        Config config = Config::loadFromEnv();
        std::cout << "Using database: " << config.dbPath << '\n';

        auto db = Database::openConnection(config.dbPath);

        Database::initializeSchema(db.get(), config.schemaPath);
        Database::ensureMigrations(db.get());

        auto cryptoService = std::make_shared<CryptoService>(config.salt);

        auto userRepo = std::make_shared<UserRepo>(db);
        auto cardRepo = std::make_shared<CardRepo>(db);
        auto policyRepo = std::make_shared<AccessPolicyRepo>(db);
        auto logRepo = std::make_shared<AccessLogRecordRepo>(db);

        if (Database::isEmpty(db.get())) {
            std::string rootPassHash = cryptoService->hashPin(config.rootPass);
            seedDefaultData(db.get(), userRepo, cardRepo, policyRepo, cryptoService, rootPassHash);
        } else {
            std::string rootPassHash = cryptoService->hashPin(config.rootPass);
            Database::initializeRootAdmin(db.get(), rootPassHash);
            std::cout << "Existing database loaded (state preserved).\n";
        }

        auto adminCheck = std::make_shared<AdminCheck>(userRepo, cardRepo, cryptoService);
        auto cardCheck = std::make_shared<CardCheck>(cardRepo, userRepo, policyRepo, logRepo, cryptoService);
        auto userService = std::make_shared<UserService>(userRepo, policyRepo, cryptoService);
        auto cardService = std::make_shared<CardService>(cardRepo, userRepo, logRepo, cryptoService);

        UI::runConsoleUI({
            .userService = userService,
            .cardService = cardService,
            .adminCheck = adminCheck,
            .cardCheck = cardCheck,
            .userRepo = userRepo,
            .cardRepo = cardRepo,
            .policyRepo = policyRepo,
            .logRepo = logRepo
        });

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
