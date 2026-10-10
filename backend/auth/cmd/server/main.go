package main

import (
	"log"
	"log/slog"
	"net/http"
	"time"

	"chat/auth/config"
	"chat/auth/internal/database"
	"chat/auth/routes"
	"context"
)

func main() {
	cfg, err := config.LoadEnv()
	if err != nil {
		log.Fatal(err)
	}
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	db, err := database.Connect(ctx, cfg.DBURL)
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()

	server := &http.Server{
		Addr:              cfg.PORT,
		Handler:           routes.NewRouter(db, []byte(cfg.JWT_SECRET)),
		ReadHeaderTimeout: 5 * time.Second,
		ReadTimeout:       10 * time.Second,
		WriteTimeout:      15 * time.Second,
		IdleTimeout:       60 * time.Second,
	}

	slog.Info("HTTP server starting", "address", cfg.PORT)

	if err := server.ListenAndServe(); err != nil && err != http.ErrServerClosed {
		log.Fatal(err)
	}
}
