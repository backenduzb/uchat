import json
import socket
import struct
import threading
import time

HOST = "127.0.0.1"
PORT = 9000

USER_1_TOKEN = "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJ0b2tlbl90eXBlIjoiYWNjZXNzIiwiZXhwIjoxODE4NjU4NzQ1LCJpYXQiOjE3ODcxMjI3NDUsImp0aSI6ImQ3ZWM4NzQ2ODA0MjRkODc5ODkyNWFmMTY2ZWI5MGE3IiwidXNlcl9pZCI6IjVlNDNmMmQ1LTNlMjktNDk3NS1iZjg5LTBjOGQ0YTFlODU4YSJ9.StjQf8IrVAowdrcsvWf0-gErZWiiEf4Int2BsbkBvZn66vpCcvfS4uQV-pcVM7pMNcF811FVRgeu-rkdjp1spi9g1GdYaii8Zdb4pSzknUmBaaKibS3fW2NAcdD-izW1m6qnbNWFJRqxtgR-91oF61Sf1qQQU0FpYgjlRu_KfrArnvu6hA-Hsa1ScP3ktSpNootAyQUiLqnCJmewgp2JppvQnaWgdOzGr1cLgSxEBdlXK4ebQHy2Z2ORSXeMRQgBSMCJ3IMXTrYK7JwdNmQKfNMyPFlEPnzGpxi04DCFzsMzZnaHcNnkDEI54qZPGWM7KRbRTZoR7as8ZTB7YPQFzA" 
USER_2_TOKEN = "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJ0b2tlbl90eXBlIjoiYWNjZXNzIiwiZXhwIjoxODE4NjU4OTcwLCJpYXQiOjE3ODcxMjI5NzAsImp0aSI6IjA0NWFhY2ZjZmYxMTRmNTI4NGVhN2VjM2FmN2I0YzQ5IiwidXNlcl9pZCI6IjE4MTkzZDAyLWVlZGEtNDE2NC1hOTkxLWNhYmY4NzRjNWQ5MiJ9.EhVyrQModzjcQ0klztJlitUX6dcvpAKXIhWYQhYLlW0x5us-8yItFwmvOz0bHovPgzOxqqgI1L2xcAi3Qt_6sud4ibxwfyko-VCGwMsyiAbexvEGsTws335wAYIvJ4_FPN88X7rIFHn7PhktoEPN1VRKBCvnjT2m5Var13KC9MwSlXSg4yTvVbXAuiwgTgbcKhj_fV9cD9VCp5dkK0C4Xzk-FBH4i7GYidR5EXh_cs8k54o4ZuCw3FgNctk_V70xC3Qy4dteXBC1UkY88RQCLLQUCy8h9p1HNFPVjrWUeZe5VsOuVA-RWjNMrm_mT-gEemICM3XakNl2Dtrk72h71g" 

SHARED_CONVERSATION_ID = "11111111-2222-3333-4444-555555555555" 


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
            raise ConnectionError("Connection closed by server")
        data += chunk
    return data


def read_frame(sock: socket.socket):
    header = read_exact(sock, 4)
    length = struct.unpack(">I", header)[0]
    data = read_exact(sock, length)
    return json.loads(data.decode("utf-8"))


def auth(sock: socket.socket, token: str):
    send_frame(sock, {
        "type": "Auth",
        "data": {"token": token}
    })

def send_chat_message(sock: socket.socket, conversation_id: str, text: str):
    send_frame(sock, {
        "type": "SendMessage",
        "data": {
            "conversation_id": conversation_id,
            "text": text
        }
    })



def listen_for_messages(sock: socket.socket):
    try:
        while True:
            response = read_frame(sock)
            print(f"\n[SERVERDAN KELDI]: {json.dumps(response)}")
            print("> ", end="", flush=True)
    except Exception as e:
        print(f"\n[XATOLIK] Aloqa uzildi: {e}")


def main():
    print("KIM SIFATIDA KIRASIZ?")
    print("1 - User 1")
    print("2 - User 2")
    user_choice = input("Tanlang (1 yoki 2): ")

    if user_choice == "1":
        my_token = USER_1_TOKEN
        print("User 1 tanlandi.")
    elif user_choice == "2":
        my_token = USER_2_TOKEN
        print("User 2 tanlandi.")
    else:
        print("Xato tanlov!")
        return

    print("Rust Chat serveriga ulanilmoqda...")

    try:
        sock = socket.create_connection((HOST, PORT))
        print("Ulandi!\n")

        listener = threading.Thread(target=listen_for_messages, args=(sock,), daemon=True)
        listener.start()

        auth(sock, my_token)
        time.sleep(1) 

        while True:
            text = input("\nXabar yozing (yoki chiqish uchun '0'): \n> ")
            if text == "0":
                break
            elif text.strip() != "":
                send_chat_message(sock, SHARED_CONVERSATION_ID, text)

    except ConnectionRefusedError:
        print("Serverga ulanib bo'lmadi.")
    finally:
        if 'sock' in locals():
            sock.close()

if __name__ == "__main__":
    main()