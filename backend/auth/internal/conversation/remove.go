package conversation

import (
	"encoding/json"
	"errors"
	"net/http"

	authmiddleware "chat/auth/internal/auth"
	db "chat/auth/internal/database/sqlc"

	"github.com/google/uuid"
	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgtype"
)

func (h *Handler) RemoveMember(w http.ResponseWriter, r *http.Request) {
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

	targetID, err := uuid.Parse(r.PathValue("user_id"))
	if err != nil {
		http.Error(w, `{"error":"invalid user id"}`, http.StatusBadRequest)
		return
	}

	ctx := r.Context()

	tx, err := h.pool.Begin(ctx)
	if err != nil {
		http.Error(w, `{"error":"could not start transaction"}`, http.StatusInternalServerError)
		return
	}
	defer func() {
		_ = tx.Rollback(ctx)
	}()

	queries := db.New(tx)

	conversation, err := queries.LockConversationForMemberRemoval(
		ctx,
		pgtype.UUID{
			Bytes: conversationID,
			Valid: true,
		},
	)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			http.Error(w, `{"error":"conversation not found"}`, http.StatusNotFound)
			return
		}

		http.Error(w, `{"error":"could not lock conversation"}`, http.StatusInternalServerError)
		return
	}

	_ = conversation

	requesterRole, err := queries.GetConversationRole(
		ctx,
		db.GetConversationRoleParams{
			ConversationID: pgtype.UUID{
				Bytes: conversationID,
				Valid: true,
			},
			UserID: pgtype.UUID{
				Bytes: requesterID,
				Valid: true,
			},
		},
	)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			http.Error(w, `{"error":"conversation not found"}`, http.StatusNotFound)
			return
		}

		http.Error(w, `{"error":"could not verify permissions"}`, http.StatusInternalServerError)
		return
	}

	targetRole, err := queries.GetConversationRole(
		ctx,
		db.GetConversationRoleParams{
			ConversationID: pgtype.UUID{
				Bytes: conversationID,
				Valid: true,
			},
			UserID: pgtype.UUID{
				Bytes: targetID,
				Valid: true,
			},
		},
	)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			http.Error(w, `{"error":"member not found"}`, http.StatusNotFound)
			return
		}

		http.Error(w, `{"error":"could not verify member"}`, http.StatusInternalServerError)
		return
	}

	if requesterRole != "owner" && requesterRole != "admin" {
		if requesterRole != "member" || requesterID != targetID {
			http.Error(w, `{"error":"insufficient permissions"}`, http.StatusForbidden)
			return
		}
	}
	if targetRole == "owner" {
		http.Error(w, `{"error":"transfer ownership before removing an owner"}`, http.StatusConflict)
		return
	}

	if conversation.Type != "group" {
		http.Error(w, `{"error":"members can only be removed from group conversations"}`, http.StatusConflict)
		return
	}

	err = queries.RemoveConversationMember(
		ctx,
		db.RemoveConversationMemberParams{
			ConversationID: pgtype.UUID{
				Bytes: conversationID,
				Valid: true,
			},
			UserID: pgtype.UUID{
				Bytes: targetID,
				Valid: true,
			},
		},
	)
	if err != nil {
		http.Error(w, `{"error":"could not remove member"}`, http.StatusInternalServerError)
		return
	}

	if err := tx.Commit(ctx); err != nil {
		http.Error(w, `{"error":"could not commit transaction"}`, http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusNoContent)
	_ = json.NewEncoder(w).Encode(nil)
}
