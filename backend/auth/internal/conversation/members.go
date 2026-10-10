package conversation

import (
	"errors"
	"net/http"

	authmiddleware "chat/auth/internal/auth"
	db "chat/auth/internal/database/sqlc"
	"chat/auth/models"
	"chat/auth/schemas"
	"chat/auth/services/decode"

	"github.com/google/uuid"
	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgconn"
)

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
			ConversationID: pgUUID(conversationID),
			UserID:         pgUUID(userID),
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
			ConversationID: pgUUID(conversationID),
			UserID:         pgUUID(userID),
		},
	)
	if err != nil {
		http.Error(w, `{"error":"could not list conversation members"}`, http.StatusInternalServerError)
		return
	}

	members := make([]models.ConversationMember, 0, len(rows))
	for _, row := range rows {
		members = append(members, models.ConversationMember{
			ConversationID: conversationID,
			UserID:         uuidFromPg(row.UserID),
			Username:       row.Username,
			Role:           row.Role,
			JoinedAt:       row.JoinedAt.Time,
		})
	}

	writeJSON(w, http.StatusOK, schemas.NewConversationMemberListResponse(members))
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

	input, ok := decode.DecodeJSON[schemas.AddConversationMemberRequest](w, r)
	if !ok {
		return
	}

	if input.UserID == uuid.Nil {
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
		ConversationID: pgUUID(conversationID),
		UserID:         pgUUID(requesterID),
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
		ConversationID: pgUUID(conversationID),
		NewUserID:      pgUUID(input.UserID),
	})
	if err != nil {
		if err == pgx.ErrNoRows {
			http.Error(w, `{"error":"group conversation not found"}`, http.StatusNotFound)
			return
		}

		var pgErr *pgconn.PgError
		if errors.As(err, &pgErr) && pgErr.Code == "23505" {
			http.Error(w, `{"error":"user is already a member"}`, http.StatusConflict)
			return
		}

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

	added := models.ConversationMember{
		ConversationID: conversationID,
		UserID:         uuidFromPg(member.UserID),
		Role:           member.Role,
		JoinedAt:       member.JoinedAt.Time,
		LastReadAt:     timePtrFromTimestamptz(member.LastReadAt),
	}

	writeJSON(w, http.StatusCreated, schemas.NewConversationMemberAddResponse(added))
}
