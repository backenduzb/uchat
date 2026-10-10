package auth

import (
	"context"
	"net/http"
	"strings"

	authservice "chat/auth/services/auth"

	"github.com/google/uuid"
)

type contextKey string

const userIDKey contextKey = "authenticated_user_id"

func AuthMiddleware(secret []byte, next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		header := r.Header.Get("Authorization")
		parts := strings.Fields(header)

		if len(parts) != 2 || !strings.EqualFold(parts[0], "Bearer") {
			http.Error(w, `{"error":"missing or invalid authorization header"}`,
				http.StatusUnauthorized)
			return
		}

		userID, err := authservice.VerifyAccessToken(parts[1], secret)
		if err != nil || userID == uuid.Nil {
			http.Error(w, `{"error":"invalid or expired token"}`,
				http.StatusUnauthorized)
			return
		}

		ctx := context.WithValue(r.Context(), userIDKey, userID)
		next.ServeHTTP(w, r.WithContext(ctx))
	})
}

func UserIDFromContext(ctx context.Context) (uuid.UUID, bool) {
	userID, ok := ctx.Value(userIDKey).(uuid.UUID)
	return userID, ok
}
