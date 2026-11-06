#pragma once

#include <string>

class CryptoService {
private:
    std::string globalSalt;

public:
    explicit CryptoService(std::string globalSalt);

    std::string hashPin(const std::string& pin) const;

    const std::string& getSalt() const noexcept;
};