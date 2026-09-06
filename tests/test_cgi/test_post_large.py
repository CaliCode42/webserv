#!/usr/bin/env python3

import os
import sys
import time

content_length = int(os.environ.get("CONTENT_LENGTH", "0"))

time.sleep(0.1)

body = sys.stdin.buffer.read(content_length)

print("Content-Type: text/plain")
print()
print("Length: " + str(len(body)))

if len(body) == content_length and body == b"A" * content_length:
	print("Integrity: OK")
else:
	print("Integrity: FAIL")