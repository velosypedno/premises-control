#pragma once

#include <vector>
#include <optional>
#include <string>
#include <memory>
#include <sqlite3.h>
#include "AccessPolicy.h"

class AccessPolicyRepo {
private:
    std::shared_ptr<sqlite3> db;

    std::vector<AccessWindow> loadWindowsForPolicy(const std::string& policyId);

public:
    explicit AccessPolicyRepo(std::shared_ptr<sqlite3> dbConnection);

    void create(const AccessPolicy& policy);
    std::optional<AccessPolicy> getById(const std::string& id);

    std::optional<AccessPolicy> getByUserId(const std::string& userId);

    void update(const AccessPolicy& policy);

    void remove(const std::string& id);

    std::vector<AccessPolicy> list();
};