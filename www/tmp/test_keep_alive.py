import socket

s = socket.create_connection(("127.0.0.1", 8080))
s.settimeout(2)

print("=== REQUEST 1 ===")

s.sendall(
    b"GET /index.html HTTP/1.1\r\n"
    b"Host: localhost:8080\r\n"
    b"\r\n"
)

try:
    data = s.recv(8192)
    print(data.decode(errors="replace"))
except Exception as e:
    print("recv 1 error:", e)

print("\n=== REQUEST 2 ON SAME SOCKET ===")

try:
    s.sendall(
        b"GET /index.html HTTP/1.1\r\n"
        b"Host: localhost:8080\r\n"
        b"\r\n"
    )
    print("send succeeded")
except Exception as e:
    print("send 2 error:", e)

try:
    data = s.recv(8192)
    if data:
        print(data.decode(errors="replace"))
    else:
        print("SERVER CLOSED CONNECTION (recv returned EOF)")
except Exception as e:
    print("recv 2 error:", e)

s.close()