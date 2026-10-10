
CREATE TABLE conversations (
    id UUID PRIMARY KEY,
    type TEXT NOT NULL
        CHECK (type IN ('direct', 'group')),
    title TEXT,
    created_by UUID NOT NULL
        REFERENCES users(id) ON DELETE RESTRICT,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),

    CONSTRAINT conversations_title_check
        CHECK (
            (type = 'direct' AND title IS NULL)
            OR
            (type = 'group')
        )
);

CREATE INDEX conversations_created_by_idx
    ON conversations (created_by);

CREATE INDEX conversations_updated_at_idx
    ON conversations (updated_at DESC);
