CREATE TABLE IF NOT EXISTS cards (
    card_token VARCHAR(128) PRIMARY KEY,
    network VARCHAR(30) NOT NULL,
    expiry_month SMALLINT NOT NULL,
    expiry_year SMALLINT NOT NULL,
    status VARCHAR(20) NOT NULL DEFAULT 'ACTIVE',
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS transactions (
    transaction_id VARCHAR(64) PRIMARY KEY,
    card_token VARCHAR(128) NOT NULL REFERENCES cards(card_token),
    network VARCHAR(30) NOT NULL,
    amount NUMERIC(18,2) NOT NULL CHECK (amount > 0),
    international BOOLEAN NOT NULL DEFAULT FALSE,
    risk_score INTEGER NOT NULL CHECK (risk_score >= 0),
    risk_level VARCHAR(10) NOT NULL,
    decision VARCHAR(20) NOT NULL,
    reason TEXT NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_transactions_card_token
ON transactions(card_token);

CREATE INDEX IF NOT EXISTS idx_transactions_created_at
ON transactions(created_at);
