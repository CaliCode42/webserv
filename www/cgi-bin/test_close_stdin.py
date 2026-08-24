#!/usr/bin/env python3

import os
import time

os.close(0)
time.sleep(0.1)

print("Content-Type: text/plain")
print()
print("CGI survived")