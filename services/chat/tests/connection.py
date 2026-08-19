import json
import socket
import struct


HOST = "127.0.0.1"
PORT = 9000

JWT_TOKEN = "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJ0b2tlbl90eXBlIjoiYWNjZXNzIiwiZXhwIjoxODE4NjU2NTUxLCJpYXQiOjE3ODcxMjA1NTEsImp0aSI6IjM5MDI0MjE0NTE5YzQzYThhOWQyOWJjNjJiMGJjMGUyIiwidXNlcl9pZCI6IjVlNDNmMmQ1LTNlMjktNDk3NS1iZjg5LTBjOGQ0YTFlODU4YSJ9.ILpaEAC1SDcaa-hDqwl937oGTB3UXj3v3rcDLNlQ3ZpgWtWKfz74NsDnjYDOe9gz9gjHs26xwO7k0MQaLXM0rCvWhT-z6uLaFbinERWk-kdkCyM6ia-0LeHwn-NkZbREEGfm1vUCBLmSV-J92TgO9wyuclU088l4E5tkIzG843gnvq_gRK0zWmXFYz8J7bpIa-5JFnVWju8PTNJHa4t8O1xYA80Qnyb5IIvKAkGcUn_0lZSz3iXCPSylyU2o5GoAWhSXE4SYh7rysX-5kPhw3uANs70rvoWAxRjub-mbBVlA9Wk94IKbimOwOnqERgnytcXk-sTIom_RVuOvQOUbhw"


def send_frame(sock: socket.socket, payload: dict):
    data = json.dumps(payload).encode("utf-8")

    header = struct.pack(">I", len(data))

    sock.sendall(header)
    sock.sendall(data)


def read_exact(sock: socket.socket, size: int) -> bytes:
    data = b""

    while len(data) < size:
        chunk = sock.recv(size - len(data))

        if not chunk:
            raise ConnectionError(
                "Connection closed by server"
            )

        data += chunk

    return data


def read_frame(sock: socket.socket):
    header = read_exact(sock, 4)

    length = struct.unpack(">I", header)[0]

    data = read_exact(sock, length)

    return json.loads(
        data.decode("utf-8")
    )


def auth(sock: socket.socket, token: str):
    send_frame(
        sock,
        {
            "type": "Auth",
            "data": {
                "token": token
            }
        }
    )

    return read_frame(sock)


def ping(sock: socket.socket):
    send_frame(
        sock,
        {
            "type": "Ping"
        }
    )

    return read_frame(sock)


def main():
    print("Connecting to Rust Chat...")

    with socket.create_connection(
        (HOST, PORT)
    ) as sock:

        print("Connected.")


        print("Authenticating...")

        response = auth(
            sock,
            JWT_TOKEN
        )

        print(
            "AUTH RESPONSE:",
            json.dumps(
                response,
                indent=2
            )
        )


        print("Sending ping...")

        response = ping(sock)

        print(
            "PING RESPONSE:",
            json.dumps(
                response,
                indent=2
            )
        )


if __name__ == "__main__":
    main()