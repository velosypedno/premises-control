#include <future>


struct Config {
    std::string dbPath;
    std::string schemaPath;
    std::string salt;

    static Config loadFromEnv();
};
