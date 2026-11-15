#include <iostream>
#include <filesystem>
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
#include "services/CardCheck.h"
#include "services/AdminCheck.h"

namespace fs = std::filesystem;

static int fails = 0;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        std::cerr << "FAIL: " << msg << '\n'; \
        ++fails; \
    } else { \
        std::cout << "OK:   " << msg << '\n'; \
    } \
} while (0)

int main() {
    try {
    const std::string dbPath = "verify_plan.db";
    const std::string schemaPath = "schema.sql";
    if (!fs::exists(schemaPath)) {
        std::cerr << "schema.sql not found in " << fs::current_path() << '\n';
        return 2;
    }
    fs::remove(dbPath);

    auto db = Database::openConnection(dbPath);
    Database::initializeSchema(db.get(), schemaPath);
    Database::ensureMigrations(db.get());

    auto crypto = std::make_shared<CryptoService>("test-salt");
    auto userRepo = std::make_shared<UserRepo>(db);
    auto cardRepo = std::make_shared<CardRepo>(db);
    auto policyRepo = std::make_shared<AccessPolicyRepo>(db);
    auto logRepo = std::make_shared<AccessLogRecordRepo>(db);

    AccessPolicy policy("Work");
    for (DayOfWeek d : {DayOfWeek::Monday, DayOfWeek::Tuesday, DayOfWeek::Wednesday,
                        DayOfWeek::Thursday, DayOfWeek::Friday}) {
        policy.addWindow(AccessWindow(std::string(policy.getId()), d, "09:00", "18:00"));
    }
    policyRepo->create(policy);

    User admin("Root", "Admin", std::string(policy.getId()), UserRole::Admin, "root", crypto->hashPin("admin"));
    userRepo->create(admin);
    Card adminCard(std::string(admin.getId()), "0000000000000001", crypto->hashPin("0000"));
    cardRepo->create(adminCard);

    User user("Maksym", "Shevchenko", std::string(policy.getId()), UserRole::User);
    userRepo->create(user);
    Card card(std::string(user.getId()), "1234567890123456", crypto->hashPin("1111"));
    cardRepo->create(card);

    CardCheck check(cardRepo, userRepo, policyRepo, logRepo, crypto, 3);
    AdminCheck adminCheck(userRepo, cardRepo, crypto);

    CHECK(check.authenticate("1234567890123456", "1111", DayOfWeek::Monday, "12:00"),
          "1. Successful access");
    CHECK(check.getLastEventType() == LogEventType::Allowed, "1. Allowed event");

    CHECK(!check.authenticate("9999999999999999", "0000", DayOfWeek::Monday, "12:00"),
          "2. Unknown card denied");
    CHECK(check.getLastEventType() == LogEventType::CardDoesNotExist, "2. Unknown card event");

    {
        auto c = cardRepo->getByCardId("1234567890123456").value();
        c.resetFailedAttempts();
        c.setStatus(CardStatus::Active);
        cardRepo->update(c);
    }
    CHECK(!check.authenticate("1234567890123456", "9999", DayOfWeek::Monday, "12:00"),
          "3. Wrong PIN denied");
    CHECK(check.getLastEventType() == LogEventType::WrongPin, "3. WrongPin event");
    CHECK(cardRepo->getByCardId("1234567890123456")->getFailedAttempts() == 1, "3. fail counter = 1");

    CHECK(check.authenticate("1234567890123456", "1111", DayOfWeek::Monday, "12:00"),
          "5. Correct PIN after fails");
    CHECK(cardRepo->getByCardId("1234567890123456")->getFailedAttempts() == 0, "5. counter reset to 0");

    {
        auto c = cardRepo->getByCardId("1234567890123456").value();
        c.resetFailedAttempts();
        c.setStatus(CardStatus::Active);
        cardRepo->update(c);
    }
    CHECK(!check.authenticate("1234567890123456", "0001", DayOfWeek::Monday, "12:00"), "4a. fail 1");
    CHECK(!check.authenticate("1234567890123456", "0002", DayOfWeek::Monday, "12:00"), "4b. fail 2");
    CHECK(!check.authenticate("1234567890123456", "0003", DayOfWeek::Monday, "12:00"), "4c. fail 3");
    CHECK(check.getLastEventType() == LogEventType::Alarm, "4. Alarm on third fail");
    CHECK(cardRepo->getByCardId("1234567890123456")->getStatus() == CardStatus::Blocked, "4. card BLOCKED");
    CHECK(!check.authenticate("1234567890123456", "1111", DayOfWeek::Monday, "12:00"),
          "4. correct PIN does not unlock blocked");
    CHECK(check.getLastEventType() == LogEventType::Blocked, "4. still Blocked event");

    {
        auto c = cardRepo->getByCardId("1234567890123456").value();
        c.setStatus(CardStatus::Active);
        c.resetFailedAttempts();
        cardRepo->update(c);
    }
    CHECK(check.authenticate("1234567890123456", "1111", DayOfWeek::Monday, "12:00"),
          "10. access after unlock");

    CHECK(!check.authenticate("1234567890123456", "1111", DayOfWeek::Monday, "08:59"), "6. 08:59 denied");
    CHECK(check.authenticate("1234567890123456", "1111", DayOfWeek::Monday, "09:00"), "6. 09:00 allowed");
    CHECK(check.authenticate("1234567890123456", "1111", DayOfWeek::Monday, "17:59"), "6. 17:59 allowed");
    CHECK(!check.authenticate("1234567890123456", "1111", DayOfWeek::Monday, "18:00"), "6. 18:00 denied");
    CHECK(!check.authenticate("1234567890123456", "1111", DayOfWeek::Saturday, "12:00"), "6. Saturday denied");

    {
        auto c = cardRepo->getByCardId("1234567890123456").value();
        c.resetFailedAttempts();
        c.setStatus(CardStatus::Active);
        cardRepo->update(c);
    }
    check.authenticate("1234567890123456", "9999", DayOfWeek::Monday, "12:00");
    int before = cardRepo->getByCardId("1234567890123456")->getFailedAttempts();
    check.logCancellation("1234567890123456");
    CHECK(check.getLastEventType() == LogEventType::Cancelled, "7. cancel logged");
    CHECK(cardRepo->getByCardId("1234567890123456")->getFailedAttempts() == before,
          "7. cancel does not change counter");

    {
        auto c = cardRepo->getByCardId("1234567890123456").value();
        c.setStatus(CardStatus::Revoked);
        cardRepo->update(c);
    }
    CHECK(!check.authenticate("1234567890123456", "1111", DayOfWeek::Monday, "12:00"),
          "9. revoked card denied");
    CHECK(check.getLastEventType() == LogEventType::RevokedCard, "9. RevokedCard event");

    {
        auto c = cardRepo->getByCardId("1234567890123456").value();
        c.setStatus(CardStatus::Active);
        c.resetFailedAttempts();
        cardRepo->update(c);
    }

    {
        auto c = cardRepo->getByCardId("1234567890123456").value();
        c.setPinHash(crypto->hashPin("5555"));
        cardRepo->update(c);
    }
    CHECK(!check.authenticate("1234567890123456", "1111", DayOfWeek::Monday, "12:00"),
          "11. old PIN rejected");
    CHECK(check.authenticate("1234567890123456", "5555", DayOfWeek::Monday, "12:00"),
          "11. new PIN accepted");

    CHECK(adminCheck.authenticateByCard("0000000000000001", "0000").has_value(),
          "12. ADMIN card+PIN accepted");
    CHECK(!adminCheck.authenticateByCard("1234567890123456", "5555").has_value(),
          "12. USER card cannot open admin");

    {
        auto u = userRepo->getById(std::string(user.getId())).value();
        u.setActive(false);
        userRepo->update(u);
    }
    {
        auto c = cardRepo->getByCardId("1234567890123456").value();
        c.setStatus(CardStatus::Active);
        c.resetFailedAttempts();
        c.setPinHash(crypto->hashPin("5555"));
        cardRepo->update(c);
    }
    CHECK(!check.authenticate("1234567890123456", "5555", DayOfWeek::Monday, "12:00"),
          "active=false denied");
    CHECK(check.getLastEventType() == LogEventType::UserInactive, "UserInactive event");

    {
        auto u = userRepo->getById(std::string(user.getId())).value();
        u.setActive(true);
        userRepo->update(u);
        auto c = cardRepo->getByCardId("1234567890123456").value();
        c.setStatus(CardStatus::Active);
        c.resetFailedAttempts();
        c.setPinHash(crypto->hashPin("5555"));
        cardRepo->update(c);
    }
    check.authenticate("1234567890123456", "0001", DayOfWeek::Monday, "12:00");
    check.authenticate("1234567890123456", "0002", DayOfWeek::Monday, "12:00");
    CHECK(cardRepo->getByCardId("1234567890123456")->getFailedAttempts() == 2, "13a. two fails saved");

    db.reset();
    db = Database::openConnection(dbPath);
    userRepo = std::make_shared<UserRepo>(db);
    cardRepo = std::make_shared<CardRepo>(db);
    policyRepo = std::make_shared<AccessPolicyRepo>(db);
    logRepo = std::make_shared<AccessLogRecordRepo>(db);
    CardCheck check2(cardRepo, userRepo, policyRepo, logRepo, crypto, 3);
    CHECK(cardRepo->getByCardId("1234567890123456")->getFailedAttempts() == 2, "13b. counter restored");
    CHECK(!check2.authenticate("1234567890123456", "0003", DayOfWeek::Monday, "12:00"), "13c. third fail");
    CHECK(check2.getLastEventType() == LogEventType::Alarm, "13d. alarm after restart");
    CHECK(!logRepo->list().empty(), "13e. journal preserved");

    User noPolicy("No", "Policy");
    userRepo->create(noPolicy);
    Card npCard(std::string(noPolicy.getId()), "2222222222222222", crypto->hashPin("1234"));
    cardRepo->create(npCard);
    CHECK(!check2.authenticate("2222222222222222", "1234", DayOfWeek::Monday, "12:00"),
          "no policy denied");
    CHECK(check2.getLastEventType() == LogEventType::PolicyNotSet, "PolicyNotSet event");

    try {
        Card dup(std::string(user.getId()), "1234567890123456", crypto->hashPin("1"));
        cardRepo->create(dup);
        CHECK(false, "8. duplicate card should throw");
    } catch (...) {
        CHECK(true, "8. duplicate card rejected");
    }

    std::cout << "\n==== " << (fails == 0 ? "ALL PASSED" : "FAILED")
              << " (" << fails << " failures) ====\n";
    logRepo.reset();
    policyRepo.reset();
    cardRepo.reset();
    userRepo.reset();
    db.reset();
    std::error_code ec;
    fs::remove(dbPath, ec);
    return fails == 0 ? 0 : 1;
    } catch (const std::exception& e) {
        std::cerr << "EXCEPTION: " << e.what() << '\n';
        return 2;
    } catch (...) {
        std::cerr << "UNKNOWN EXCEPTION\n";
        return 2;
    }
}
