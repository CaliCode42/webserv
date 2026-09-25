import socket
import time

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("localhost", 8080))

parts = [
    b"POST /trailer-fragment.txt HTTP/1.1\r\n",
    b"Host: localhost:8080\r\n",
    b"Transfer-Encoding: chunked\r\n",
    b"Trailer: X-Test\r\n",
    b"\r\n",
    b"5\r\n",
    b"hello",
    b"\r\n",
    b"0\r\n",
    b"X-Test: yes\r\n",
    b"\r\n",
]

for part in parts:
    s.sendall(part)
    time.sleep(0.05)

response = b""
while True:
    data = s.recv(4096)
    if not data:
        break
    response += data

print(response.decode("latin1"))
s.close()