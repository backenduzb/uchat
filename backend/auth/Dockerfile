FROM golang:1.27.1-alpine AS builder

WORKDIR /auth

COPY go.mod go.sum ./
RUN go mod download

COPY . .

RUN CGO_ENABLED=0 GOOS=linux go build \
    -trimpath \
    -ldflags="-s -w" \
    -o main ./cmd/server/main.go

FROM alpine:latest

RUN apk --no-cache add ca-certificates

WORKDIR /app

COPY --from=builder /auth/main ./main
COPY .env.example ./.env

EXPOSE 8080

CMD ["./main"]
