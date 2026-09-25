import socket

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.settimeout(2)
s.connect(("127.0.0.1", 8080))

request = (
    "GET /index.html HTTP/1.1\r\n"
    "Host: localhost:8080\r\n"
    "Connection: close\r\n"
    "\r\n"
)

s.sendall(request.encode())

data = b""

while True:
    chunk = s.recv(4096)
    if not chunk:
        print("SERVER CLOSED CONNECTION: OK")
        break
    data += chunk

print(data.decode(errors="replace"))
s.close()