
#include "Config.h"
#include <cstdlib>

Config Config::loadFromEnv() {
    return Config{
        std::getenv("DB_PATH") ? std::getenv("DB_PATH") : "app.db",
        std::getenv("SCHEMA_PATH") ? std::getenv("SCHEMA_PATH") : "schema.sql",
        std::getenv("SALT") ? std::getenv("SALT") : "",
        std::getenv("ROOT_PASS") ? std::getenv("ROOT_PASS") : ""
    };
}
