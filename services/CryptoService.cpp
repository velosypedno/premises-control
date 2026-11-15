#include "CryptoService.h"
#include "sha256.h"

CryptoService::CryptoService(std::string globalSalt)
    : globalSalt(std::move(globalSalt)) {}

std::string CryptoService::hashPin(const std::string& pin) const {
    return sha256_hex(pin + globalSalt);
}

const std::string& CryptoService::getSalt() const noexcept {
    return globalSalt;
}
