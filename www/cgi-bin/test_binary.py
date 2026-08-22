#!/usr/bin/env python3

import os
import sys

content_length = int(os.environ.get("CONTENT_LENGTH", "0"))
body = sys.stdin.buffer.read(content_length)

sys.stdout.buffer.write(b"Content-Type: application/octet-stream\r\n\r\n")
sys.stdout.buffer.write(body)