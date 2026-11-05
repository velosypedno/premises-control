#include "CardRepo.h"
#include <stdexcept>

CardRepo::CardRepo(std::shared_ptr<sqlite3> dbConnection) : db(std::move(dbConnection)) {}

Card CardRepo::extractCard(sqlite3_stmt* stmt) {
    std::string id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    std::string userId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    std::string cardId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    std::string pinHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    std::string statusStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    int failedAttempts = sqlite3_column_int(stmt, 5);

    CardStatus status = Card::stringToStatus(statusStr);

    return Card(std::move(id), std::move(userId), std::move(cardId), 
                std::move(pinHash), status, failedAttempts);
}

void CardRepo::create(const Card& card) {
    const char* sql = "INSERT INTO cards (id, user_id, card_id, pin_hash, status, failed_attempts) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    sqlite3_bind_text(stmt, 1, card.getId().data(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, card.getUserId().data(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, card.getCardId().data(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, card.getPinHash().data(), -1, SQLITE_TRANSIENT);
    
    std::string statusStr = Card::statusToString(card.getStatus());
    sqlite3_bind_text(stmt, 5, statusStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, card.getFailedAttempts());

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    sqlite3_finalize(stmt);
}

std::optional<Card> CardRepo::getByCardId(const std::string& cardId) {
    const char* sql = "SELECT id, user_id, card_id, pin_hash, status, failed_attempts FROM cards WHERE card_id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    sqlite3_bind_text(stmt, 1, cardId.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<Card> card = std::nullopt;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        card.emplace(extractCard(stmt));
    }

    sqlite3_finalize(stmt);
    return card;
}

std::vector<Card> CardRepo::listByUserId(const std::string& userId) {
    const char* sql = "SELECT id, user_id, card_id, pin_hash, status, failed_attempts FROM cards WHERE user_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    std::vector<Card> cards;

    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        cards.emplace_back(extractCard(stmt));
    }

    sqlite3_finalize(stmt);
    return cards;
}

void CardRepo::update(const Card& card) {
    // Оновлюємо pin_hash, status та failed_attempts (весь об'єкт передається, але міняються специфічні поля)
    const char* sql = "UPDATE cards SET pin_hash = ?, status = ?, failed_attempts = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    sqlite3_bind_text(stmt, 1, card.getPinHash().data(), -1, SQLITE_TRANSIENT);
    
    std::string statusStr = Card::statusToString(card.getStatus());
    sqlite3_bind_text(stmt, 2, statusStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, card.getFailedAttempts());
    sqlite3_bind_text(stmt, 4, card.getId().data(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    sqlite3_finalize(stmt);
}

void CardRepo::remove(const std::string& id) {
    const char* sql = "DELETE FROM cards WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    sqlite3_finalize(stmt);
}

std::vector<Card> CardRepo::list() {
    const char* sql = "SELECT id, user_id, card_id, pin_hash, status, failed_attempts FROM cards;";
    sqlite3_stmt* stmt = nullptr;
    std::vector<Card> cards;

    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db.get()));
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        cards.emplace_back(extractCard(stmt));
    }

    sqlite3_finalize(stmt);
    return cards;
}