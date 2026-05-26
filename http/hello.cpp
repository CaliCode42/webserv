#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <string>
#include <vector>

int main()
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
        return 1;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&addr, sizeof(addr)) == -1)
        return 1;

    if (listen(serverSocket, 10) == -1)
        return 1;

    std::vector<pollfd> fds;

    pollfd server_fd;
    server_fd.fd = serverSocket;
    server_fd.events = POLLIN;
    server_fd.revents = 0;

    fds.push_back(server_fd);

    while (true)
    {
        if (poll(fds.data(), fds.size(), -1) < 0)
            continue;

        for (size_t i = 0; i < fds.size(); i++)
        {
            // =========================
            // 1. NOUVEAU CLIENT
            // =========================
            if (fds[i].fd == serverSocket && (fds[i].revents & POLLIN))
            {
                int client_fd = accept(serverSocket, NULL, NULL);
                if (client_fd == -1)
                    continue;

                pollfd client;
                client.fd = client_fd;
                client.events = POLLIN;
                client.revents = 0;

                fds.push_back(client);

                std::cout << "Client connected: " << client_fd << std::endl;
            }

            // =========================
            // 2. CLIENT ENVOIE DONNÉES
            // =========================
            else if (fds[i].fd != serverSocket && (fds[i].revents & POLLIN))
            {
                char buffer[4096];

                int bytes = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);

                if (bytes <= 0)
                {
                    std::cout << "Client disconnected: " << fds[i].fd << std::endl;
                    close(fds[i].fd);
                    fds.erase(fds.begin() + i);
                    i--;
                    continue;
                }

                buffer[bytes] = '\0';
                std::cout << "Request:\n" << buffer << std::endl;

                std::string body = "<h1>Hello webserv</h1>";

                std::string response =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: " + std::to_string(body.size()) + "\r\n"
                    "\r\n" +
                    body;

                send(fds[i].fd, response.c_str(), response.size(), 0);

                std::cout << "Response sent to fd: " << fds[i].fd << std::endl;

                // version simple: on ferme direct (OK pour étape 1)
                close(fds[i].fd);
                fds.erase(fds.begin() + i);
                i--;
            }
        }
    }

    close(serverSocket);
}