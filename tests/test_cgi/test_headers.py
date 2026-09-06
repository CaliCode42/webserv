#!/usr/bin/env python3

import os

print("Content-Type: text/plain")
print()
print("HTTP_HOST=" + os.environ.get("HTTP_HOST", ""))
print("HTTP_X_WEBSERV_TEST=" + os.environ.get("HTTP_X_WEBSERV_TEST", ""))
print("CONTENT_TYPE=" + os.environ.get("CONTENT_TYPE", ""))
print("CONTENT_LENGTH=" + os.environ.get("CONTENT_LENGTH", ""))