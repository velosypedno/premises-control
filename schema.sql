CREATE TABLE IF NOT EXISTS users (
     id TEXT PRIMARY KEY,
     first_name TEXT NOT NULL,
     last_name TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS cards (
    id TEXT PRIMARY KEY,
    user_id TEXT NOT NULL,
    card_id TEXT NOT NULL UNIQUE CHECK(length(card_id) = 16),
    pin_hash TEXT NOT NULL,
    status TEXT NOT NULL CHECK(status IN ('ACTIVE', 'BLOCKED', 'REVOKED')) DEFAULT 'ACTIVE',
    failed_attempts INTEGER NOT NULL DEFAULT 0,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);