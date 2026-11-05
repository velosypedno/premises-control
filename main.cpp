#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <memory>
#include <sqlite3.h>
#include "models/User.h"
#include "models/Card.h"
#include "models/AccessPolicy.h"
#include "models/UserRepo.h"
#include "models/CardRepo.h"
#include "models/AccessPolicyRepo.h"

namespace fs = std::filesystem;

struct Config {
    std::string dbPath;
    std::string schemaPath;

    static Config loadFromEnv() {
        Config config;

        const char* envDb = std::getenv("DB_PATH");
        const char* envSchema = std::getenv("SCHEMA_PATH");

        config.dbPath = envDb ? envDb : "app.db";
        config.schemaPath = envSchema ? envSchema : "schema.sql";

        return config;
    }
};

void initializeSchema(sqlite3* db, const std::string& schemaPath) {
    if (!fs::exists(schemaPath)) {
        throw std::runtime_error("Файл схеми не знайдено за шляхом: " + schemaPath);
    }

    std::ifstream file(schemaPath);
    if (!file.is_open()) {
        throw std::runtime_error("Не вдалося відкрити файл схеми: " + schemaPath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, buffer.str().c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string error(errMsg);
        sqlite3_free(errMsg);
        throw std::runtime_error("Помилка виконання SQL схеми: " + error);
    }

    std::cout << "Схему успішно ініціалізовано з файлу: " << schemaPath << '\n';
}

void clearDatabase(sqlite3* db) {
    char* errMsg = nullptr;
    const char* clearSql = "DELETE FROM cards; DELETE FROM users;";

    if (sqlite3_exec(db, clearSql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string error(errMsg);
        sqlite3_free(errMsg);
        throw std::runtime_error("Помилка очищення таблиць: " + error);
    }

    std::cout << "Усі попередні дані з бази успішно стерто.\n\n";
}
int main() {
    try {
        Config config = Config::loadFromEnv();
        std::cout << "Використовуємо БД: " << config.dbPath << '\n';

        sqlite3* rawDb = nullptr;
        int rc = sqlite3_open(config.dbPath.c_str(), &rawDb);
        if (rc != SQLITE_OK) {
            std::string err = rawDb ? sqlite3_errmsg(rawDb) : "Unknown error";
            if (rawDb) sqlite3_close(rawDb);
            throw std::runtime_error("Не вдалося відкрити базу даних: " + err);
        }

        std::shared_ptr<sqlite3> db(rawDb, [](sqlite3* ptr) {
            if (ptr) {
                sqlite3_close(ptr);
                std::cout << "З'єднання з БД закрите.\n";
            }
        });

        initializeSchema(db.get(), config.schemaPath);
        clearDatabase(db.get());

        UserRepo userRepo(db);
        CardRepo cardRepo(db);
        AccessPolicyRepo policyRepo(db);

        AccessPolicy workHoursPolicy("Стандартні робочі години");

        workHoursPolicy.addWindow(AccessWindow(std::string(workHoursPolicy.getId()), DayOfWeek::Monday, "09:00", "18:00"));
        workHoursPolicy.addWindow(AccessWindow(std::string(workHoursPolicy.getId()), DayOfWeek::Friday, "09:00", "17:00"));

        policyRepo.create(workHoursPolicy);
        std::cout << "Політику доступу '" << workHoursPolicy.getName() << "' збережено в базу.\n";

        User u1("Максим", "Шевченко");
        userRepo.create(u1);

        u1.setPolicyId(std::string(workHoursPolicy.getId()));
        userRepo.update(u1);

        User u2("Катерина", "Бондаренко");
        userRepo.create(u2);
        std::cout << "Користувачів створено та прив'язано до політик через UserRepo.\n";

        Card c1(std::string(u1.getId()), "1234567890123456", "hashed_pin_1");
        Card c2(std::string(u2.getId()), "9876543210987654", "hashed_pin_2");
        cardRepo.create(c1);
        cardRepo.create(c2);
        std::cout << "Картки створено.\n\n";

        std::cout << "--- Перевірка пошуку політики за ID користувача (Максим) ---:\n";
        auto foundPolicy = policyRepo.getByUserId(std::string(u1.getId()));
        if (foundPolicy) {
            std::cout << "Знайдено політику: " << foundPolicy->getName() << " (ID: " << foundPolicy->getId() << ")\n";
            std::cout << "Кількість часових вікон: " << foundPolicy->getWindows().size() << '\n';

            bool allowedMon10 = foundPolicy->isAllowed(DayOfWeek::Monday, "10:00");
            bool allowedMon19 = foundPolicy->isAllowed(DayOfWeek::Monday, "19:00");
            std::cout << "Доступ у понеділок о 10:00 (очікується true): " << (allowedMon10 ? "ДОЗВОЛЕНО" : "ЗАБОРОНЕНО") << '\n';
            std::cout << "Доступ у понеділок о 19:00 (очікується false): " << (allowedMon19 ? "ДОЗВОЛЕНО" : "ЗАБОРОНЕНО") << '\n';
        }

        std::cout << "\n--- Перевірка пошуку політики для Катерини (у неї немає політики) ---:\n";
        auto noPolicy = policyRepo.getByUserId(std::string(u2.getId()));
        std::cout << "Політика знайдена: " << (noPolicy.has_value() ? "Так" : "Ні") << '\n';

        std::cout << "\n--- Список усіх політик у системі ---:\n";
        for (const auto& pol : policyRepo.list()) {
            std::cout << "Політика: " << pol.getName() << " | Вікон: " << pol.getWindows().size() << '\n';
        }

    } catch (const std::exception& e) {
        std::cerr << "Помилка: " << e.what() << '\n';
        return 1;
    }

    return 0;
}