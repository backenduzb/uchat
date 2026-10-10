# Chat

<p align="center">
  <strong>A real-time messaging platform built with Go, Rust, PostgreSQL, and Qt.</strong>
  <br />
  Designed for secure communication, low-latency messaging, and a modular architecture.
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Go-00ADD8?style=flat-square&logo=go&logoColor=white" alt="Go" />
  <img src="https://img.shields.io/badge/Rust-000000?style=flat-square&logo=rust&logoColor=white" alt="Rust" />
  <img src="https://img.shields.io/badge/PostgreSQL-4169E1?style=flat-square&logo=postgresql&logoColor=white" alt="PostgreSQL" />
  <img src="https://img.shields.io/badge/Qt-41CD52?style=flat-square&logo=qt&logoColor=white" alt="Qt" />
  <img src="https://img.shields.io/badge/Platform-Linux%20%7C%20Desktop-lightgrey?style=flat-square" alt="Platform" />
</p>

---

## Overview

**Chat** is a desktop messaging platform under active development, built with a focus on performance, security, and maintainable systems architecture.

### Architecture

| Component | Technology | Responsibility |
|---|---|---|
| Authentication API | Go | Registration, login, JWT authentication |
| Realtime service | Rust | Realtime connections and message delivery |
| Database | PostgreSQL | Users, conversations, memberships, messages |
| Desktop client | C++ / Qt | Desktop user interface |
| Query generation | sqlc | Type-safe Go database access |

Go manages authentication and conversation metadata. Rust is intended to handle realtime messaging, while PostgreSQL provides persistent storage.

## Features

- [x] User registration and login
- [x] Argon2id password hashing
- [x] JWT access tokens
- [x] Protected HTTP endpoints
- [x] PostgreSQL connection pooling
- [x] Conversation creation with transactional membership
- [x] Type-safe SQL queries with sqlc
- [ ] Conversation listing and member management
- [ ] Realtime messaging service
- [ ] Qt desktop client integration

## Project Structure

```text
chat/
├── backend/
│   ├── auth/          # Go authentication and API
│   │   ├── cmd/
│   │   ├── config/
│   │   ├── db/
│   │   ├── internal/
│   │   ├── migrations/
│   │   ├── routes/
│   │   └── sqlc.yaml
│   └── sfu/           # Rust realtime service
├── client/            # Qt desktop client (planned)
└── README.md
```

*The structure may evolve as development progresses.*

## Getting Started

### Requirements

- Go
- Rust toolchain and Cargo
- PostgreSQL
- sqlc

### 1. Clone the repository

```bash
git clone <repository-url>
cd chat
```

### 2. Configure PostgreSQL

Create a database:

```sql
CREATE DATABASE chat;
```

Configure the database connection and authentication settings in `backend/auth/.env` according to the application's configuration.

Example environment variables:

```dotenv
PORT=:8080
DB_URL=postgres://USER:PASSWORD@localhost:5432/chat
JWT_SECRET=your-long-random-secret
```

> Never commit `.env` files, database credentials, or JWT secrets.

### 3. Set up the Go API

```bash
cd backend/auth
```

Apply the database migrations in their intended order, if they have not already been applied. Then generate the database code:

```bash
sqlc generate
```

Run the API:

```bash
go mod download
go run ./cmd/server
```

Health check:

```bash
curl -i http://localhost:8080/health
```

## API

Base URL: `http://localhost:8080`

| Method | Endpoint | Purpose |
|---|---|---|
| `GET` | `/health` | Health check |
| `POST` | `/auth/register` | Register a user |
| `POST` | `/auth/login` | Authenticate a user |
| `POST` | `/conversations` | Create a conversation |

### Register

`POST /auth/register`

```json
{
  "email": "user@example.com",
  "username": "user123",
  "password": "replace-with-a-strong-password"
}
```

### Login

`POST /auth/login`

```json
{
  "email": "user@example.com",
  "password": "replace-with-a-strong-password"
}
```

Use the returned access token for protected endpoints:

```http
Authorization: Bearer <access-token>
```

### Create a group conversation

`POST /conversations`

```json
{
  "type": "group",
  "title": "Developers"
}
```

A successful request returns `201 Created` with the conversation ID and metadata.

## Development

Run the Go checks:

```bash
cd backend/auth
gofmt -w .
go test ./...
go vet ./...
```

Regenerate SQL code after changing queries or schema definitions:

```bash
sqlc generate
```

Generated code should remain consistent with the SQL source files and `sqlc.yaml`.

## Security

- Passwords are hashed using Argon2id.
- Protected endpoints validate JWT signatures and claims.
- Database operations use parameterized queries.
- Conversation creation uses a transaction.
- Secrets belong in environment variables, never in source control.

**Production deployment requires additional hardening**, including rate limiting, secure secret management, TLS, authorization checks, and least-privilege database roles.

## Roadmap

- [ ] Conversation listing and retrieval
- [ ] Direct messaging and group membership management
- [ ] Realtime messaging with Rust
- [ ] Message persistence and history
- [ ] Delivery acknowledgements and reconnect handling
- [ ] Qt desktop client
- [ ] Automated tests and CI
- [ ] Production deployment

## Contributing

Contributions, bug reports, and ideas are welcome.

1. Fork the repository.
2. Create a feature branch.
3. Make your changes and run the relevant checks.
4. Open a pull request describing the change.

## License

No license has been specified yet. Add a `LICENSE` file before distributing the project or accepting contributions under explicit reuse terms.
