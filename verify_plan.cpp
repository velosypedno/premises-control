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
#include "services/Result.h"
#include "services/UserService.h"
#include "services/CardService.h"

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

    {
        auto res = check.authenticate("1234567890123456", "1111", DayOfWeek::Monday, "12:00");
        CHECK(res.is_ok(), "1. Successful access");
        CHECK(res.is_ok() && res.value().user.getId() == user.getId(), "1. Returned authenticated user");
    }

    {
        auto res = check.authenticate("9999999999999999", "0000", DayOfWeek::Monday, "12:00");
        CHECK(res.is_err(), "2. Unknown card denied");
        CHECK(res.is_err() && res.error() == AuthError::CardDoesNotExist, "2. Unknown card event");
    }

    {
        auto c = cardRepo->getByCardId("1234567890123456").value();
        c.resetFailedAttempts();
        c.setStatus(CardStatus::Active);
        cardRepo->update(c);
    }
    {
        auto res = check.authenticate("1234567890123456", "9999", DayOfWeek::Monday, "12:00");
        CHECK(res.is_err(), "3. Wrong PIN denied");
        CHECK(res.is_err() && res.error() == AuthError::WrongPin, "3. WrongPin event");
        CHECK(cardRepo->getByCardId("1234567890123456")->getFailedAttempts() == 1, "3. fail counter = 1");
    }

    CHECK(check.authenticate("1234567890123456", "1111", DayOfWeek::Monday, "12:00").is_ok(),
          "5. Correct PIN after fails");
    CHECK(cardRepo->getByCardId("1234567890123456")->getFailedAttempts() == 0, "5. counter reset to 0");

    {
        auto c = cardRepo->getByCardId("1234567890123456").value();
        c.resetFailedAttempts();
        c.setStatus(CardStatus::Active);
        cardRepo->update(c);
    }
    CHECK(check.authenticate("1234567890123456", "0001", DayOfWeek::Monday, "12:00").is_err(), "4a. fail 1");
    CHECK(check.authenticate("1234567890123456", "0002", DayOfWeek::Monday, "12:00").is_err(), "4b. fail 2");
    {
        auto res = check.authenticate("1234567890123456", "0003", DayOfWeek::Monday, "12:00");
        CHECK(res.is_err(), "4c. fail 3");
        CHECK(res.is_err() && res.error() == AuthError::Alarm, "4. Alarm on third fail");
    }
    CHECK(cardRepo->getByCardId("1234567890123456")->getStatus() == CardStatus::Blocked, "4. card BLOCKED");
    {
        auto res = check.authenticate("1234567890123456", "1111", DayOfWeek::Monday, "12:00");
        CHECK(res.is_err(), "4. correct PIN does not unlock blocked");
        CHECK(res.is_err() && res.error() == AuthError::Blocked, "4. still Blocked event");
    }

    {
        auto c = cardRepo->getByCardId("1234567890123456").value();
        c.setStatus(CardStatus::Active);
        c.resetFailedAttempts();
        cardRepo->update(c);
    }
    CHECK(check.authenticate("1234567890123456", "1111", DayOfWeek::Monday, "12:00").is_ok(),
          "10. access after unlock");

    CHECK(check.authenticate("1234567890123456", "1111", DayOfWeek::Monday, "08:59").is_err(), "6. 08:59 denied");
    CHECK(check.authenticate("1234567890123456", "1111", DayOfWeek::Monday, "09:00").is_ok(), "6. 09:00 allowed");
    CHECK(check.authenticate("1234567890123456", "1111", DayOfWeek::Monday, "17:59").is_ok(), "6. 17:59 allowed");
    CHECK(check.authenticate("1234567890123456", "1111", DayOfWeek::Monday, "18:00").is_err(), "6. 18:00 denied");
    CHECK(check.authenticate("1234567890123456", "1111", DayOfWeek::Saturday, "12:00").is_err(), "6. Saturday denied");

    {
        auto c = cardRepo->getByCardId("1234567890123456").value();
        c.resetFailedAttempts();
        c.setStatus(CardStatus::Active);
        cardRepo->update(c);
    }
    check.authenticate("1234567890123456", "9999", DayOfWeek::Monday, "12:00");
    int before = cardRepo->getByCardId("1234567890123456")->getFailedAttempts();
    check.logCancellation("1234567890123456");
    {
        auto logs = logRepo->list();
        bool hasCancelled = false;
        for (const auto& entry : logs) {
            if (entry.getCardId() == "1234567890123456" && entry.getEventType() == LogEventType::Cancelled) {
                hasCancelled = true;
            }
        }
        CHECK(hasCancelled, "7. cancel logged");
    }
    CHECK(cardRepo->getByCardId("1234567890123456")->getFailedAttempts() == before,
          "7. cancel does not change counter");

    {
        auto c = cardRepo->getByCardId("1234567890123456").value();
        c.setStatus(CardStatus::Revoked);
        cardRepo->update(c);
    }
    {
        auto res = check.authenticate("1234567890123456", "1111", DayOfWeek::Monday, "12:00");
        CHECK(res.is_err(), "9. revoked card denied");
        CHECK(res.is_err() && res.error() == AuthError::RevokedCard, "9. RevokedCard event");
    }

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
    CHECK(check.authenticate("1234567890123456", "1111", DayOfWeek::Monday, "12:00").is_err(),
          "11. old PIN rejected");
    CHECK(check.authenticate("1234567890123456", "5555", DayOfWeek::Monday, "12:00").is_ok(),
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
    {
        auto res = check.authenticate("1234567890123456", "5555", DayOfWeek::Monday, "12:00");
        CHECK(res.is_err(), "active=false denied");
        CHECK(res.is_err() && res.error() == AuthError::UserInactive, "UserInactive event");
    }

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
    {
        auto res = check2.authenticate("1234567890123456", "0003", DayOfWeek::Monday, "12:00");
        CHECK(res.is_err(), "13c. third fail");
        CHECK(res.is_err() && res.error() == AuthError::Alarm, "13d. alarm after restart");
    }
    CHECK(!logRepo->list().empty(), "13e. journal preserved");

    User noPolicy("No", "Policy");
    userRepo->create(noPolicy);
    Card npCard(std::string(noPolicy.getId()), "2222222222222222", crypto->hashPin("1234"));
    cardRepo->create(npCard);
    {
        auto res = check2.authenticate("2222222222222222", "1234", DayOfWeek::Monday, "12:00");
        CHECK(res.is_err(), "no policy denied");
        CHECK(res.is_err() && res.error() == AuthError::PolicyNotSet, "PolicyNotSet event");
    }

    try {
        Card dup(std::string(user.getId()), "1234567890123456", crypto->hashPin("1"));
        cardRepo->create(dup);
        CHECK(false, "8. duplicate card should throw");
    } catch (...) {
        CHECK(true, "8. duplicate card rejected");
    }

    // Test UserService
    UserService userService(userRepo, policyRepo, crypto);
    auto createUserRes = userService.createUser("Test", "User", "testuser", "secret");
    CHECK(createUserRes.is_ok(), "14a. UserService createUser success");
    auto dupUserRes = userService.createUser("Test", "User", "testuser", "secret");
    CHECK(dupUserRes.is_err() && dupUserRes.error() == Services::UserError::UsernameAlreadyTaken,
          "14b. UserService duplicate username rejected");

    std::string testUserId = std::string(createUserRes.value().getId());
    auto updateProfRes = userService.updateProfile(testUserId, "Updated", "Name", "testuser2", "newpass");
    CHECK(updateProfRes.is_ok(), "14c. UserService updateProfile success");
    CHECK(userService.getUserById(testUserId).value().getFirstName() == "Updated", "14d. Updated name persisted");

    auto roleRes = userService.assignAdminRole(testUserId);
    CHECK(roleRes.is_ok(), "14e. UserService assignAdminRole success");
    CHECK(userService.getUserById(testUserId).value().getRole() == UserRole::Admin, "14f. Admin role persisted");

    auto toggleRes = userService.toggleUserActive(testUserId);
    CHECK(toggleRes.is_ok() && !toggleRes.value(), "14g. UserService toggleUserActive to false");

    // Test CardService
    CardService cardService(cardRepo, userRepo, logRepo, crypto);
    auto addCardRes = cardService.addCard(testUserId, "1111222233334444", "4321");
    CHECK(addCardRes.is_ok(), "15a. CardService addCard success");

    auto invalidCardRes = cardService.addCard(testUserId, "123", "4321");
    CHECK(invalidCardRes.is_err() && invalidCardRes.error() == Services::CardError::InvalidCardFormat,
          "15b. CardService invalid card format rejected");

    auto invalidPinRes = cardService.addCard(testUserId, "5555666677778888", "12");
    CHECK(invalidPinRes.is_err() && invalidPinRes.error() == Services::CardError::InvalidPinFormat,
          "15c. CardService invalid PIN format rejected");

    auto blockCardRes = cardService.blockCard("1111222233334444");
    CHECK(blockCardRes.is_ok(), "15d. CardService blockCard success");
    CHECK(cardService.getCardByNumber("1111222233334444").value().getStatus() == CardStatus::Blocked,
          "15e. Card is Blocked");

    auto unblockCardRes = cardService.unblockCard("1111222233334444");
    CHECK(unblockCardRes.is_ok(), "15f. CardService unblockCard success");
    CHECK(cardService.getCardByNumber("1111222233334444").value().getStatus() == CardStatus::Active,
          "15g. Card is Active again");

    auto changePinRes = cardService.changePinByUser("1111222233334444", "4321", "4321", "9999", "9999");
    CHECK(changePinRes.is_ok(), "15h. CardService changePinByUser success");

    auto wrongOldPinRes = cardService.changePinByUser("1111222233334444", "0000", "0000", "8888", "8888");
    CHECK(wrongOldPinRes.is_err() && wrongOldPinRes.error() == Services::CardError::OldPinIncorrect,
          "15i. CardService wrong old PIN rejected");

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
