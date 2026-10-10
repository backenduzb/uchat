package schemas

import (
	"chat/auth/models"
	"time"

	"github.com/google/uuid"
)

type ConversationResponse struct {
	ID        uuid.UUID `json:"id"`
	Type      string    `json:"type"`
	Title     *string   `json:"title,omitempty"`
	CreatedBy uuid.UUID `json:"created_by"`
	CreatedAt time.Time `json:"created_at"`
	UpdatedAt time.Time `json:"updated_at"`
}

type ConversationDetailResponse struct {
	ID         uuid.UUID  `json:"id"`
	Type       string     `json:"type"`
	Title      *string    `json:"title,omitempty"`
	CreatedBy  uuid.UUID  `json:"created_by"`
	CreatedAt  time.Time  `json:"created_at"`
	UpdatedAt  time.Time  `json:"updated_at"`
	Role       string     `json:"role"`
	JoinedAt   time.Time  `json:"joined_at"`
	LastReadAt *time.Time `json:"last_read_at,omitempty"`
}

type ConversationListItemResponse struct {
	ID        uuid.UUID `json:"id"`
	Type      string    `json:"type"`
	Title     *string   `json:"title,omitempty"`
	CreatedAt time.Time `json:"created_at"`
	UpdatedAt time.Time `json:"updated_at"`
	Role      string    `json:"role"`
}

type ConversationListResponse struct {
	Items []ConversationListItemResponse `json:"items"`
	Page  int                            `json:"page"`
	Limit int                            `json:"limit"`
}

type ConversationMemberResponse struct {
	UserID   uuid.UUID `json:"user_id"`
	Username string    `json:"username"`
	Role     string    `json:"role"`
	JoinedAt time.Time `json:"joined_at"`
}

type ConversationMemberAddResponse struct {
	UserID   uuid.UUID `json:"user_id"`
	Role     string    `json:"role"`
	JoinedAt time.Time `json:"joined_at"`
}

func NewConversationResponse(conversation models.Conversation) ConversationResponse {
	return ConversationResponse{
		ID:        conversation.ID,
		Type:      conversation.Type,
		Title:     conversation.Title,
		CreatedBy: conversation.CreatedBy,
		CreatedAt: conversation.CreatedAt,
		UpdatedAt: conversation.UpdatedAt,
	}
}

func NewConversationDetailResponse(detail models.ConversationDetail) ConversationDetailResponse {
	return ConversationDetailResponse{
		ID:         detail.ID,
		Type:       detail.Type,
		Title:      detail.Title,
		CreatedBy:  detail.CreatedBy,
		CreatedAt:  detail.CreatedAt,
		UpdatedAt:  detail.UpdatedAt,
		Role:       detail.Role,
		JoinedAt:   detail.JoinedAt,
		LastReadAt: detail.LastReadAt,
	}
}

func NewConversationListItemResponse(summary models.ConversationSummary) ConversationListItemResponse {
	return ConversationListItemResponse{
		ID:        summary.ID,
		Type:      summary.Type,
		Title:     summary.Title,
		CreatedAt: summary.CreatedAt,
		UpdatedAt: summary.UpdatedAt,
		Role:      summary.Role,
	}
}

func NewConversationListResponse(summaries []models.ConversationSummary, page int, limit int) ConversationListResponse {
	items := make([]ConversationListItemResponse, 0, len(summaries))
	for _, summary := range summaries {
		items = append(items, NewConversationListItemResponse(summary))
	}
	return ConversationListResponse{
		Items: items,
		Page:  page,
		Limit: limit,
	}
}

func NewConversationMemberResponse(member models.ConversationMember) ConversationMemberResponse {
	return ConversationMemberResponse{
		UserID:   member.UserID,
		Username: member.Username,
		Role:     member.Role,
		JoinedAt: member.JoinedAt,
	}
}

func NewConversationMemberListResponse(members []models.ConversationMember) []ConversationMemberResponse {
	responses := make([]ConversationMemberResponse, 0, len(members))
	for _, member := range members {
		responses = append(responses, NewConversationMemberResponse(member))
	}
	return responses
}

func NewConversationMemberAddResponse(member models.ConversationMember) ConversationMemberAddResponse {
	return ConversationMemberAddResponse{
		UserID:   member.UserID,
		Role:     member.Role,
		JoinedAt: member.JoinedAt,
	}
}
