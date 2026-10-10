package conversation

import (
	"context"
	"net/http"
	"strings"

	authmiddleware "chat/auth/internal/auth"
	db "chat/auth/internal/database/sqlc"
	"chat/auth/models"
	"chat/auth/schemas"
	"chat/auth/services/decode"

	"github.com/google/uuid"
	"github.com/jackc/pgx/v5/pgtype"
)

func (h *Handler) Create(w http.ResponseWriter, r *http.Request) {
	userID, ok := authmiddleware.UserIDFromContext(r.Context())
	if !ok {
		http.Error(w, `{"error":"unauthorized"}`, http.StatusUnauthorized)
		return
	}

	input, ok := decode.DecodeJSON[schemas.CreateConversationRequest](w, r)
	if !ok {
		return
	}

	conversationType := strings.TrimSpace(input.Type)
	if conversationType != "direct" && conversationType != "group" {
		http.Error(w, `{"error":"type must be direct or group"}`, http.StatusBadRequest)
		return
	}

	var title string
	if input.Title != nil {
		title = strings.TrimSpace(*input.Title)
	}

	if conversationType == "direct" && title != "" {
		http.Error(w, `{"error":"direct conversations cannot have a title"}`, http.StatusBadRequest)
		return
	}

	if len(title) > 200 {
		http.Error(w, `{"error":"title must be at most 200 bytes"}`, http.StatusBadRequest)
		return
	}

	conversationID := uuid.New()

	tx, err := h.pool.Begin(r.Context())
	if err != nil {
		http.Error(w, `{"error":"could not create conversation"}`, http.StatusInternalServerError)
		return
	}
	defer func() {
		_ = tx.Rollback(context.Background())
	}()

	queries := db.New(tx)

	row, err := queries.CreateConversation(r.Context(), db.CreateConversationParams{
		ID:        pgUUID(conversationID),
		Type:      conversationType,
		Title:     pgtype.Text{String: title, Valid: title != ""},
		CreatedBy: pgUUID(userID),
	})
	if err != nil {
		http.Error(w, `{"error":"could not create conversation"}`, http.StatusInternalServerError)
		return
	}

	_, err = queries.AddConversationMember(r.Context(), db.AddConversationMemberParams{
		ConversationID: pgUUID(conversationID),
		UserID:         pgUUID(userID),
		Role:           "owner",
	})
	if err != nil {
		http.Error(w, `{"error":"could not add conversation owner"}`, http.StatusInternalServerError)
		return
	}

	if err := tx.Commit(r.Context()); err != nil {
		http.Error(w, `{"error":"could not save conversation"}`, http.StatusInternalServerError)
		return
	}

	conversation := models.Conversation{
		ID:        conversationID,
		Type:      row.Type,
		Title:     textToPtr(row.Title),
		CreatedBy: userID,
		CreatedAt: row.CreatedAt.Time,
		UpdatedAt: row.UpdatedAt.Time,
	}

	writeJSON(w, http.StatusCreated, schemas.NewConversationResponse(conversation))
}
