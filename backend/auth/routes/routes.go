package routes

import (
	"net/http"

	authhandler "chat/auth/internal/auth"
	conversationhandler "chat/auth/internal/conversation"

	"github.com/jackc/pgx/v5/pgxpool"
)

func NewRouter(db *pgxpool.Pool, jwtSecret []byte) http.Handler {
	mux := http.NewServeMux()

	registerHealRoutes(mux, db)
	registerAuthRoutes(mux, authhandler.NewHandler(db, jwtSecret))

	conversations := conversationhandler.NewHandler(db)

	mux.Handle(
		"POST /conversations",
		authhandler.AuthMiddleware(jwtSecret, http.HandlerFunc(conversations.Create)),
	)

	mux.Handle(
		"GET /conversations",
		authhandler.AuthMiddleware(
			jwtSecret,
			http.HandlerFunc(conversations.List),
		),
	)
	mux.Handle(
		"GET /conversations/{id}",
		authhandler.AuthMiddleware(
			jwtSecret,
			http.HandlerFunc(conversations.Get),
		),
	)
	mux.Handle(
		"GET /conversations/{id}/members",
		authhandler.AuthMiddleware(
			jwtSecret,
			http.HandlerFunc(conversations.ListMembers),
		),
	)

	mux.Handle(
		"POST /conversations/{id}/members",
		authhandler.AuthMiddleware(
			jwtSecret,
			http.HandlerFunc(conversations.AddMember),
		),
	)
	mux.Handle(
		"DELETE /conversations/{id}/members/{user_id}",
		authhandler.AuthMiddleware(
			jwtSecret,
			http.HandlerFunc(conversations.RemoveMember),
		),
	)

	return mux
}

func registerHealRoutes(mux *http.ServeMux, db *pgxpool.Pool) {
	mux.HandleFunc("GET /health", func(w http.ResponseWriter, r *http.Request) {
		if err := db.Ping(r.Context()); err != nil {
			http.Error(w, `{"status":"error","database":"disconnected"}`, http.StatusServiceUnavailable)
			return
		}

		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(http.StatusOK)
		_, _ = w.Write([]byte(`{"status":"ok","database":"connected"}`))
	})
}

func registerAuthRoutes(mux *http.ServeMux, handler *authhandler.Handler) {
	mux.HandleFunc("POST /auth/register", handler.Register)
	mux.HandleFunc("POST /auth/login", handler.Login)
}
