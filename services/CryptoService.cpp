#include "CryptoService.h"
#include <iomanip>
#include <sstream>
#include <openssl/sha.h>

CryptoService::CryptoService(std::string globalSalt) 
    : globalSalt(std::move(globalSalt)) {}

std::string CryptoService::hashPin(const std::string& pin) const {
    std::string combined = pin + globalSalt;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(combined.c_str()), combined.size(), hash);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

const std::string& CryptoService::getSalt() const noexcept {
    return globalSalt;
}