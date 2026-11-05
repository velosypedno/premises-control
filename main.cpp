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
#include "UserRepo.h"

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

        UserRepo userRepo(db);

        User u1("Максим", "Шевченко");
        User u2("Катерина", "Бондаренко");

        userRepo.create(u1);
        userRepo.create(u2);
        std::cout << "Користувачів успішно збережено в базу через UserRepo.\n";

        std::cout << "\nСписок користувачів з бази даних:\n";
        for (const auto& user : userRepo.list()) {
            std::cout << "- ID: " << user.getId() << " | Ім'я: " << user.getFullName() << '\n';
        }

    } catch (const std::exception& e) {
        std::cerr << "Помилка: " << e.what() << '\n';
        return 1;
    }

    return 0;
}