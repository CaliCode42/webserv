#!/usr/bin/env python3

with open("cgi_relative_data.txt", "r") as file:
	data = file.read().strip()

print("Content-Type: text/plain")
print()
print(data)