/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:33:43 by tcali             #+#    #+#             */
/*   Updated: 2026/06/23 10:53:59 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Utils.hpp"
#include "Client.hpp"
#include <unistd.h>
#include <netinet/in.h>
#include <string>

Server::Server(int port) : _port(port), _serverSocket(-1)
{
	std::cout << "[Server] constructor called: server created" << std::endl;
}

// Server::Server(const Server& other)
// {
// 	std::cout << "[Server] Copy constructor called" << std::endl;
// 	*this = other;
// }

// Server& Server::operator=(const Server& other)
// {
// 	if (this != &other)
// 	{
// 		// copy attributes here
// 	}
// 	std::cout << "[Server] Copy assignment operator called" << std::endl;
// 	return (*this);
// }

Server::~Server()
{
	std::cout << "[Server] Destructor called" << std::endl;
	for (std::vector<pollfd>::iterator it = _fds.begin();
		it != _fds.end(); ++it)
{
	close(it->fd);
};
}

void	Server::initSocket()
{
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket == -1)
		throw std::runtime_error("failed to init socket.");

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(_serverSocket, (sockaddr*)&addr, sizeof(addr)) == -1)
	{
		throw std::runtime_error("failed to bind socket.");
	}

	if (listen(_serverSocket, 10) == -1)
	{
		throw std::runtime_error("socket failed to listen.");
	}

	pollfd	pfd;

	pfd.fd = _serverSocket;
	pfd.events = POLLIN;
	pfd.revents = 0;

	_fds.push_back(pfd);
}

void	Server::run()
{
	while (true)
	{
		if (poll(_fds.data(), _fds.size(), -1) < 0)
			continue;

		for (size_t i = 0; i < _fds.size(); i++)
		{
			if (!(_fds[i].revents & POLLIN))
				continue;

			if (_fds[i].fd == _serverSocket)
				acceptClient();
			else
				handleClientRead(_clients[_fds[i].fd]);
		}
	}
}

void	Server::acceptClient()
{
	int client_fd = accept(_serverSocket, NULL, NULL);
	if (client_fd == -1)
		return ;

	pollfd	client;
	
	client.fd = client_fd;
	client.events = POLLIN;
	client.revents = 0;

	_fds.push_back(client);
	_clients[client_fd] = Client(client_fd);

	std::cout << "Client connected: " << client_fd << std::endl;
}


void	Server::handleClientRead(Client& client)
{
	char buffer[4096];

	int bytes = recv(client.getFd(), buffer, sizeof(buffer) - 1, 0);

	if (bytes <= 0)
	{
		Server::removeClient(client.getFd());
		return ;
	}

	buffer[bytes] = '\0';
	client.appendToBuffer(buffer);

	std::cout << client.getBuffer() << std::endl;

	std::string body = "<h1>Hello webserv</h1>";

	std::string response =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: " + toString(body.size()) + "\r\n"
		"\r\n" +
		body;

	send(client.getFd(), response.c_str(), response.size(), 0);

	std::cout << "Response sent to fd: " << client.getFd() << std::endl;
	removeClient(client.getFd());
}

void	Server::removeClient(int fd)
{
	std::cout << "Client disconnected: " << fd << std::endl;
	close(fd);
	_clients.erase(fd);

	for (std::vector<pollfd>::iterator it = _fds.begin();
        	it != _fds.end(); ++it)
    {
        if (it->fd == fd)
        {
            _fds.erase(it);
            break;
        }
    }
}
