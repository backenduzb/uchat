package conversations

import (
	"encoding/json"
	"errors"
	"net/http"
	"time"

	authmiddleware "chat/auth/internal/auth"
	db "chat/auth/internal/database/sqlc"

	"github.com/google/uuid"
	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgconn"
	"github.com/jackc/pgx/v5/pgtype"
)

type conversationMemberResponse struct {
	UserID   uuid.UUID `json:"user_id"`
	Username string    `json:"username"`
	Role     string    `json:"role"`
	JoinedAt time.Time `json:"joined_at"`
}

func (h *Handler) ListMembers(w http.ResponseWriter, r *http.Request) {
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

	queries := db.New(h.pool)

	_, err = queries.GetConversationForMember(
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

		http.Error(w, `{"error":"could not verify conversation membership"}`, http.StatusInternalServerError)
		return
	}

	rows, err := queries.ListConversationMembers(
		r.Context(),
		db.ListConversationMembersParams{
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
		http.Error(w, `{"error":"could not list conversation members"}`, http.StatusInternalServerError)
		return
	}

	members := make([]conversationMemberResponse, 0, len(rows))
	for _, row := range rows {
		members = append(members, conversationMemberResponse{
			UserID:   uuid.UUID(row.UserID.Bytes),
			Username: row.Username,
			Role:     row.Role,
			JoinedAt: row.JoinedAt.Time,
		})
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	_ = json.NewEncoder(w).Encode(members)
}

type addMemberRequest struct {
	UserID string `json:"user_id"`
}

type addMemberResponse struct {
    UserID   uuid.UUID `json:"user_id"`
    Role     string    `json:"role"`
    JoinedAt time.Time `json:"joined_at"`
}

func (h *Handler) AddMember(w http.ResponseWriter, r *http.Request) {
	requesterID, ok := authmiddleware.UserIDFromContext(r.Context())
	if !ok {
		http.Error(w, `{"error":"unauthorized"}`, http.StatusUnauthorized)
		return
	}

	conversationID, err := uuid.Parse(r.PathValue("id"))
	if err != nil {
		http.Error(w, `{"error":"invalid conversation id"}`, http.StatusBadRequest)
		return
	}

	var req addMemberRequest
	decoder := json.NewDecoder(r.Body)
	decoder.DisallowUnknownFields()

	if err := decoder.Decode(&req); err != nil {
		http.Error(w, `{"error":"invalid request body"}`, http.StatusBadRequest)
		return
	}

	newUserID, err := uuid.Parse(req.UserID)
	if err != nil {
		http.Error(w, `{"error":"invalid user_id"}`, http.StatusBadRequest)
		return
	}

	ctx := r.Context()
	tx, err := h.pool.Begin(ctx)
	if err != nil {
		http.Error(w, `{"error":"could not start transaction"}`, http.StatusInternalServerError)
		return
	}
	defer func() { _ = tx.Rollback(ctx) }()

	queries := db.New(tx)

	role, err := queries.GetConversationRole(ctx, db.GetConversationRoleParams{
		ConversationID: pgtype.UUID{Bytes: conversationID, Valid: true},
		UserID:         pgtype.UUID{Bytes: requesterID, Valid: true},
	})
	if err != nil {
		if err == pgx.ErrNoRows {
			http.Error(w, `{"error":"conversation not found or access denied"}`, http.StatusNotFound)
			return
		}
		http.Error(w, `{"error":"could not verify permissions"}`, http.StatusInternalServerError)
		return
	}

	if role != "owner" && role != "admin" {
		http.Error(w, `{"error":"insufficient permissions"}`, http.StatusForbidden)
		return
	}

	member, err := queries.AddMemberToConversation(ctx, db.AddMemberToConversationParams{
		ConversationID: pgtype.UUID{Bytes: conversationID, Valid: true},
		NewUserID:      pgtype.UUID{Bytes: newUserID, Valid: true},
	})
	if err != nil {
		if err == pgx.ErrNoRows {
			http.Error(w, `{"error":"group conversation not found"}`, http.StatusNotFound)
			return
		}

		// PostgreSQL unique_violation: the user is already a member.
		var pgErr *pgconn.PgError
		if errors.As(err, &pgErr) && pgErr.Code == "23505" {
			http.Error(w, `{"error":"user is already a member"}`, http.StatusConflict)
			return
		}

		// PostgreSQL foreign_key_violation: the target user does not exist.
		if errors.As(err, &pgErr) && pgErr.Code == "23503" {
			http.Error(w, `{"error":"user not found"}`, http.StatusNotFound)
			return
		}

		http.Error(w, `{"error":"could not add member"}`, http.StatusInternalServerError)
		return
	}

	if err := tx.Commit(ctx); err != nil {
		http.Error(w, `{"error":"could not commit transaction"}`, http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusCreated)
	_ = json.NewEncoder(w).Encode(addMemberResponse{
		UserID:   uuid.UUID(member.UserID.Bytes),
		Role:     member.Role,
		JoinedAt: member.JoinedAt.Time,
	})
}
