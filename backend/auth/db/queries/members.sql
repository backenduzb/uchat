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

-- name: ListConversationMembers :many
SELECT
    cm.user_id,
    u.username,
    cm.role,
    cm.joined_at
FROM conversation_members AS cm
JOIN users AS u
    ON u.id = cm.user_id
WHERE cm.conversation_id = sqlc.arg(conversation_id)
  AND EXISTS (
      SELECT 1
      FROM conversation_members AS requester
      WHERE requester.conversation_id = cm.conversation_id
        AND requester.user_id = sqlc.arg(user_id)
  )
ORDER BY cm.joined_at ASC, cm.user_id ASC;

-- name: GetConversationRole :one
SELECT role
FROM conversation_members
WHERE conversation_id = sqlc.arg(conversation_id)
  AND user_id = sqlc.arg(user_id);

-- name: AddMemberToConversation :one
INSERT INTO conversation_members (
    conversation_id,
    user_id,
    role
)
SELECT
    c.id,
    sqlc.arg(new_user_id),
    'member'
FROM conversations AS c
WHERE c.id = sqlc.arg(conversation_id)
  AND c.type = 'group'
RETURNING *;

-- name: LockConversationForMemberRemoval :one
SELECT id, type
FROM conversations
WHERE id = sqlc.arg(conversation_id)
FOR UPDATE;

-- name: CountConversationOwners :one
SELECT COUNT(*)::int
FROM conversation_members
WHERE conversation_id = sqlc.arg(conversation_id)
  AND role = 'owner';

-- name: RemoveConversationMember :exec
DELETE FROM conversation_members
WHERE conversation_id = sqlc.arg(conversation_id)
  AND user_id = sqlc.arg(user_id);
