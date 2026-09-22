 #!/usr/bin/env python3

import os
import sys
from urllib.parse import parse_qs

if 'HTTP_CONTENT_TYPE' in os.environ and 'CONTENT_TYPE' not in os.environ:
    os.environ['CONTENT_TYPE'] = os.environ['HTTP_CONTENT_TYPE']

print("Content-Type: text/html\r\n\r\n", end='')
print("<html><body>")
print("<h1>CGI Test - Python</h1>")
print("<h2>Environment Variables:</h2>")
print("<pre>")
for key, value in sorted(os.environ.items()):
    print(f"{key} = {value}")
print("</pre>")

print("<h2>Request Info:</h2>")
print(f"<p>Method: {os.environ.get('REQUEST_METHOD', 'N/A')}</p>")
print(f"<p>Query: {os.environ.get('QUERY_STRING', 'N/A')}</p>")

if os.environ.get('REQUEST_METHOD') == 'POST':
    content_type = os.environ.get('CONTENT_TYPE', '')

    if 'multipart/form-data' in content_type:
        print("<p>Multipart parsing is not supported by this test script.</p>")
    else:
        body = sys.stdin.buffer.read()
        fields = parse_qs(body.decode('utf-8', errors='replace'))
        print(f"<p>Body received: {len(body)} bytes</p>")
        print(f"<p>nom: {fields.get('nom', [''])[0]}</p>")
        print(f"<p>prenom: {fields.get('prenom', [''])[0]}</p>")
    print("</body></html>")