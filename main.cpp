#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <memory>
#include <sqlite3.h>
#include "User.h"
#include "Card.h"
#include "UserRepo.h"
#include "CardRepo.h"

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

        User u1("Максим", "Шевченко");
        User u2("Катерина", "Бондаренко");

        userRepo.create(u1);
        userRepo.create(u2);
        std::cout << "Користувачів успішно збережено в базу.\n";

        Card c1(std::string(u1.getId()), "1234567890123456", "hashed_pin_salt_1");
        Card c2(std::string(u2.getId()), "9876543210987654", "hashed_pin_salt_2");

        cardRepo.create(c1);
        cardRepo.create(c2);
        std::cout << "Картки успішно прив'язані та збережені в базу.\n";

        std::cout << "\n--- Список користувачів ---:\n";
        for (const auto& user : userRepo.list()) {
            std::cout << "ID: " << user.getId() << " | Ім'я: " << user.getFullName() << '\n';
        }

        std::cout << "\n--- Список карток ---:\n";
        for (const auto& card : cardRepo.list()) {
            std::cout << "Card ID: " << card.getCardId()
                      << " | Власник ID: " << card.getUserId()
                      << " | Статус: " << Card::statusToString(card.getStatus())
                      << " | Помилок: " << card.getFailedAttempts() << '\n';
        }

    } catch (const std::exception& e) {
        std::cerr << "Помилка: " << e.what() << '\n';
        return 1;
    }

    return 0;
}