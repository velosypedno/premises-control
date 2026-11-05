#pragma once

#include <vector>
#include <optional>
#include <string>
#include <memory>
#include <sqlite3.h>
#include "Card.h"

class CardRepo {
private:
    std::shared_ptr<sqlite3> db;
    static Card extractCard(sqlite3_stmt* stmt);

public:
    explicit CardRepo(std::shared_ptr<sqlite3> dbConnection);

    void create(const Card& card);
    std::optional<Card> getByCardId(const std::string& cardId);
    std::vector<Card> listByUserId(const std::string& userId);
    void update(const Card& card);
    void remove(const std::string& id);
    std::vector<Card> list();
};