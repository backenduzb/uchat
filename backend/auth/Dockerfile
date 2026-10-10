FROM golang:1.26-alpine AS builder

WORKDIR /auth

COPY go.mod /auth/go.mod
RUN go mod download

COPY . .
RUN CGO_ENABLED=0 GOOS=linux go build -o main ./cmd/server/main.go

FROM alpine:latest

RUN apk --no-cache add ca-certificates
WORKDIR /root/

COPY --from=builder /auth/main .
COPY .env.examle .env

EXPOSE 8080

CMD ["./main"]