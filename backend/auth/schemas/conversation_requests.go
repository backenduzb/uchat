package schemas

import (
	"github.com/google/uuid"
)

type CreateConversationRequest struct {
	Type  string  `json:"type"`
	Title *string `json:"title"`
}

type AddConversationMemberRequest struct {
	UserID uuid.UUID `json:"user_id"`
}
