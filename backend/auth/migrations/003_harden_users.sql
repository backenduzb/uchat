ALTER TABLE users
    ALTER COLUMN email SET NOT NULL;

ALTER TABLE users
    ALTER COLUMN username SET NOT NULL;

ALTER TABLE users
    ADD COLUMN IF NOT EXISTS gender TEXT;

ALTER TABLE users
    ADD COLUMN IF NOT EXISTS date_of_birth DATE;

DROP INDEX IF EXISTS user_username_unique;

CREATE UNIQUE INDEX IF NOT EXISTS users_username_unique
    ON users (LOWER(username));

CREATE UNIQUE INDEX IF NOT EXISTS users_email_unique
    ON users (LOWER(email));

ALTER TABLE users
    ADD CONSTRAINT users_gender_check
    CHECK (gender IS NULL OR gender IN ('male', 'female', 'other'));