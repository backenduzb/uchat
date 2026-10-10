ALTER TABLE users
    ADD COLUMN IF NOT EXISTS username TEXT,
    ADD COLUMN IF NOT EXISTS date_of_birth DATE;

CREATE UNIQUE INDEX IF NOT EXISTS user_username_unique
    ON users(LOWER(username));