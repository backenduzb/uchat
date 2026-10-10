package auth

import (
	"encoding/json"
	"errors"
	"net/http"
	"net/mail"
	"strings"
	"time"
	"unicode/utf8"

	"chat/auth/models"
	"chat/auth/schemas"
	authservice "chat/auth/services/auth"
	"chat/auth/services/decode"

	"github.com/google/uuid"
	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgconn"
	"github.com/jackc/pgx/v5/pgxpool"
)

type Handler struct {
	db     *pgxpool.Pool
	secret []byte
}

func NewHandler(db *pgxpool.Pool, secret []byte) *Handler {
	return &Handler{
		db:     db,
		secret: secret,
	}
}

func writeJSON(w http.ResponseWriter, status int, value any) {
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(status)
	_ = json.NewEncoder(w).Encode(value)
}

func validEmail(email string) bool {
	if len(email) > 254 || !utf8.ValidString(email) {
		return false
	}

	addr, err := mail.ParseAddress(email)
	return err == nil && addr.Address == email
}

func validUsername(username string) bool {
	if len(username) < 3 || len(username) > 32 {
		return false
	}

	for _, r := range username {
		if !(r >= 'a' && r <= 'z') &&
			!(r >= 'A' && r <= 'Z') &&
			!(r >= '0' && r <= '9') &&
			r != '_' {
			return false
		}
	}

	return true
}

func validPassword(password string) bool {
	return utf8.ValidString(password) &&
		len(password) >= 12 &&
		len(password) <= 128
}

func (h *Handler) Register(w http.ResponseWriter, r *http.Request) {
	input, ok := decode.DecodeJSON[schemas.RegisterRequest](w, r)
	if !ok {
		return
	}

	username := strings.TrimSpace(input.Username)
	email := strings.ToLower(strings.TrimSpace(input.Email))

	if !validUsername(username) {
		http.Error(w,
			`{"error":"username must be 3-32 characters and contain only letters, numbers or underscores"}`,
			http.StatusBadRequest,
		)
		return
	}

	if !validEmail(email) {
		http.Error(w, `{"error":"invalid email"}`, http.StatusBadRequest)
		return
	}

	if !validPassword(input.Password) {
		http.Error(w,
			`{"error":"password must be 12-128 bytes"}`,
			http.StatusBadRequest,
		)
		return
	}

	hash, err := authservice.HashPassword(input.Password)
	if err != nil {
		http.Error(w, `{"error":"internal server error"}`,
			http.StatusInternalServerError)
		return
	}

	user := models.User{
		ID:           uuid.New(),
		Username:     username,
		Email:        email,
		PasswordHash: hash,
		CreatedAt:    time.Now().UTC(),
	}

	err = h.db.QueryRow(r.Context(), `
		INSERT INTO users (id, username, email, password_hash)
		VALUES ($1, $2, $3, $4)
		RETURNING created_at
	`,
		user.ID,
		user.Username,
		user.Email,
		user.PasswordHash,
	).Scan(&user.CreatedAt)

	if err != nil {
		var pgErr *pgconn.PgError
		if errors.As(err, &pgErr) && pgErr.Code == "23505" {
			switch pgErr.ConstraintName {
			case "users_username_unique":
				http.Error(w, `{"error":"username already taken"}`,
					http.StatusConflict)
			case "users_email_unique":
				http.Error(w, `{"error":"email already registered"}`,
					http.StatusConflict)
			default:
				http.Error(w, `{"error":"conflict"}`, http.StatusConflict)
			}
			return
		}

		http.Error(w, `{"error":"internal server error"}`,
			http.StatusInternalServerError)
		return
	}

	writeJSON(w, http.StatusCreated, schemas.NewUserResponse(user))
}

func (h *Handler) Login(w http.ResponseWriter, r *http.Request) {
	input, ok := decode.DecodeJSON[schemas.LoginRequest](w, r)
	if !ok {
		return
	}

	email := strings.ToLower(strings.TrimSpace(input.Email))

	if !validEmail(email) || !validPassword(input.Password) {
		http.Error(w, `{"error":"invalid email or password"}`,
			http.StatusBadRequest)
		return
	}

	var user models.User

	err := h.db.QueryRow(r.Context(), `
    SELECT id, username, email, password_hash, date_of_birth, created_at
    FROM users
    WHERE LOWER(email) = $1
	`, email).Scan(
		&user.ID,
		&user.Username,
		&user.Email,
		&user.PasswordHash,
		&user.DateOfBirth,
		&user.CreatedAt,
	)

	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			http.Error(w, `{"error":"invalid email or password"}`,
				http.StatusUnauthorized)
			return
		}

		http.Error(w, `{"error":"internal server error"}`,
			http.StatusInternalServerError)
		return
	}

	if !authservice.VerifyPassword(input.Password, user.PasswordHash) {
		http.Error(w, `{"error":"invalid email or password"}`,
			http.StatusUnauthorized)
		return
	}

	token, err := authservice.CreateAccessToken(user.ID, h.secret)
	if err != nil {
		http.Error(w, `{"error":"internal server error"}`,
			http.StatusInternalServerError)
		return
	}

	writeJSON(w, http.StatusOK, schemas.NewLoginResponse(token, user))
}
