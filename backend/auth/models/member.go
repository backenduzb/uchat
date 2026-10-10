package models

import (
	"time"

	"github.com/google/uuid"
)

type ConversationMember struct {
	ConversationID uuid.UUID  `json:"conversation_id"`
	UserID         uuid.UUID  `json:"user_id"`
	Username       string     `json:"username"`
	Role           string     `json:"role"`
	JoinedAt       time.Time  `json:"joined_at"`
	LastReadAt     *time.Time `json:"last_read_at,omitempty"`
}
