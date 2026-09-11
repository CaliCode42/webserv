#!/usr/bin/env bash
# Checks for memory leaks under mixed traffic (GET/POST/DELETE).
# macOS: launches webserv normally, attaches 'leaks' to the running PID.
# Linux: launches webserv directly under 'valgrind --leak-check=full'.
# Usage: ./memory_test.sh [path/to/config.conf]

set -u

CONFIG="${1:-test.conf}"
HOST="${HOST:-localhost}"
PORT="${PORT:-8080}"
BASE_URL="http://${HOST}:${PORT}"
WEBSERV_BIN="${WEBSERV_BIN:-./webserv}"

if [ ! -x "${WEBSERV_BIN}" ]; then
    echo "Error: ${WEBSERV_BIN} not found. Run 'make' first."
    exit 1
fi

pkill -9 -f "${WEBSERV_BIN} ${CONFIG}" 2>/dev/null

OS_NAME="$(uname)"

if [ "${OS_NAME}" = "Linux" ] && command -v valgrind > /dev/null 2>&1; then
    valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
        "${WEBSERV_BIN}" "${CONFIG}" &
    SERVER_PID=$!
else
    "${WEBSERV_BIN}" "${CONFIG}" &
    SERVER_PID=$!
fi

sleep 2

if ! kill -0 "${SERVER_PID}" 2>/dev/null; then
    echo "Error: server failed to start."
    exit 1
fi

for i in $(seq 1 20); do
    curl -s "${BASE_URL}/" > /dev/null
done
curl -s -X POST "${BASE_URL}/leak_test_upload.txt" -d "hello world" > /dev/null
curl -s -X DELETE "${BASE_URL}/uploads/leak_test_upload.txt" > /dev/null

if [ "${OS_NAME}" = "Darwin" ]; then
    leaks "${SERVER_PID}"
    kill -9 "${SERVER_PID}" 2>/dev/null
elif [ "${OS_NAME}" = "Linux" ] && command -v valgrind > /dev/null 2>&1; then
    # SIGTERM lets valgrind print its leak summary before exiting
    kill -TERM "${SERVER_PID}" 2>/dev/null
    wait "${SERVER_PID}" 2>/dev/null
else
    echo "[SKIP] no leak-detection tool available on this system."
    kill -9 "${SERVER_PID}" 2>/dev/null
fi
