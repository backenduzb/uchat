package config

import (
	"fmt"
	"os"
	
	"github.com/joho/godotenv"
)

type Config struct {
	DBURL     string
	PORT      string
	JWT_SECRET string
	DEBUG bool
}

func LoadEnv() (Config, error) {
	_ = godotenv.Load()

	debug := os.Getenv("DEBUG") == "true" || os.Getenv("DEBUG") == "1"
	
	cfg := Config{
		DBURL:      os.Getenv("DATABASE_URL"),
		PORT:       os.Getenv("PORT"),
		JWT_SECRET: os.Getenv("JWT_SECRET"),
		DEBUG: 		debug,
	}

	if cfg.DBURL == "" {
		return Config{}, fmt.Errorf("DATABASE_URL is required")
	}

	if cfg.PORT == "" {
		cfg.PORT = ":8080"
	} else if cfg.PORT[0] != ':' {
		cfg.PORT = ":" + cfg.PORT
	}

	if len(cfg.JWT_SECRET) < 32 {
		return Config{}, fmt.Errorf("JWT_SECRET must be at least 32 bytes")
	}

	return cfg, nil
}