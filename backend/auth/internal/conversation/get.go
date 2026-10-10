package conversation

import (
	"net/http"

	authmiddleware "chat/auth/internal/auth"
	db "chat/auth/internal/database/sqlc"
	"chat/auth/models"
	"chat/auth/schemas"

	"github.com/google/uuid"
	"github.com/jackc/pgx/v5"
)

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
			ConversationID: pgUUID(conversationID),
			UserID:         pgUUID(userID),
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

	detail := models.ConversationDetail{
		ID:         uuidFromPg(row.ID),
		Type:       row.Type,
		Title:      textToPtr(row.Title),
		CreatedBy:  uuidFromPg(row.CreatedBy),
		CreatedAt:  row.CreatedAt.Time,
		UpdatedAt:  row.UpdatedAt.Time,
		Role:       row.Role,
		JoinedAt:   row.JoinedAt.Time,
		LastReadAt: timePtrFromTimestamptz(row.LastReadAt),
	}

	writeJSON(w, http.StatusOK, schemas.NewConversationDetailResponse(detail))
}
