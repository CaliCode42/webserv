#!/usr/bin/env bash
# Load test with concurrent connections (requires 'siege').
# Usage: ./load_test.sh [path/to/config.conf]

set -u

CONFIG="${1:-test.conf}"
HOST="${HOST:-localhost}"
PORT="${PORT:-8080}"
BASE_URL="http://${HOST}:${PORT}"
WEBSERV_BIN="${WEBSERV_BIN:-./webserv}"

if ! command -v siege > /dev/null 2>&1; then
    echo "[SKIP] 'siege' not installed."
    exit 0
fi

if [ ! -x "${WEBSERV_BIN}" ]; then
    echo "Error: ${WEBSERV_BIN} not found. Run 'make' first."
    exit 1
fi

pkill -9 -f "${WEBSERV_BIN} ${CONFIG}" 2>/dev/null
"${WEBSERV_BIN}" "${CONFIG}" &
SERVER_PID=$!
sleep 2

siege -c 20 -t 15s "${BASE_URL}/"

kill -9 "${SERVER_PID}" 2>/dev/null
