#pragma once

#include <string>

struct Config {
    std::string dbPath;
    std::string schemaPath;
    std::string salt;
    std::string rootPass;

    static Config loadFromEnv();
};
