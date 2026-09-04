*This project has been created as part of the 42 curriculum by tcali,
sdossa.*

# Webserv

HTTP/1.1 server implemented from scratch in C++98.

Core challenges:
- non-blocking network I/O with poll()
- incremental HTTP request parsing
- virtual server / route configuration
- CGI lifecycle using fork, execve and pipes
- partial socket reads/writes
- resource and timeout management

[Architecture Diagram]

``` text
                        ┌──────────────┐
                        │ ConfigParser │
                        └──────┬───────┘
                               ↓
Client ──TCP──> Listening socket
                   │
                   ↓
             ┌──────────┐
             │ poll()   │
             └────┬─────┘
                  ↓
               Client
                  ↓
             HttpRequest
                  ↓
          ServerConfig /
        Location selection
             ↙        ↘
    MethodHandler      CGI
         ↓           fork/exec
    HttpResponse       pipes
             ↘        ↙
                Client
                  ↓
                send()
```

## Description

Webserv is a C++98 HTTP/1.1 server developed as part of the 42
curriculum.

The goal of the project is to understand how a web server works at a low
level by implementing request parsing, response generation, socket
management, routing, configuration, file serving, uploads, and CGI
execution without relying on an existing HTTP server library.

The server uses non-blocking sockets and a `poll()`-based event loop to
handle network I/O. A single process can listen on multiple ports and
serve different configurations.

The project supports:

-   HTTP `GET`, `POST`, and `DELETE` methods;
-   static file serving;
-   configurable routes;
-   configurable document roots and index files;
-   per-route allowed methods;
-   directory listing (`autoindex`);
-   HTTP redirections;
-   file uploads with configurable storage directories;
-   configurable maximum request body size;
-   custom error pages;
-   CGI execution based on file extensions;
-   Python and PHP CGI configuration;
-   multiple listening ports;
-   multiple server configurations;
-   HTTP request validation and error handling.

The repository also contains dedicated configuration files, CGI scripts,
static resources, custom error pages, and automated tests used to
validate the server.

## Project Structure

``` text
.
├── Makefile
├── webserv.conf
├── test.conf
├── config_tests/
├── http/
├── includes/
├── srcs/
├── tests/
│   ├── scripts/
│   └── test_cgi/
├── www/
└── www2/
```

The main directories are:

-   `srcs/` --- server, client, configuration, and CGI implementation;
-   `http/` --- HTTP request/response handling and method processing;
-   `includes/` --- project headers;
-   `www/` and `www2/` --- files and directories served by the
    demonstration configurations;
-   `tests/scripts/` --- automated and edge-case test scripts;
-   `tests/test_cgi/` --- CGI scripts used by the automated tests;
-   `config_tests/` --- invalid configuration files used to test parser
    validation.

## Instructions

### Requirements

The project is written for a Unix-like environment and is compiled in
C++98 mode.

A C++ compiler and `make` are required.

Python CGI execution uses `/usr/bin/python3`.

The provided configuration also contains a PHP CGI mapping using
`/usr/bin/php-cgi`. PHP CGI requests therefore require `php-cgi` to be
installed at that path.

### Compilation

Compile the project from the repository root:

``` bash
make
```

This creates the `webserv` executable.

Available Makefile rules are:

``` bash
make
make clean
make fclean
make re
```

### Execution

Run the server with the provided demonstration configuration:

``` bash
./webserv webserv.conf
```

The provided `webserv.conf` defines two servers:

-   port `8080`, serving content from `www`;
-   port `8081`, serving content from `www2`.

The main configuration demonstrates static content, custom error pages,
method restrictions, autoindex, a custom index file, redirection,
uploads, CGI execution, and a maximum request body size.

Example requests:

``` bash
curl http://localhost:8080/
curl http://localhost:8080/listing/
curl http://localhost:8080/custom-index/
curl -i http://localhost:8080/old
curl http://localhost:8081/
curl http://localhost:8080/cgi/hello.py
```

POST through the CGI echo script:

``` bash
curl -X POST -d 'Hello from POST' http://localhost:8080/cgi/echo.py
```

### Configuration

Configuration is organized into `server` blocks containing `location`
blocks.

The provided configuration demonstrates directives such as:

``` text
listen
server_name
root
client_max_body_size
error_page
allow_methods
index
autoindex
redirect
upload_enabled
upload_path
cgi_extension
```

Example:

``` conf
server {
    listen 8080;
    server_name localhost;
    root www;

    client_max_body_size 1048576;

    error_page 404 /errors/404.html;

    location / {
        root www;
        index index.html;
        allow_methods GET POST DELETE;
    }

    location /listing {
        root www/listing;
        allow_methods GET;
        autoindex on;
    }

    location /cgi {
        root www/cgi-bin/;
        allow_methods GET POST;
        cgi_extension .py /usr/bin/python3;
        cgi_extension .php /usr/bin/php-cgi;
    }
}
```

`webserv.conf` is the main demonstration configuration.

`test.conf` is used by the automated test suite and contains additional
routes and server configurations intended specifically for testing.

## Testing

The main automated test suite can be launched directly from the
repository root:

``` bash
./tests/scripts/test_webserv.sh
```

The script handles the test lifecycle automatically:

1.  it builds `webserv` with `make` if the executable is missing;
2.  it creates the temporary files required by the tests;
3.  it starts the server using `test.conf`;
4.  it waits for the server to become available;
5.  it runs the test suite;
6.  it stops the server when the tests finish;
7.  it removes the temporary test files.

It is therefore not necessary to start `webserv` manually before running
the test suite.

Invalid configuration samples are available in `config_tests/`. They
cover cases such as invalid ports, missing arguments, missing
semicolons, unknown directives, invalid `autoindex` values, and unclosed
blocks.

## Technical Choices

### Non-blocking I/O

Network sockets are configured as non-blocking. The server uses `poll()`
to monitor listening sockets, connected clients, and CGI pipe file
descriptors.

This allows multiple clients and CGI processes to be handled without
creating one server process per connection.

### HTTP Processing

Incoming data is accumulated and parsed into HTTP requests. The server
validates requests, applies the selected server and location
configuration, dispatches the request to the appropriate method handler
or CGI process, and serializes the resulting HTTP response.

### CGI

CGI programs are executed in child processes. Pipes are used to send the
request body to the CGI process and collect its output.

CGI interpreters are selected from the configuration according to the
requested file extension. The provided demonstration configuration
includes Python and PHP mappings.

### Error Handling

The server generates HTTP error responses for invalid requests and
processing failures. When a custom error page is configured for a status
code, that page is used when available; otherwise, the server generates
a default error response.

## Team & Contributions

This project was developed collaboratively by **tcali** and **sdossa**.
While both contributors worked across the codebase during integration and debugging, the initial areas of responsibility were:

### tcali
- Server core and connection management
- Non-blocking network I/O and `poll()` event loop
- CGI execution and process integration (`fork`, `execve`, pipes)
- Parts of the server configuration system
- Integration, debugging and modifications across HTTP request/response handling

### sdossa
- HTTP request parsing and method handling
- HTTP response handling
- Cookies and session management
- Parts of the configuration parsing and route configuration

Both contributors worked on configuration handling, integration, testing and debugging throughout the project.

## Resources

The following references were useful for understanding HTTP and the
system interfaces involved in the project:

-   MDN Web Docs --- HTTP overview:
    https://developer.mozilla.org/en-US/docs/Web/HTTP
-   RFC 9110 --- HTTP Semantics:
    https://datatracker.ietf.org/doc/html/rfc9110
-   RFC 9112 --- HTTP/1.1: https://datatracker.ietf.org/doc/html/rfc9112
-   Linux manual pages (`man`) for system calls and interfaces used by
    the project, including `socket`, `bind`, `listen`, `accept`, `poll`,
    `recv`, `send`, `fcntl`, `fork`, `pipe`, `dup2`, `execve`, and
    `waitpid`.
-    CGI documentation :
     https://www.tutorialspoint.com/python/python_cgi_programming.htm
     https://www.tutorialspoint.com/cplusplus/cpp_web_programming.htm
-    Network Programming Documentation :
     https://beej.us/guide/bgnet/
     https://www.youtube.com/watch?v=jS9rBienEFQ

### Use of AI

AI tools were used for debugging, test design, edge-case investigation and documentation assistance. All integrated changes were reviewed and validated by the authors.
