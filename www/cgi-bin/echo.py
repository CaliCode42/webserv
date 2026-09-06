#!/usr/bin/env python3
import os
import sys

length = int(os.environ.get("CONTENT_LENGTH", "0") or "0")
body = sys.stdin.buffer.read(length) if length > 0 else b""

sys.stdout.write("Content-Type: text/plain\r\n")
sys.stdout.write("\r\n")
sys.stdout.write("Method: %s\n" % os.environ.get("REQUEST_METHOD", ""))
sys.stdout.write("Query: %s\n" % os.environ.get("QUERY_STRING", ""))
sys.stdout.flush()

if body:
    sys.stdout.buffer.write(b"Body: ")
    sys.stdout.buffer.write(body)
    sys.stdout.buffer.write(b"\n")
