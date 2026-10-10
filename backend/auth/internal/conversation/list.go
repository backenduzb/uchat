package conversation

import (
	"net/http"
	"strconv"

	authmiddleware "chat/auth/internal/auth"
	db "chat/auth/internal/database/sqlc"
	"chat/auth/models"
	"chat/auth/schemas"
)

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
			UserID:     pgUUID(userID),
			PageSize:   int32(limit),
			PageOffset: int32(offset),
		},
	)
	if err != nil {
		http.Error(w, `{"error":"could not list conversations"}`, http.StatusInternalServerError)
		return
	}

	summaries := make([]models.ConversationSummary, 0, len(rows))
	for _, row := range rows {
		summaries = append(summaries, models.ConversationSummary{
			ID:        uuidFromPg(row.ID),
			Type:      row.Type,
			Title:     textToPtr(row.Title),
			CreatedAt: row.CreatedAt.Time,
			UpdatedAt: row.UpdatedAt.Time,
			Role:      row.Role,
		})
	}

	writeJSON(w, http.StatusOK, schemas.NewConversationListResponse(summaries, page, limit))
}
