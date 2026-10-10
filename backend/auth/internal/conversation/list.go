package conversations

import (
	"encoding/json"
	"net/http"
	"strconv"
	"time"

	authmiddleware "chat/auth/internal/auth"
	db "chat/auth/internal/database/sqlc"

	"github.com/google/uuid"
	"github.com/jackc/pgx/v5/pgtype"
)


type conversationListItem struct {
	ID        uuid.UUID `json:"id"`
	Type      string    `json:"type"`
	Title     *string   `json:"title"`
	CreatedAt time.Time `json:"created_at"`
	UpdatedAt time.Time `json:"updated_at"`
	Role      string    `json:"role"`
}

type listConversationsResponse struct {
	Items []conversationListItem `json:"items"`
	Page  int                    `json:"page"`
	Limit int                    `json:"limit"`
}

func (h *Handler) List(w http.ResponseWriter, r *http.Request) {
	userID, ok := authmiddleware.UserIDFromContext(r.Context())
	if !ok {
		http.Error(w, `{"error":"unauthorized"}`, http.StatusUnauthorized)
		return
	}

	page, limit := 1, 20

	if value := r.URL.Query().Get("page"); value != "" {
		n, err := strconv.Atoi(value)
		if err != nil || n < 1 {
			http.Error(w, `{"error":"invalid page"}`, http.StatusBadRequest)
			return
		}
		page = n
	}

	if value := r.URL.Query().Get("limit"); value != "" {
		n, err := strconv.Atoi(value)
		if err != nil || n < 1 || n > 100 {
			http.Error(w, `{"error":"limit must be between 1 and 100"}`, http.StatusBadRequest)
			return
		}
		limit = n
	}

	offset := int64(page-1) * int64(limit)
	if offset > int64(1<<31-1) {
		http.Error(w, `{"error":"page is too large"}`, http.StatusBadRequest)
		return
	}

	rows, err := db.New(h.pool).ListUserConversations(
		r.Context(),
		db.ListUserConversationsParams{
			UserID: pgtype.UUID{
				Bytes: [16]byte(userID),
				Valid: true,
			},
			PageSize:   int32(limit),
			PageOffset: int32(offset),
		},
	)
	if err != nil {
		http.Error(w, `{"error":"could not list conversations"}`, http.StatusInternalServerError)
		return
	}

	items := make([]conversationListItem, 0, len(rows))

	for _, row := range rows {
		item := conversationListItem{
			ID:        uuid.UUID(row.ID.Bytes),
			Type:      row.Type,
			CreatedAt: row.CreatedAt.Time,
			UpdatedAt: row.UpdatedAt.Time,
			Role:      row.Role,
		}

		if row.Title.Valid {
			title := row.Title.String
			item.Title = &title
		}

		items = append(items, item)
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)

	_ = json.NewEncoder(w).Encode(listConversationsResponse{
		Items: items,
		Page:  page,
		Limit: limit,
	})
}
