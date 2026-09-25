import socket

s = socket.create_connection(("127.0.0.1", 8080))
s.settimeout(2)

print("=== CHUNKED REQUEST ===")

s.sendall(
    b"POST /uploads/keepalive-test.txt HTTP/1.1\r\n"
    b"Host: localhost:8080\r\n"
    b"Transfer-Encoding: chunked\r\n"
    b"Content-Type: text/plain\r\n"
    b"\r\n"
    b"5\r\n"
    b"hello\r\n"
    b"0\r\n"
    b"\r\n"
)

try:
    data = s.recv(8192)
    print(data.decode(errors="replace"))
except Exception as e:
    print("recv 1 error:", e)

print("\n=== GET ON SAME SOCKET ===")

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