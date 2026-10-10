
CREATE TABLE messages (
    id UUID PRIMARY KEY,
    conversation_id UUID NOT NULL
        REFERENCES conversations(id) ON DELETE CASCADE,
    sender_id UUID NOT NULL
        REFERENCES users(id) ON DELETE RESTRICT,
    client_message_id UUID NOT NULL,
    content TEXT NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    edited_at TIMESTAMPTZ,
    deleted_at TIMESTAMPTZ,

    CONSTRAINT messages_content_check
        CHECK (char_length(content) BETWEEN 1 AND 10000),

    CONSTRAINT messages_client_id_unique
        UNIQUE (sender_id, client_message_id)
);

CREATE INDEX messages_history_idx
    ON messages (conversation_id, created_at DESC, id DESC);
