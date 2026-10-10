package conversations

import (
	"encoding/json"
	"net/http"
	"time"

	authmiddleware "chat/auth/internal/auth"
	db "chat/auth/internal/database/sqlc"

	"github.com/google/uuid"	
	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgtype"
)

type conversationDetailsResponse struct {
	ID         uuid.UUID  `json:"id"`
	Type       string     `json:"type"`
	Title      *string    `json:"title"`
	CreatedBy  uuid.UUID  `json:"created_by"`
	CreatedAt  time.Time  `json:"created_at"`
	UpdatedAt  time.Time  `json:"updated_at"`
	Role       string     `json:"role"`
	JoinedAt   time.Time  `json:"joined_at"`
	LastReadAt *time.Time `json:"last_read_at"`
}

func (h *Handler) Get(w http.ResponseWriter, r *http.Request) {
	userID, ok := authmiddleware.UserIDFromContext(r.Context())
	if !ok {
		http.Error(w, `{"error":"unauthorized"}`, http.StatusUnauthorized)
		return
	}

	conversationID, err := uuid.Parse(r.PathValue("id"))
	if err != nil {
		http.Error(w, `{"error":"invalid conversation id"}`, http.StatusBadRequest)
		return
	}

	row, err := db.New(h.pool).GetConversationForMember(
		r.Context(),
		db.GetConversationForMemberParams{
			ConversationID: pgtype.UUID{
				Bytes: conversationID,
				Valid: true,
			},
			UserID: pgtype.UUID{
				Bytes: userID,
				Valid: true,
			},
		},
	)
	if err != nil {
		if err == pgx.ErrNoRows {
			http.Error(w, `{"error":"conversation not found"}`, http.StatusNotFound)
			return
		}

		http.Error(w, `{"error":"could not get conversation"}`, http.StatusInternalServerError)
		return
	}

	response := conversationDetailsResponse{
		ID:        uuid.UUID(row.ID.Bytes),
		Type:      row.Type,
		CreatedBy: uuid.UUID(row.CreatedBy.Bytes),
		CreatedAt: row.CreatedAt.Time,
		UpdatedAt: row.UpdatedAt.Time,
		Role:      row.Role,
		JoinedAt:  row.JoinedAt.Time,
	}

	if row.Title.Valid {
		title := row.Title.String
		response.Title = &title
	}

	if row.LastReadAt.Valid {
		lastReadAt := row.LastReadAt.Time
		response.LastReadAt = &lastReadAt
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	_ = json.NewEncoder(w).Encode(response)
}
