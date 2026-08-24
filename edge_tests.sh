#!/usr/bin/env bash
# Tests de cas limites non couverts par test_webserv.sh
# Usage: ./edge_tests.sh [host] [port]

HOST="${1:-localhost}"
PORT="${2:-8080}"

GREEN="\033[0;32m"
RED="\033[0;31m"
RESET="\033[0m"
PASS=0
FAIL=0

raw_status() {
	printf "%b" "$1" | nc "$HOST" "$PORT" | head -n 1 | awk '{print $2}'
}

check() {
	local name="$1" expected="$2" got="$3"
	if [ "$got" = "$expected" ]; then
		printf "${GREEN}[PASS]${RESET} %s (got %s)\n" "$name" "$got"
		PASS=$((PASS + 1))
	else
		printf "${RED}[FAIL]${RESET} %s (expected %s, got %s)\n" "$name" "$expected" "$got"
		FAIL=$((FAIL + 1))
	fi
}

echo "=== 1. Deux Content-Length differents (smuggling) ==="
status=$(raw_status "POST /test.txt HTTP/1.1\r\nHost: ${HOST}\r\nContent-Length: 5\r\nContent-Length: 10\r\n\r\nHello")
check "Duplicate Content-Length rejected" "400" "$status"

echo "=== 2. Chunk size en majuscules hexa ==="
status=$(raw_status "POST /test_upper.txt HTTP/1.1\r\nHost: ${HOST}\r\nTransfer-Encoding: chunked\r\n\r\n1A\r\nabcdefghijklmnopqrstuvwxyz\r\n0\r\n\r\n")
check "Uppercase hex chunk size accepted" "201" "$status"

echo "=== 3. Query string dans l'URI d'un GET ==="
status=$(curl -sS -o /dev/null -w "%{http_code}" "http://${HOST}:${PORT}/index.html?foo=bar" 2>/dev/null)
check "GET with query string" "200" "$status"

echo "=== 4. Version HTTP/1.0 (non standard pour ce projet) ==="
status=$(raw_status "GET /index.html HTTP/1.0\r\nHost: ${HOST}\r\n\r\n")
echo "  -> statut recu: ${status} (pas de PASS/FAIL, juste observation)"

echo "=== 5. Cookie malforme sans '=' ==="
status=$(raw_status "GET / HTTP/1.1\r\nHost: ${HOST}\r\nCookie: justakey\r\n\r\n")
check "Malformed cookie does not crash server" "200" "$status"

echo "=== 6. Chunk size enorme (risque d'overflow) ==="
status=$(raw_status "POST /overflow.txt HTTP/1.1\r\nHost: ${HOST}\r\nTransfer-Encoding: chunked\r\n\r\nFFFFFFFFFFFFFFFF\r\nAB\r\n0\r\n\r\n")
echo "  -> statut recu: ${status} (attendu: 400 ou 413, pas un crash silencieux)"

echo "=== 7. Requete sans header Host ==="
status=$(raw_status "GET /index.html HTTP/1.1\r\n\r\n")
echo "  -> statut recu: ${status} (le sujet HTTP/1.1 exige Host ; observation seulement)"

echo "=== 8. DELETE avec query string ==="
status=$(curl -sS -o /dev/null -w "%{http_code}" -X DELETE "http://${HOST}:${PORT}/uploads/inexistant.txt?x=1" 2>/dev/null)
check "DELETE with query string on missing file" "404" "$status"

echo "=== 9. Methode en minuscules (case-sensitive selon RFC) ==="
status=$(raw_status "get /index.html HTTP/1.1\r\nHost: ${HOST}\r\n\r\n")
check "Lowercase method rejected" "501" "$status"

echo "=== 10. Espaces multiples dans la ligne de requete ==="
status=$(raw_status "GET  /index.html  HTTP/1.1\r\nHost: ${HOST}\r\n\r\n")
check "Multiple spaces in request line still parsed" "200" "$status"

echo ""
echo "PASS: ${PASS}  FAIL: ${FAIL}"
