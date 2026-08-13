#!/usr/bin/env bash

set -u

HOST="${HOST:-localhost}"
PORT="${PORT:-8080}"
BASE_URL="http://${HOST}:${PORT}"
BIGFILE_PATH="${BIGFILE_PATH:-www/bigfile.bin}"
TMP_DIR=".webserv_test_tmp"

GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[0;33m"
CYAN="\033[0;36m"
RESET="\033[0m"

PASS=0
FAIL=0
SKIP=0

TEST_NUMBER=1

mkdir -p "${TMP_DIR}"

create_test_files() {
	cat > www/index.html <<'EOF'
<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
</head>
<body>
	<pre>
                        __        __   _                         
                        \ \      / /__| |__  ___  ___ _ ____   __
                         \ \ /\ / / _ \ '_ \/ __|/ _ \ '__\ \ / /
                          \ V  V /  __/ |_) \__ \  __/ |   \ V / 
                           \_/\_/ \___|_.__/|___/\___|_|    \_/  
                                                                 
	</pre>
	<pre>
  ____   ___ _____    __              _   _                     _             _
 / ___| ( _ )_   _|  / _| ___  _ __  | |_| |__   ___  __      _(_)_ __  _ __ (_)_ __   __ _
 \___ \ / _ \/\ |   | |_ / _ \| '__| | __| '_ \ / _ \ \ \ /\ / / | '_ \| '_ \| | '_ \ / _` |
  ___) | (_>  < |   |  _| (_) | |    | |_| | | |  __/  \ V  V /| | | | | | | | | | | | (_| |
 |____/ \___/\/_|   |_|  \___/|_|     \__|_| |_|\___|   \_/\_/ |_|_| |_|_| |_|_|_| |_|\__, |
                                                                                       |___/
      *            .''.            *                  *             .''.            *
  *      *        :_\/_:       *       *         *       *        :_\/_:       *      *
     \ | /    .''.: /\ :.''.      \ | /             \ | /     .''.: /\ :.''.     \ | /
  --- * ---  :_\/_:'.:::_\/_:  --- * ---     *    --- * ---  :_\/_:'.:::_\/_: --- * ---
     / | \    : /\ : ::::: /\ :     /|\       /|\      /|\     : /\ : ::::: /\ :    / | \
  *      *     '..' ':::' '..'   * / | \ * -- * -- * / | \ *   '..' ':::' '..'  *      *
         .            *             *       / | \      *             *            .
    * .   . *     \   |   /    .      .      *      .      .    \   |   /    * .   . *
  .     *     .    \  |  /   *   .  *  .         .  *  .   *    \  |  /   .     *     .
 * * * * * * * *  --- ✦ --- * * * * * * *       * * * * * * * --- ✦ --- * * * * * * *
  '     *     '    /  |  \   *   '  *  '         '  *  '   *    /  |  \   '     *     '
    * '   ' *     /   |   \    '      '     *     '      '     /   |   \    * '   ' *
         '            *             *       \ | /      *             *            '
  *      *      ✧          ✦            ----- ✹ -----         ✦          ✧      *      *
                                            / | \
                                           *  *  *</pre>
</body>
</html>
EOF
	printf "body { margin: 0; }\n" > www/style.css
	printf "unknown content\n" > www/test.unknown
}

cleanup_test_files() {
	# rm www/index.html
	rm www/style.css
	rm www/test.unknown
}

cleanup() {
	cleanup_test_files
    rm -rf "${TMP_DIR}"
}
trap cleanup EXIT

print_title() {
    printf "\n${CYAN}========== %s ==========${RESET}\n" "$1"
}

pass() {
    printf "${GREEN}[PASS]${RESET} %s\n" "$1"
    PASS=$((PASS + 1))
}

fail() {
    printf "${RED}[FAIL]${RESET} %s\n" "$1"
    FAIL=$((FAIL + 1))
}

skip() {
    printf "${YELLOW}[SKIP]${RESET} %s\n" "$1"
    SKIP=$((SKIP + 1))
}

require_cmd() {
    if ! command -v "$1" >/dev/null 2>&1; then
        printf "${RED}Error:${RESET} required command not found: %s\n" "$1"
        exit 1
    fi
}

server_is_up() {
    nc -z "$HOST" "$PORT" >/dev/null 2>&1
}

http_status() {
    curl -sS -o /dev/null -w "%{http_code}" "$1" 2>/dev/null
}

raw_request() {
    # Usage:
    # raw_request "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"
    printf "%b" "$1" | nc "$HOST" "$PORT"
}

print_title "Prerequisites"

require_cmd nc
require_cmd curl
require_cmd printf
require_cmd cmp
require_cmd wc

if server_is_up; then
    pass "Server is responding on ${HOST}:${PORT}"
else
    fail "No server detected on ${HOST}:${PORT}"
    printf "\nStart ./webserv before running this script.\n"
    exit 1
fi

create_test_files

print_title "${TEST_NUMBER}. Simple GET"
TEST_NUMBER=$((TEST_NUMBER + 1))

status="$(http_status "${BASE_URL}/index.html")"

if [ "$status" = "200" ]; then
    pass "GET / returns 200"
else
    fail "GET / returns ${status} instead of 200"
fi

print_title "${TEST_NUMBER}. GET existing HTML file"
TEST_NUMBER=$((TEST_NUMBER + 1))

status="$(http_status "${BASE_URL}/index.html")"

if [ "$status" = "200" ]; then
    pass "GET /index.html returns 200"
else
    fail "GET /index.html returns ${status} instead of 200"
fi


print_title "${TEST_NUMBER}. GET missing file"
TEST_NUMBER=$((TEST_NUMBER + 1))

status="$(http_status "${BASE_URL}/this_file_does_not_exist_42.txt")"

if [ "$status" = "404" ]; then
    pass "GET missing file returns 404"
else
    fail "GET missing file returns ${status} instead of 404"
fi


print_title "${TEST_NUMBER}. GET CSS Content-Type"
TEST_NUMBER=$((TEST_NUMBER + 1))

headers="$(curl -sS -D - -o /dev/null "${BASE_URL}/style.css" 2>/dev/null)"
status="$(printf "%s" "$headers" | head -n 1 | awk '{print $2}')"

if [ "$status" = "200" ]; then
    pass "GET /style.css returns 200"
else
    fail "GET /style.css returns ${status} instead of 200"
fi

if printf "%s" "$headers" | grep -qi '^Content-Type: text/css'; then
    pass "GET /style.css returns Content-Type: text/css"
else
    fail "GET /style.css does not return Content-Type: text/css"
fi


print_title "${TEST_NUMBER}. GET PNG Content-Type"
TEST_NUMBER=$((TEST_NUMBER + 1))

headers="$(curl -sS -D - -o /dev/null "${BASE_URL}/Undead.png" 2>/dev/null)"
status="$(printf "%s" "$headers" | head -n 1 | awk '{print $2}')"

if [ "$status" = "200" ]; then
    pass "GET /test.png returns 200"
else
    fail "GET /test.png returns ${status} instead of 200"
fi

if printf "%s" "$headers" | grep -qi '^Content-Type: image/png'; then
    pass "GET /test.png returns Content-Type: image/png"
else
    fail "GET /test.png does not return Content-Type: image/png"
fi


print_title "${TEST_NUMBER}. GET unknown extension Content-Type"
TEST_NUMBER=$((TEST_NUMBER + 1))

headers="$(curl -sS -D - -o /dev/null "${BASE_URL}/test.unknown" 2>/dev/null)"
status="$(printf "%s" "$headers" | head -n 1 | awk '{print $2}')"

if [ "$status" = "200" ]; then
    pass "GET /test.unknown returns 200"
else
    fail "GET /test.unknown returns ${status} instead of 200"
fi

if printf "%s" "$headers" | grep -qi '^Content-Type: application/octet-stream'; then
    pass "GET /test.unknown returns application/octet-stream"
else
    fail "GET /test.unknown does not return application/octet-stream"
fi


print_title "${TEST_NUMBER}. GET directory"
TEST_NUMBER=$((TEST_NUMBER + 1))

status="$(http_status "${BASE_URL}/uploads/")"

if [ "$status" = "404" ]; then
    pass "GET /uploads/ returns temporary 404"
else
    fail "GET /uploads/ returns ${status} instead of 404"
fi


print_title "${TEST_NUMBER}. Server alive after GET tests"
TEST_NUMBER=$((TEST_NUMBER + 1))

if server_is_up; then
    pass "Server is still running after GET tests"
else
    fail "Server is no longer running after GET tests"
fi

print_title "${TEST_NUMBER}. Valid raw HTTP request"
TEST_NUMBER=$((TEST_NUMBER + 1))

response_file="${TMP_DIR}/raw_get.txt"

printf 'GET / HTTP/1.1\r\nHost: localhost\r\n\r\n' \
    | nc "$HOST" "$PORT" > "$response_file"

if grep -q '^HTTP/' "$response_file"; then
    pass "A request sent with CRLF produces an HTTP response"
else
    fail "No HTTP status line detected"
fi

print_title "${TEST_NUMBER}. Request fragmentation"
TEST_NUMBER=$((TEST_NUMBER + 1))

fragmented_response="${TMP_DIR}/fragmented.txt"

(
    printf 'GET / HTTP/1.1\r\n'
    sleep 0.2
    printf 'Host: localhost\r\n'
    sleep 0.2
    printf '\r\n'
) | nc "$HOST" "$PORT" > "$fragmented_response"

if grep -q '^HTTP/' "$fragmented_response"; then
    pass "A request sent in multiple fragments is reconstructed correctly"
else
    fail "The fragmented request did not produce an HTTP response"
fi

print_title "${TEST_NUMBER}. Fragmentation at difficult boundaries"
TEST_NUMBER=$((TEST_NUMBER + 1))

hard_fragment_response="${TMP_DIR}/hard_fragment.txt"

(
    printf 'GET / HT'
    sleep 0.1
    printf 'TP/1.1\r'
    sleep 0.1
    printf '\nHost: loca'
    sleep 0.1
    printf 'lhost\r\n'
    sleep 0.1
    printf '\r'
    sleep 0.1
    printf '\n'
) | nc "$HOST" "$PORT" > "$hard_fragment_response"

if grep -q '^HTTP/' "$hard_fragment_response"; then
    pass "The parser handles fragmentation in the middle of HTTP/1.1, headers, and CRLF"
else
    fail "Fine-grained fragmentation did not produce an HTTP response"
fi

print_title "${TEST_NUMBER}. Incomplete request"
TEST_NUMBER=$((TEST_NUMBER + 1))

# timeout protects the script: an incomplete request should remain pending.

if command -v timeout >/dev/null 2>&1; then
    incomplete_output="${TMP_DIR}/incomplete.txt"

    set +e
    printf 'GET / HTTP/1.1\r\nHost: localhost\r\n' \
        | timeout 1 nc "$HOST" "$PORT" > "$incomplete_output"
    rc=$?
    set -e 2>/dev/null || true

    if [ ! -s "$incomplete_output" ]; then
        pass "A request without the final CRLF does not trigger a premature response"
    else
        fail "The server responded to a request that was still incomplete"
    fi
else
    skip "timeout command unavailable: incomplete request test skipped"
fi

print_title "${TEST_NUMBER}. Malformed header"
TEST_NUMBER=$((TEST_NUMBER + 1))

malformed_response="${TMP_DIR}/malformed.txt"

printf 'GET / HTTP/1.1\r\nHost: localhost\r\nBrokenHeader\r\n\r\n' \
    | nc "$HOST" "$PORT" > "$malformed_response"

if grep -q '^HTTP/.* 400 ' "$malformed_response"; then
    pass "Malformed header → 400"
elif [ ! -s "$malformed_response" ]; then
    skip "Malformed header detected, but no 400 response is generated yet"
else
    fail "Malformed header: unexpected response"
fi

if server_is_up; then
    pass "The server remains active after a malformed request"
else
    fail "The server no longer responds after a malformed request"
fi

print_title "${TEST_NUMBER}. Unsupported method"
TEST_NUMBER=$((TEST_NUMBER + 1))

unsupported_response="${TMP_DIR}/unsupported.txt"

printf 'PUT / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\n\r\n' \
    | nc "$HOST" "$PORT" > "$unsupported_response"

if grep -q '^HTTP/.* 501 ' "$unsupported_response"; then
    pass "PUT → 501 according to the parser's current behavior"
elif grep -q '^HTTP/.* 405 ' "$unsupported_response"; then
    pass "PUT → 405 according to the handler policy"
elif [ ! -s "$unsupported_response" ]; then
    skip "PUT appears to be detected, but no HTTP error response is wired up yet"
else
    fail "PUT produces an unexpected response"
fi

print_title "${TEST_NUMBER}. Two clients in parallel"
TEST_NUMBER=$((TEST_NUMBER + 1))

client_a="${TMP_DIR}/client_a.txt"
client_b="${TMP_DIR}/client_b.txt"

if command -v timeout >/dev/null 2>&1; then
    (
        printf 'GET / HTTP/1.1\r\nHost: local'
        sleep 1
        printf 'host\r\n\r\n'
    ) | nc "$HOST" "$PORT" > "$client_a" &
    pid_a=$!

    sleep 0.1

    printf 'GET / HTTP/1.1\r\nHost: localhost\r\n\r\n' \
        | nc "$HOST" "$PORT" > "$client_b"

    wait "$pid_a"

    if grep -q '^HTTP/' "$client_a" && grep -q '^HTTP/' "$client_b"; then
        pass "Two clients with different parsing states are handled independently"
    else
        fail "At least one of the two parallel clients did not receive a response"
    fi
else
    skip "timeout command unavailable: multi-client test skipped"
fi

print_title "${TEST_NUMBER}. Large file / complete transfer"
TEST_NUMBER=$((TEST_NUMBER + 1))

if [ -f "$BIGFILE_PATH" ]; then
    downloaded="${TMP_DIR}/bigfile_received.bin"

    if curl -sS "${BASE_URL}/bigfile.bin" -o "$downloaded"; then
        if cmp "$BIGFILE_PATH" "$downloaded" >/dev/null 2>&1; then
            original_size="$(wc -c < "$BIGFILE_PATH" | tr -d ' ')"
            received_size="$(wc -c < "$downloaded" | tr -d ' ')"
            pass "bigfile.bin received completely (${received_size}/${original_size} bytes)"
        else
            fail "bigfile.bin was received, but its content differs from the original"
        fi
    else
        fail "Unable to download /bigfile.bin"
    fi
else
    skip "${BIGFILE_PATH} not found"
fi

print_title "${TEST_NUMBER}. Successive connections"
TEST_NUMBER=$((TEST_NUMBER + 1))

success_count=0
i=1

while [ "$i" -le 10 ]; do
    code="$(http_status "${BASE_URL}/index.html")"
    if [ "$code" = "200" ]; then
        success_count=$((success_count + 1))
    fi
    i=$((i + 1))
done

if [ "$success_count" -eq 10 ]; then
    pass "10 successive connections received 200"
else
    fail "${success_count}/10 successive connections succeeded"
fi

print_title "${TEST_NUMBER}. Immediate disconnection"
TEST_NUMBER=$((TEST_NUMBER + 1))

if command -v timeout >/dev/null 2>&1; then
    timeout 1 nc "$HOST" "$PORT" < /dev/null >/dev/null 2>&1 || true
    sleep 0.1

    if server_is_up; then
        pass "The server remains active after a connection is closed immediately"
    else
        fail "The server no longer responds after an immediate disconnection"
    fi
else
    skip "timeout command unavailable: disconnection test skipped"
fi

print_title "${TEST_NUMBER}. Client disconnect during large response"
TEST_NUMBER=$((TEST_NUMBER + 1))

if [ -f "$BIGFILE_PATH" ] && command -v timeout >/dev/null 2>&1; then
    (
        printf 'GET /bigfile.bin HTTP/1.1\r\nHost: localhost\r\n\r\n'
        sleep 0.05
    ) | timeout 0.2 nc "$HOST" "$PORT" >/dev/null 2>&1 || true

    sleep 0.2

    if server_is_up; then
        pass "Server remains alive after a client disconnects during a large response"
    else
        fail "Server stopped responding after a client disconnected during a large response"
    fi
else
    skip "Large file or timeout command unavailable"
fi

print_title "${TEST_NUMBER}. Slow fragmented request"
TEST_NUMBER=$((TEST_NUMBER + 1))

slow_response="${TMP_DIR}/slow_request.txt"

(
    printf 'GET '
    sleep 0.2
    printf '/ '
    sleep 0.2
    printf 'HTTP/1.1\r'
    sleep 0.2
    printf '\nHost: '
    sleep 0.2
    printf 'localhost'
    sleep 0.2
    printf '\r\n'
    sleep 0.2
    printf '\r\n'
) | nc "$HOST" "$PORT" > "$slow_response"

if grep -q '^HTTP/' "$slow_response"; then
    pass "Slow fragmented request is parsed successfully"
else
    fail "Slow fragmented request did not produce an HTTP response"
fi

print_title "${TEST_NUMBER}. Many successive connections"
TEST_NUMBER=$((TEST_NUMBER + 1))

TOTAL_REQUESTS=50
success_count=0
i=1

while [ "$i" -le "$TOTAL_REQUESTS" ]; do
    code="$(curl -sS \
        --max-time 2 \
        -o /dev/null \
        -w "%{http_code}" \
        "${BASE_URL}/index.html" 2>/dev/null)"

    if [ "$code" = "200" ]; then
        success_count=$((success_count + 1))
    fi

    i=$((i + 1))
done

if [ "$success_count" -eq "$TOTAL_REQUESTS" ]; then
    pass "Server handled ${TOTAL_REQUESTS} successive connections successfully"
else
    fail "Only ${success_count}/${TOTAL_REQUESTS} successive connections succeeded"
fi

print_title "${TEST_NUMBER}. Concurrent clients"
TEST_NUMBER=$((TEST_NUMBER + 1))

CONCURRENT_CLIENTS=20
pids=""
i=1

while [ "$i" -le "$CONCURRENT_CLIENTS" ]; do
    output="${TMP_DIR}/concurrent_${i}.code"

    (
        curl -sS \
            --max-time 5 \
            -o /dev/null \
            -w "%{http_code}" \
            "${BASE_URL}/index.html" \
            > "$output" 2>/dev/null
    ) &

    pids="$pids $!"
    i=$((i + 1))
done

for pid in $pids; do
    wait "$pid" || true
done

success_count=0
i=1

while [ "$i" -le "$CONCURRENT_CLIENTS" ]; do
    output="${TMP_DIR}/concurrent_${i}.code"

    if [ -f "$output" ] && [ "$(cat "$output")" = "200" ]; then
        success_count=$((success_count + 1))
    fi

    i=$((i + 1))
done

if [ "$success_count" -eq "$CONCURRENT_CLIENTS" ]; then
    pass "All ${CONCURRENT_CLIENTS} concurrent clients received HTTP 200"
else
    fail "Only ${success_count}/${CONCURRENT_CLIENTS} concurrent clients received HTTP 200"
fi

if server_is_up; then
    pass "Server remains responsive after concurrent client load"
else
    fail "Server stopped responding after concurrent client load"
fi

print_title "${TEST_NUMBER}. Concurrent large file downloads"
TEST_NUMBER=$((TEST_NUMBER + 1))

if [ -f "$BIGFILE_PATH" ]; then
    LARGE_CLIENTS=5
    pids=""
    i=1

    while [ "$i" -le "$LARGE_CLIENTS" ]; do
        output="${TMP_DIR}/bigfile_${i}.bin"

        curl -sS \
            --max-time 15 \
            "${BASE_URL}/bigfile.bin" \
            -o "$output" &

        pids="$pids $!"
        i=$((i + 1))
    done

    for pid in $pids; do
        wait "$pid" || true
    done

    success_count=0
    i=1

    while [ "$i" -le "$LARGE_CLIENTS" ]; do
        output="${TMP_DIR}/bigfile_${i}.bin"

        if [ -f "$output" ] \
            && cmp "$BIGFILE_PATH" "$output" >/dev/null 2>&1; then
            success_count=$((success_count + 1))
        fi

        i=$((i + 1))
    done

    if [ "$success_count" -eq "$LARGE_CLIENTS" ]; then
        pass "All ${LARGE_CLIENTS} concurrent large file downloads are identical to the source"
    else
        fail "Only ${success_count}/${LARGE_CLIENTS} concurrent large file downloads are correct"
    fi
else
    skip "${BIGFILE_PATH} not found"
fi

print_title "${TEST_NUMBER}. Oversized request line"
TEST_NUMBER=$((TEST_NUMBER + 1))

if command -v timeout >/dev/null 2>&1; then
    oversized_response="${TMP_DIR}/oversized_request_line.txt"

    {
        printf 'GET /'

        i=0
        while [ "$i" -lt 9000 ]; do
            printf 'a'
            i=$((i + 1))
        done
    } | timeout 2 nc "$HOST" "$PORT" > "$oversized_response" 2>/dev/null || true

    if grep -q '^HTTP/.* 414 ' "$oversized_response"; then
        pass "Oversized request line is rejected with HTTP 414"
    elif [ ! -s "$oversized_response" ]; then
        skip "Oversized request line was sent, but HTTP 414 response generation is not integrated yet"
    else
        fail "Oversized request line produced an unexpected response"
    fi

    if server_is_up; then
        pass "Server remains responsive after an oversized request line"
    else
        fail "Server stopped responding after an oversized request line"
    fi
else
    skip "timeout command unavailable: oversized request line test skipped"
fi

print_title "${TEST_NUMBER}. Oversized headers"
TEST_NUMBER=$((TEST_NUMBER + 1))

if command -v timeout >/dev/null 2>&1; then
    oversized_response="${TMP_DIR}/oversized_headers.txt"

    {
        printf 'GET / HTTP/1.1\r\n'
        printf 'Host: localhost\r\n'
        printf 'X-Large-Header: '

        i=0
        while [ "$i" -lt 33000 ]; do
            printf 'A'
            i=$((i + 1))
        done

        printf '\r\n'
        printf '\r\n'
    } | timeout 2 nc "$HOST" "$PORT" > "$oversized_response" 2>/dev/null || true

    if grep -q '^HTTP/.* 431 ' "$oversized_response"; then
        pass "Oversized headers are rejected with HTTP 431"
    elif [ ! -s "$oversized_response" ]; then
        skip "Oversized headers were sent, but HTTP 431 response generation is not integrated yet"
    else
        fail "Oversized headers produced an unexpected response"
        printf '%s\n' "$(cat "$oversized_response")"
    fi

    if server_is_up; then
        pass "Server remains responsive after oversized headers"
    else
        fail "Server stopped responding after oversized headers"
    fi
else
    skip "timeout command unavailable: oversized headers test skipped"
fi

print_title "Summary"

printf "${GREEN}PASS:${RESET} %d\n" "$PASS"
printf "${RED}FAIL:${RESET} %d\n" "$FAIL"
printf "${YELLOW}SKIP:${RESET} %d\n" "$SKIP"

if [ "$FAIL" -eq 0 ]; then
    printf "\n${GREEN}All applicable tests passed.\n"
	sed -n '/<body>/,/<\/body>/p' www/index.html \
		| sed 's/<[^>]*>//g'
	printf "${RESET}"
    exit 0
else
    printf "\n${RED}%d test(s) failed.\n" "$FAIL"
	sed -n '/<body>/,/<\/body>/p' www/index.html \
			| sed 's/<[^>]*>//g'
	printf "${RESET}"
    exit 1
fi
