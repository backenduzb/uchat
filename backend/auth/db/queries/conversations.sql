-- name: CreateConversation :one
INSERT INTO conversations (
    id,
    type,
    title,
    created_by
)
VALUES (
    sqlc.arg(id),
    sqlc.arg(type),
    sqlc.narg(title),
    sqlc.arg(created_by)
)
RETURNING *;

-- name: GetConversationForMember :one
SELECT
    c.id,
    c.type,
    c.title,
    c.created_by,
    c.created_at,
    c.updated_at,
    cm.role,
    cm.joined_at,
    cm.last_read_at
FROM conversations AS c
JOIN conversation_members AS cm
    ON cm.conversation_id = c.id
WHERE c.id = sqlc.arg(conversation_id)
  AND cm.user_id = sqlc.arg(user_id);

-- name: ListUserConversations :many
SELECT
    c.id,
    c.type,
    c.title,
    c.created_by,
    c.created_at,
    c.updated_at,
    cm.role,
    cm.joined_at,
    cm.last_read_at
FROM conversations AS c
JOIN conversation_members AS cm
    ON cm.conversation_id = c.id
WHERE cm.user_id = sqlc.arg(user_id)
ORDER BY c.updated_at DESC, c.id DESC
LIMIT sqlc.arg(page_size)::int
OFFSET sqlc.arg(page_offset)::int;

-- name: AddConversationMember :one
INSERT INTO conversation_members (
    conversation_id,
    user_id,
    role
)
VALUES (
    sqlc.arg(conversation_id),
    sqlc.arg(user_id),
    sqlc.arg(role)
)
RETURNING *;
