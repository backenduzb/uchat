package conversations

import (
	"context"
	"encoding/json"
	"net/http"
	"strings"
	"time"

	authmiddleware "chat/auth/internal/auth"
	db "chat/auth/internal/database/sqlc"

	"github.com/google/uuid"
	"github.com/jackc/pgx/v5/pgtype"
	"github.com/jackc/pgx/v5/pgxpool"
)

type Handler struct {
	pool *pgxpool.Pool
}

func NewHandler(pool *pgxpool.Pool) *Handler {
	return &Handler{pool: pool}
}

type createConversationRequest struct {
	Type  string  `json:"type"`
	Title *string `json:"title"`
}

type conversationResponse struct {
	ID        uuid.UUID `json:"id"`
	Type      string    `json:"type"`
	Title     *string   `json:"title"`
	CreatedAt time.Time `json:"created_at"`
}

func (h *Handler) Create(w http.ResponseWriter, r *http.Request) {
	userID, ok := authmiddleware.UserIDFromContext(r.Context())
	if !ok {
		http.Error(w, `{"error":"unauthorized"}`, http.StatusUnauthorized)
		return
	}

	r.Body = http.MaxBytesReader(w, r.Body, 4096)

	var input createConversationRequest
	decoder := json.NewDecoder(r.Body)
	decoder.DisallowUnknownFields()

	if err := decoder.Decode(&input); err != nil {
		http.Error(w, `{"error":"invalid JSON body"}`, http.StatusBadRequest)
		return
	}

	input.Type = strings.TrimSpace(input.Type)
	if input.Type != "direct" && input.Type != "group" {
		http.Error(w, `{"error":"type must be direct or group"}`, http.StatusBadRequest)
		return
	}

	var title string
	if input.Title != nil {
		title = strings.TrimSpace(*input.Title)
	}

	if input.Type == "direct" && title != "" {
		http.Error(w, `{"error":"direct conversations cannot have a title"}`, http.StatusBadRequest)
		return
	}

	if len(title) > 200 {
		http.Error(w, `{"error":"title must be at most 200 bytes"}`, http.StatusBadRequest)
		return
	}

	if title == "" {
		title = ""
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

	conversation, err := queries.CreateConversation(r.Context(), db.CreateConversationParams{
		ID:        pgtype.UUID{Bytes: [16]byte(conversationID), Valid: true},
		Type:      input.Type,
		Title:     pgtype.Text{String: title, Valid: title != ""},
		CreatedBy: pgtype.UUID{Bytes: [16]byte(userID), Valid: true},
	})
	if err != nil {
		http.Error(w, `{"error":"could not create conversation"}`, http.StatusInternalServerError)
		return
	}

	_, err = queries.AddConversationMember(r.Context(), db.AddConversationMemberParams{
		ConversationID: pgtype.UUID{Bytes: [16]byte(conversationID), Valid: true},
		UserID:         pgtype.UUID{Bytes: [16]byte(userID), Valid: true},
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

	var responseTitle *string
	if conversation.Title.Valid {
		responseTitle = &conversation.Title.String
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusCreated)
	_ = json.NewEncoder(w).Encode(conversationResponse{
		ID:        conversationID,
		Type:      conversation.Type,
		Title:     responseTitle,
		CreatedAt: conversation.CreatedAt.Time,
	})
}
