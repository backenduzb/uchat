package models

import (
	"github.com/google/uuid"
	"time"
)

type User struct {
	ID           uuid.UUID  `json:"id"`
	Username     string     `json:"username"`
	Email        string     `json:"email"`
	PasswordHash string     `json:"-"`
	DateOfBirth  *time.Time `json:"date_of_birth,omitempty"`
	CreatedAt    time.Time  `json:"created_at"`
}
