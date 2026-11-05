#pragma once

#include <vector>
#include <optional>
#include <string>
#include <memory>
#include <sqlite3.h>
#include "User.h"

class UserRepo {
private:
    std::shared_ptr<sqlite3> db;

public:
    explicit UserRepo(std::shared_ptr<sqlite3> dbConnection);

    void create(const User& user);
    std::optional<User> getById(const std::string& id);
    void remove(const std::string& id);
    std::vector<User> list();
};