package schemas

import (
	"chat/auth/models"
	"time"

	"github.com/google/uuid"
)

type UserResponse struct {
	ID uuid.UUID `json:"id"`
	Username string `json:"username"`
	Email string `json:"email"`
	DateOfBirth *time.Time `json:"date_of_birth, omitempty"`
	CreateAt time.Time `json:"created_at"`
}

type LoginResponse struct {
	AccessToken string 	`json:"access_token"`
	TokenType string `json:"token_type"`
	ExpiresIn int  `json:"expires_in"`
	User UserResponse `json:"user"`
}

func NewUserResponse(user models.User) UserResponse {
	return UserResponse{
		ID: user.ID,
		Username: user.Username,
		Email: user.Email,
		DateOfBirth: user.DateOfBirth,
		CreateAt: user.CreatedAt,
	}
}

func NewLoginResponse(token string, user models.User) LoginResponse {
	return LoginResponse {
		AccessToken: token,
		TokenType: "Bearer",
		ExpiresIn: 900,
		User: NewUserResponse(user),
	}
}