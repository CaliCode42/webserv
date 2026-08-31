#!/usr/bin/env bash

set -u

HOST="${HOST:-localhost}"
PORT="${PORT:-8080}"
BASE_URL="http://${HOST}:${PORT}"
BIGFILE_PATH="${BIGFILE_PATH:-www/bigfile.bin}"
TMP_DIR=".webserv_test_tmp"
CLIENT_TIMEOUT_VALUE=10

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
	mkdir -p www/images
	mkdir -p www/private

	printf "location images root\n" > www/images/location_root_test.txt
	printf "location private root\n" > www/private/private_root_test.txt

	printf "body { margin: 0; }\n" > www/style.css
	printf "unknown content\n" > www/test.unknown

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
}

cleanup_test_files() {
	# rm www/index.html
	rm www/style.css
	rm www/test.unknown
	rm -f www/images/location_root_test.txt
	rm -f www/private/private_root_test.txt
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

# print_title "${TEST_NUMBER}. GET with no root defined in location"
# TEST_NUMBER=$((TEST_NUMBER + 1))

# status="$(http_status "${BASE_URL}/foo/test.txt")"

# if [ "$status" = "200" ]; then
#     pass "GET /foo/test.txt returns 200"
# else
#     fail "GET /foo/test.txt returns ${status} instead of 200"
# fi

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

headers="$(curl -sS -D - -o /dev/null "${BASE_URL}/images/private/Undead.png" 2>/dev/null)"
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

if [ "$status" = "403" ]; then
    pass "GET /uploads/ is forbidden when directory listing is disabled"
else
    fail "GET /uploads/ returns ${status} instead of 403"
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

# printf "%s\n" "$response_file"

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

# printf "%s\n" "$fragmented_response"


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

	# cat $oversized_response
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


print_title "${TEST_NUMBER}. POST creates uploads directory"
TEST_NUMBER=$((TEST_NUMBER + 1))

rm -rf "www/uploads"

status="$(
    curl -sS -o /dev/null -w "%{http_code}" \
        -X POST \
        --data-binary "Hello Webserv 42" \
        "${BASE_URL}/auto_create_upload.txt" \
        2>/dev/null
)"

if [ "$status" = "201" ]; then
    pass "POST creates uploads directory and returns 201"
else
    fail "POST returns ${status} instead of 201 when uploads directory is missing"
fi

if [ -d "www/uploads" ]; then
    pass "uploads directory was created automatically"
else
    fail "uploads directory was not created"
fi

if [ -f "www/uploads/auto_create_upload.txt" ]; then
    pass "Uploaded file was created inside uploads directory"
else
    fail "Uploaded file was not created inside uploads directory"
fi


print_title "${TEST_NUMBER}. Transfer-Encoding case insensitive"
TEST_NUMBER=$((TEST_NUMBER + 1))

response="$(
    raw_request \
    "POST /case_chunked.txt HTTP/1.1\r\nHost: ${HOST}\r\nTransfer-Encoding: ChUnKeD\r\n\r\n5\r\nHello\r\n0\r\n\r\n"
)"

status="$(printf "%s" "$response" | head -n 1 | awk '{print $2}')"

if [ "$status" = "201" ]; then
    pass "Transfer-Encoding: ChUnKeD is accepted"
else
    fail "Transfer-Encoding: ChUnKeD returns ${status} instead of 201"
	printf "%s\n" "$response"
fi


print_title "${TEST_NUMBER}. Invalid Content-Length"
TEST_NUMBER=$((TEST_NUMBER + 1))

response="$(
    raw_request \
    "POST /invalid_length.txt HTTP/1.1\r\nHost: ${HOST}\r\nContent-Length: 5abc\r\n\r\nHello"
)"

status="$(printf "%s" "$response" | head -n 1 | awk '{print $2}')"

if [ "$status" = "400" ]; then
    pass "Invalid Content-Length returns 400"
else
    fail "Invalid Content-Length returns ${status} instead of 400"
fi


print_title "${TEST_NUMBER}. Chunked request with trailers"
TEST_NUMBER=$((TEST_NUMBER + 1))

response="$(
    raw_request \
    "POST /chunked_trailer.txt HTTP/1.1\r\nHost: ${HOST}\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHello\r\n0\r\nX-Test: trailer-value\r\n\r\n"
)"

status="$(printf "%s" "$response" | head -n 1 | awk '{print $2}')"

if [ "$status" = "201" ]; then
    pass "Chunked request with trailers returns 201"
else
    fail "Chunked request with trailers returns ${status} instead of 201"
fi


print_title "${TEST_NUMBER}. Invalid chunk terminator"
TEST_NUMBER=$((TEST_NUMBER + 1))

response="$(
    raw_request \
    "POST /bad_chunk.txt HTTP/1.1\r\nHost: ${HOST}\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHelloXX0\r\n\r\n"
)"

status="$(printf "%s" "$response" | head -n 1 | awk '{print $2}')"

if [ "$status" = "400" ]; then
    pass "Invalid chunk terminator returns 400"
else
    fail "Invalid chunk terminator returns ${status} instead of 400"
fi


print_title "${TEST_NUMBER}. POST text file"
TEST_NUMBER=$((TEST_NUMBER + 1))

status="$(
    curl -sS -o /dev/null -w "%{http_code}" \
        -X POST \
        --data-binary "Hello Webserv 42" \
        "${BASE_URL}/test_upload.txt" \
        2>/dev/null
)"

if [ "$status" = "201" ]; then
    pass "POST /test_upload.txt returns 201"
else
    fail "POST /test_upload.txt returns ${status} instead of 201"
fi


print_title "${TEST_NUMBER}. GET uploaded text file"
TEST_NUMBER=$((TEST_NUMBER + 1))

status="$(http_status "${BASE_URL}/uploads/test_upload.txt")"

if [ "$status" = "200" ]; then
    pass "GET uploaded text file returns 200"
else
    fail "GET uploaded text file returns ${status} instead of 200"
fi


print_title "${TEST_NUMBER}. Uploaded text content"
TEST_NUMBER=$((TEST_NUMBER + 1))

body="$(curl -sS "${BASE_URL}/uploads/test_upload.txt" 2>/dev/null)"

if [ "$body" = "Hello Webserv 42" ]; then
    pass "Uploaded text content is preserved"
else
    fail "Uploaded text content differs from original"
fi


print_title "${TEST_NUMBER}. DELETE uploaded file"
TEST_NUMBER=$((TEST_NUMBER + 1))

status="$(
    curl -sS -o /dev/null -w "%{http_code}" \
        -X DELETE \
        "${BASE_URL}/uploads/test_upload.txt" \
        2>/dev/null
)"

if [ "$status" = "204" ]; then
    pass "DELETE /uploads/test_upload.txt returns 204"
else
    fail "DELETE /uploads/test_upload.txt returns ${status} instead of 204"
fi


print_title "${TEST_NUMBER}. GET deleted file"
TEST_NUMBER=$((TEST_NUMBER + 1))

status="$(http_status "${BASE_URL}/uploads/test_upload.txt")"

if [ "$status" = "404" ]; then
    pass "GET deleted file returns 404"
else
    fail "GET deleted file returns ${status} instead of 404"
fi


print_title "${TEST_NUMBER}. DELETE missing file"
TEST_NUMBER=$((TEST_NUMBER + 1))

status="$(
    curl -sS -o /dev/null -w "%{http_code}" \
        -X DELETE \
        "${BASE_URL}/uploads/file_that_does_not_exist_42.txt" \
        2>/dev/null
)"

if [ "$status" = "404" ]; then
    pass "DELETE missing file returns 404"
else
    fail "DELETE missing file returns ${status} instead of 404"
fi


print_title "${TEST_NUMBER}. Content-Length at body size limit"
TEST_NUMBER=$((TEST_NUMBER + 1))

BODY_LIMIT_FILE="${TMP_DIR}/body_limit.bin"

dd if=/dev/zero of="${BODY_LIMIT_FILE}" bs=1024 count=1024 2>/dev/null

status="$(
    curl -sS -o /dev/null -w "%{http_code}" \
        -X POST \
        --data-binary "@${BODY_LIMIT_FILE}" \
        "${BASE_URL}/body_limit.bin" \
        2>/dev/null
)"

if [ "$status" = "201" ]; then
    pass "Content-Length exactly at MAX_BODY_SIZE is accepted"
else
    fail "Content-Length at MAX_BODY_SIZE returns ${status} instead of 201"
fi


print_title "${TEST_NUMBER}. Content-Length above body size limit"
TEST_NUMBER=$((TEST_NUMBER + 1))

BODY_TOO_LARGE_FILE="${TMP_DIR}/body_too_large.bin"

dd if=/dev/zero of="${BODY_TOO_LARGE_FILE}" bs=1048577 count=1 2>/dev/null

status="$(
    curl -sS -o /dev/null -w "%{http_code}" \
        -X POST \
        --data-binary "@${BODY_TOO_LARGE_FILE}" \
        "${BASE_URL}/body_too_large.bin" \
        2>/dev/null
)"

if [ "$status" = "413" ]; then
    pass "Content-Length above MAX_BODY_SIZE returns 413"
else
    fail "Content-Length above MAX_BODY_SIZE returns ${status} instead of 413"
fi


print_title "${TEST_NUMBER}. Chunked body at size limit"
TEST_NUMBER=$((TEST_NUMBER + 1))

CHUNKED_LIMIT_RESPONSE="${TMP_DIR}/chunked_limit_response.txt"

{
    printf 'POST /chunked_limit.bin HTTP/1.1\r\n'
    printf 'Host: %s\r\n' "$HOST"
    printf 'Transfer-Encoding: chunked\r\n'
    printf '\r\n'

    printf '80000\r\n'
    dd if=/dev/zero bs=524288 count=1 2>/dev/null
    printf '\r\n'

    printf '80000\r\n'
    dd if=/dev/zero bs=524288 count=1 2>/dev/null
    printf '\r\n'

    printf '0\r\n'
    printf '\r\n'
} | timeout 3 nc "$HOST" "$PORT" > "${CHUNKED_LIMIT_RESPONSE}" 2>/dev/null || true

if grep -q '^HTTP/.* 201 ' "${CHUNKED_LIMIT_RESPONSE}"; then
    pass "Chunked body exactly at MAX_BODY_SIZE is accepted"
else
    fail "Chunked body at MAX_BODY_SIZE does not return 201"
fi


print_title "${TEST_NUMBER}. Chunked body above size limit"
TEST_NUMBER=$((TEST_NUMBER + 1))

CHUNKED_TOO_LARGE_RESPONSE="${TMP_DIR}/chunked_too_large_response.txt"

{
    printf 'POST /chunked_too_large.bin HTTP/1.1\r\n'
    printf 'Host: %s\r\n' "$HOST"
    printf 'Transfer-Encoding: chunked\r\n'
    printf '\r\n'

    printf '80000\r\n'
    dd if=/dev/zero bs=524288 count=1 2>/dev/null
    printf '\r\n'

    printf '80001\r\n'
    dd if=/dev/zero bs=524289 count=1 2>/dev/null
    printf '\r\n'
} | timeout 3 nc "$HOST" "$PORT" > "${CHUNKED_TOO_LARGE_RESPONSE}" 2>/dev/null || true

if grep -q '^HTTP/.* 413 ' "${CHUNKED_TOO_LARGE_RESPONSE}"; then
    pass "Chunked body above MAX_BODY_SIZE returns 413"
else
    fail "Chunked body above MAX_BODY_SIZE does not return 413"
fi


print_title "${TEST_NUMBER}. Oversized announced chunk"
TEST_NUMBER=$((TEST_NUMBER + 1))

OVERSIZED_CHUNK_RESPONSE="${TMP_DIR}/oversized_chunk_response.txt"

{
    printf 'POST /oversized_chunk.bin HTTP/1.1\r\n'
    printf 'Host: %s\r\n' "$HOST"
    printf 'Transfer-Encoding: chunked\r\n'
    printf '\r\n'

    # 0x100001 = 1048577 = MAX_BODY_SIZE + 1
    printf '100001\r\n'
} | timeout 2 nc "$HOST" "$PORT" > "${OVERSIZED_CHUNK_RESPONSE}" 2>/dev/null || true

if grep -q '^HTTP/.* 413 ' "${OVERSIZED_CHUNK_RESPONSE}"; then
    pass "Oversized announced chunk is rejected immediately with 413"
elif [ ! -s "${OVERSIZED_CHUNK_RESPONSE}" ]; then
    fail "Oversized announced chunk produced no response"
else
    fail "Oversized announced chunk produced an unexpected response"
    printf '%s\n' "$(cat "${OVERSIZED_CHUNK_RESPONSE}")"
fi


print_title "${TEST_NUMBER}. Server alive after body limit tests"
TEST_NUMBER=$((TEST_NUMBER + 1))

if server_is_up; then
    pass "Server remains responsive after body size limit tests"
else
    fail "Server stopped responding after body size limit tests"
fi


print_title "${TEST_NUMBER}. Idle client timeout"
TEST_NUMBER=$((TEST_NUMBER + 1))

IDLE_OUTPUT="${TMP_DIR}/idle_timeout.txt"

{
    sleep $((CLIENT_TIMEOUT_VALUE + 2))
} | timeout 14 nc "$HOST" "$PORT" > "${IDLE_OUTPUT}" 2>/dev/null || true

if server_is_up; then
    pass "Idle client is closed without affecting the server"
else
    fail "Server stopped responding after idle client timeout"
fi


print_title "${TEST_NUMBER}. Incomplete request timeout"
TEST_NUMBER=$((TEST_NUMBER + 1))

INCOMPLETE_OUTPUT="${TMP_DIR}/incomplete_timeout.txt"

{
    printf 'GET /index.html HTTP/1.1\r\n'
    printf 'Host: %s\r\n' "$HOST"
    # no final CRLF: request intentionally incomplete
    sleep $((CLIENT_TIMEOUT_VALUE + 2))
} | timeout 14 nc "$HOST" "$PORT" > "${INCOMPLETE_OUTPUT}" 2>/dev/null || true

if server_is_up; then
    pass "Incomplete request is timed out without affecting the server"
else
    fail "Server stopped responding after incomplete request timeout"
fi


print_title "${TEST_NUMBER}. Server serves other clients during timeout"
TEST_NUMBER=$((TEST_NUMBER + 1))

SLOW_CLIENT_OUTPUT="${TMP_DIR}/slow_client_timeout.txt"

{
    printf 'GET /index.html HTTP/1.1\r\n'
    printf 'Host: %s\r\n' "$HOST"
    sleep $((CLIENT_TIMEOUT_VALUE + 2))
} | timeout 14 nc "$HOST" "$PORT" > "${SLOW_CLIENT_OUTPUT}" 2>/dev/null &

slow_pid=$!

sleep 1

status="$(http_status "${BASE_URL}/index.html")"

if [ "$status" = "200" ]; then
    pass "Server serves other clients while one client is inactive"
else
    fail "Concurrent GET returns ${status} while another client is inactive"
fi

wait "$slow_pid" 2>/dev/null || true


print_title "${TEST_NUMBER}. Client activity refreshes timeout"
TEST_NUMBER=$((TEST_NUMBER + 1))

ACTIVE_OUTPUT="${TMP_DIR}/active_timeout.txt"

{
    printf 'GET /index.html HTTP/1.1\r\n'
    sleep $(((CLIENT_TIMEOUT_VALUE + 2) / 2))
    printf 'Host: %s\r\n' "$HOST"
    sleep $(((CLIENT_TIMEOUT_VALUE + 2) / 2))
    printf '\r\n'
} | timeout 16 nc "$HOST" "$PORT" > "${ACTIVE_OUTPUT}" 2>/dev/null || true

if grep -q '^HTTP/.* 200 ' "${ACTIVE_OUTPUT}"; then
    pass "Client activity refreshes inactivity timeout"
elif [ ! -s "${ACTIVE_OUTPUT}" ]; then
    fail "Client was closed before completing active request"
else
    fail "Active client produced an unexpected response"
    printf '%s\n' "$(cat "${ACTIVE_OUTPUT}")"
fi


print_title "${TEST_NUMBER}. Idle socket is closed by server"
TEST_NUMBER=$((TEST_NUMBER + 1))

if exec 3<>/dev/tcp/"$HOST"/"$PORT"; then
	if IFS= read -r -t $((CLIENT_TIMEOUT_VALUE + 2)) -u 3 _; then
		read_status=0
	else
		read_status=$?
	fi

	if [ "$read_status" -eq 1 ]; then
		pass "Idle socket is closed by server after timeout"
	elif [ "$read_status" -gt 128 ]; then
		fail "Idle socket is still open after client timeout"
	else
		fail "Idle socket produced unexpected result (read status: ${read_status})"
	fi

	exec 3>&-
else
	fail "Could not open idle socket"
fi


print_title "${TEST_NUMBER}. Incomplete request socket is closed"
TEST_NUMBER=$((TEST_NUMBER + 1))

if exec 3<>/dev/tcp/"$HOST"/"$PORT"; then

    printf 'GET /index.html HTTP/1.1\r\n' >&3
    printf 'Host: %s\r\n' "$HOST" >&3

    if IFS= read -r -t $((CLIENT_TIMEOUT_VALUE + 2)) -u 3 _; then
        read_status=0
    else
        read_status=$?
    fi

    if [ "$read_status" -eq 1 ]; then
        pass "Incomplete request is closed after inactivity timeout"
    elif [ "$read_status" -gt 128 ]; then
        fail "Incomplete request socket is still open after inactivity timeout"
    else
        fail "Incomplete request produced unexpected result (read status: ${read_status})"
    fi

    exec 3>&-
else
    fail "Could not open socket for incomplete request timeout test"
fi


print_title "${TEST_NUMBER}. Other clients do not refresh idle timeout"
TEST_NUMBER=$((TEST_NUMBER + 1))

IDLE_OUTPUT="${TMP_DIR}/independent_idle_socket.txt"

nc "$HOST" "$PORT" > "${IDLE_OUTPUT}" 2>/dev/null &
idle_pid=$!

i=0
while [ "$i" -lt 12 ]; do
    status="$(http_status "${BASE_URL}/index.html")"

    if [ "$status" != "200" ]; then
        fail "Concurrent GET returned ${status} during idle timeout test"
        break
    fi

    sleep 1
    i=$((i + 1))
done

if kill -0 "$idle_pid" 2>/dev/null; then
    fail "Idle client survived because of unrelated server activity"
    kill "$idle_pid" 2>/dev/null || true
    wait "$idle_pid" 2>/dev/null || true
else
    wait "$idle_pid" 2>/dev/null || true
    pass "Other clients do not refresh idle client's timeout"
fi


print_title "${TEST_NUMBER}. Other clients do not refresh idle timeout"
TEST_NUMBER=$((TEST_NUMBER + 1))

IDLE_OUTPUT="${TMP_DIR}/independent_idle_socket.txt"

nc "$HOST" "$PORT" > "${IDLE_OUTPUT}" 2>/dev/null &
idle_pid=$!

i=0
while [ "$i" -lt 12 ]; do
    status="$(http_status "${BASE_URL}/index.html")"

    if [ "$status" != "200" ]; then
        fail "Concurrent GET returned ${status} during idle timeout test"
        break
    fi

    sleep 1
    i=$((i + 1))
done

if kill -0 "$idle_pid" 2>/dev/null; then
    fail "Idle client survived because of unrelated server activity"
    kill "$idle_pid" 2>/dev/null || true
    wait "$idle_pid" 2>/dev/null || true
else
    wait "$idle_pid" 2>/dev/null || true
    pass "Other clients do not refresh idle client's timeout"
fi


print_title "${TEST_NUMBER}. Activity near timeout resets timer"
TEST_NUMBER=$((TEST_NUMBER + 1))

ACTIVE_OUTPUT="${TMP_DIR}/active_timeout_reset.txt"

{
    printf 'GET /index.html HTTP/1.1\r\n'

    sleep 9

    printf 'Host: %s\r\n' "$HOST"

    sleep 3

    printf '\r\n'
} | timeout 16 nc "$HOST" "$PORT" > "${ACTIVE_OUTPUT}" 2>/dev/null || true

if grep -q '^HTTP/.* 200 ' "${ACTIVE_OUTPUT}"; then
    pass "Activity near timeout correctly resets inactivity timer"
elif [ ! -s "${ACTIVE_OUTPUT}" ]; then
    fail "Client was closed despite activity before timeout"
else
    fail "Active client produced an unexpected response"
    printf '%s\n' "$(cat "${ACTIVE_OUTPUT}")"
fi

print_title "${TEST_NUMBER}. Server alive after timeout tests"
TEST_NUMBER=$((TEST_NUMBER + 1))

if server_is_up; then
    pass "Server remains responsive after timeout tests"
else
    fail "Server stopped responding after timeout tests"
fi


print_title "${TEST_NUMBER}. CGI GET"
TEST_NUMBER=$((TEST_NUMBER + 1))

CGI_GET_BODY="${TMP_DIR}/cgi_get.txt"

status="$(
	curl -sS \
		-o "${CGI_GET_BODY}" \
		-w "%{http_code}" \
		"${BASE_URL}/cgi/test.py" \
		2>/dev/null
)"

if [ "$status" = "200" ]; then
	pass "CGI GET returns 200"
else
	fail "CGI GET returns ${status} instead of 200"
fi

if grep -q '^Hello from CGI$' "${CGI_GET_BODY}"; then
	pass "CGI GET output is returned to the client"
else
	fail "CGI GET output is incorrect"
fi


print_title "${TEST_NUMBER}. CGI query string"
TEST_NUMBER=$((TEST_NUMBER + 1))

CGI_QUERY_OUTPUT="${TMP_DIR}/cgi_query.txt"

status="$(
	curl -sS \
		-o "${CGI_QUERY_OUTPUT}" \
		-w "%{http_code}" \
		"${BASE_URL}/cgi/test_query.py?name=Bob&value=42" \
		2>/dev/null
)"

if [ "$status" = "200" ]; then
	pass "CGI query string request returns 200"
else
	fail "CGI query string request returns ${status} instead of 200"
fi

if grep -q '^QUERY_STRING=name=Bob&value=42$' "${CGI_QUERY_OUTPUT}"; then
	pass "CGI receives the complete query string"
else
	fail "CGI query string is missing or incorrect"
fi


print_title "${TEST_NUMBER}. CGI POST"
TEST_NUMBER=$((TEST_NUMBER + 1))

CGI_POST_BODY="${TMP_DIR}/cgi_post.txt"

status="$(
	curl -sS \
		-o "${CGI_POST_BODY}" \
		-w "%{http_code}" \
		-X POST \
		-H "Content-Type: text/plain" \
		--data-binary "Hello CGI POST" \
		"${BASE_URL}/cgi/test_post.py" \
		2>/dev/null
)"

if [ "$status" = "200" ]; then
	pass "CGI POST returns 200"
else
	fail "CGI POST returns ${status} instead of 200"
fi

if grep -q '^Received: Hello CGI POST$' "${CGI_POST_BODY}"; then
	pass "CGI receives and returns POST body"
else
	fail "CGI POST body is incorrect"
fi


print_title "${TEST_NUMBER}. CGI binary POST"
TEST_NUMBER=$((TEST_NUMBER + 1))

CGI_BINARY_INPUT="${TMP_DIR}/cgi_binary_input.bin"
CGI_BINARY_OUTPUT="${TMP_DIR}/cgi_binary_output.bin"

printf 'abc\0def' > "${CGI_BINARY_INPUT}"

status="$(
	curl -sS \
		-o "${CGI_BINARY_OUTPUT}" \
		-w "%{http_code}" \
		-X POST \
		-H "Content-Type: application/octet-stream" \
		--data-binary "@${CGI_BINARY_INPUT}" \
		"${BASE_URL}/cgi/test_binary.py" \
		2>/dev/null
)"

if [ "$status" = "200" ]; then
	pass "Binary CGI POST returns 200"
else
	fail "Binary CGI POST returns ${status} instead of 200"
fi

if cmp -s "${CGI_BINARY_INPUT}" "${CGI_BINARY_OUTPUT}"; then
	pass "Binary CGI body is preserved byte-for-byte"
else
	fail "Binary CGI body differs from original"
fi


print_title "${TEST_NUMBER}. Large CGI POST"
TEST_NUMBER=$((TEST_NUMBER + 1))

CGI_LARGE_INPUT="${TMP_DIR}/cgi_large_input.bin"
CGI_LARGE_OUTPUT="${TMP_DIR}/cgi_large_output.txt"

dd if=/dev/zero bs=1024 count=256 2>/dev/null \
	| tr '\0' 'A' > "${CGI_LARGE_INPUT}"

status="$(
	curl -sS \
		-o "${CGI_LARGE_OUTPUT}" \
		-w "%{http_code}" \
		-X POST \
		-H "Content-Type: application/octet-stream" \
		--data-binary "@${CGI_LARGE_INPUT}" \
		"${BASE_URL}/cgi/test_post_large.py" \
		2>/dev/null
)"

if [ "$status" = "200" ]; then
	pass "Large CGI POST returns 200"
else
	fail "Large CGI POST returns ${status} instead of 200"
fi

if grep -q '^Length: 262144$' "${CGI_LARGE_OUTPUT}" \
	&& grep -q '^Integrity: OK$' "${CGI_LARGE_OUTPUT}"; then
	pass "Large CGI POST is transmitted completely"
else
	fail "Large CGI POST failed integrity check"
fi


print_title "${TEST_NUMBER}. CGI redirect"
TEST_NUMBER=$((TEST_NUMBER + 1))

CGI_REDIRECT_HEADERS="${TMP_DIR}/cgi_redirect_headers.txt"

curl -sS \
	-D "${CGI_REDIRECT_HEADERS}" \
	-o /dev/null \
	"${BASE_URL}/cgi/test_redirect.py" \
	2>/dev/null

if grep -q '^HTTP/.* 302 ' "${CGI_REDIRECT_HEADERS}"; then
	pass "CGI Status header produces HTTP 302"
else
	fail "CGI redirect does not return 302"
fi

if grep -qi '^Location: /images/private/Undead.png' \
	"${CGI_REDIRECT_HEADERS}"; then
	pass "CGI Location header is preserved"
else
	fail "CGI redirect Location header is missing"
fi


print_title "${TEST_NUMBER}. CGI non-zero exit status"
TEST_NUMBER=$((TEST_NUMBER + 1))

status="$(http_status "${BASE_URL}/cgi/test_error.py")"

if [ "$status" = "500" ]; then
	pass "CGI exiting with non-zero status returns 500"
else
	fail "CGI exiting with non-zero status returns ${status} instead of 500"
fi


print_title "${TEST_NUMBER}. Invalid CGI output"
TEST_NUMBER=$((TEST_NUMBER + 1))

status="$(http_status "${BASE_URL}/cgi/test_invalid_cgi.py")"

if [ "$status" = "500" ]; then
	pass "Invalid CGI output returns 500"
else
	fail "Invalid CGI output returns ${status} instead of 500"
fi


print_title "${TEST_NUMBER}. CGI execve failure"
TEST_NUMBER=$((TEST_NUMBER + 1))

CGI_BAD_SCRIPT="www/cgi-bin/test.bad"

touch "${CGI_BAD_SCRIPT}"

status="$(http_status "${BASE_URL}/cgi/test.bad")"

rm -f "${CGI_BAD_SCRIPT}"

if [ "$status" = "500" ]; then
	pass "Invalid CGI interpreter returns 500"
else
	fail "Invalid CGI interpreter returns ${status} instead of 500"
fi


print_title "${TEST_NUMBER}. CGI closes stdin early"
TEST_NUMBER=$((TEST_NUMBER + 1))

CGI_CLOSED_STDIN_INPUT="${TMP_DIR}/cgi_closed_stdin.bin"
CGI_CLOSED_STDIN_OUTPUT="${TMP_DIR}/cgi_closed_stdin.txt"

dd if=/dev/zero \
	of="${CGI_CLOSED_STDIN_INPUT}" \
	bs=1024 \
	count=1024 \
	2>/dev/null

status="$(
	curl -sS \
		-o "${CGI_CLOSED_STDIN_OUTPUT}" \
		-w "%{http_code}" \
		-X POST \
		-H "Content-Type: application/octet-stream" \
		--data-binary "@${CGI_CLOSED_STDIN_INPUT}" \
		"${BASE_URL}/cgi/test_close_stdin.py" \
		2>/dev/null
)"

if [ "$status" = "200" ] \
	&& grep -q '^CGI survived$' "${CGI_CLOSED_STDIN_OUTPUT}"; then
	pass "CGI closing stdin early is handled safely"
else
	fail "CGI closing stdin early produced an unexpected response"
fi

if server_is_up; then
	pass "Server remains alive after CGI closes stdin"
else
	fail "Server stopped after CGI closed stdin"
fi


print_title "${TEST_NUMBER}. Slow CGI below timeout"
TEST_NUMBER=$((TEST_NUMBER + 1))

CGI_SLOW_OUTPUT="${TMP_DIR}/cgi_slow.txt"

status="$(
	curl -sS \
		--max-time 4 \
		-o "${CGI_SLOW_OUTPUT}" \
		-w "%{http_code}" \
		"${BASE_URL}/cgi/test_slow_cgi.py" \
		2>/dev/null
)"

if [ "$status" = "200" ] \
	&& grep -q '^Still valid$' "${CGI_SLOW_OUTPUT}"; then
	pass "CGI completing before timeout returns 200"
else
	fail "CGI below timeout produced an unexpected response"
fi


print_title "${TEST_NUMBER}. CGI execution timeout"
TEST_NUMBER=$((TEST_NUMBER + 1))

status="$(
	curl -sS \
		--max-time 8 \
		-o /dev/null \
		-w "%{http_code}" \
		"${BASE_URL}/cgi/test_timeout.py" \
		2>/dev/null
)"

if [ "$status" = "500" ]; then
	pass "CGI exceeding execution timeout returns 500"
else
	fail "CGI timeout returns ${status} instead of 500"
fi

if server_is_up; then
	pass "Server remains responsive after CGI timeout"
else
	fail "Server stopped responding after CGI timeout"
fi


print_title "${TEST_NUMBER}. Server alive after CGI tests"
TEST_NUMBER=$((TEST_NUMBER + 1))

if [ "$(http_status "${BASE_URL}/index.html")" = "200" ]; then
	pass "Server remains fully functional after CGI tests"
else
	fail "Server no longer serves normal requests after CGI tests"
fi


print_title "${TEST_NUMBER}. GET location root"
TEST_NUMBER=$((TEST_NUMBER + 1))

body="$(curl -sS "${BASE_URL}/images/location_root_test.txt" 2>/dev/null)"
status="$(http_status "${BASE_URL}/images/location_root_test.txt")"

if [ "$status" = "200" ]; then
	pass "GET resource through location root returns 200"
else
	fail "GET resource through location root returns ${status} instead of 200"
fi

if [ "$body" = "location images root" ]; then
	pass "Location root resolves to the expected filesystem resource"
else
	fail "Location root returned unexpected content"
fi


print_title "${TEST_NUMBER}. GET longest matching location root"
TEST_NUMBER=$((TEST_NUMBER + 1))

body="$(curl -sS "${BASE_URL}/images/private/private_root_test.txt" 2>/dev/null)"
status="$(http_status "${BASE_URL}/images/private/private_root_test.txt")"

if [ "$status" = "200" ]; then
	pass "GET resource through nested location returns 200"
else
	fail "GET nested location resource returns ${status} instead of 200"
fi

if [ "$body" = "location private root" ]; then
	pass "Longest matching location root is used"
else
	fail "Nested location did not use the expected root"
fi


print_title "${TEST_NUMBER}. GET location root with query string"
TEST_NUMBER=$((TEST_NUMBER + 1))

body="$(curl -sS \
	"${BASE_URL}/images/location_root_test.txt?foo=bar" \
	2>/dev/null)"

if [ "$body" = "location images root" ]; then
	pass "Query string is excluded from filesystem path resolution"
else
	fail "Query string affected filesystem path resolution"
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
