#!/usr/bin/env bash

printf 'Content-Type: text/plain\r\n'
printf '\r\n'

printf 'Hello from Bash CGI\n'
printf 'Method: %s\n' "$REQUEST_METHOD"
printf 'Query: %s\n' "$QUERY_STRING"
printf 'Content-Type: %s\n' "$CONTENT_TYPE"
printf 'Content-Length: %s\n' "$CONTENT_LENGTH"

printf 'Body: '
cat
printf '\n'