/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:33:43 by tcali             #+#    #+#             */
/*   Updated: 2026/07/29 19:39:30 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Utils.hpp"
#include "Client.hpp"
#include "../http/MethodHandler.hpp"
#include "../http/HttpRequest.hpp"
#include <unistd.h>
#include <netinet/in.h>
#include <string>
#include <cerrno>

// Server::Server(int port): _port(port), _serverSocket(-1) 
// {
// 	std::cout << "[Server] constructor called: server created" << std::endl;
// }

Server::Server(int port, const ServerConfig& config): _port(port), _serverSocket(-1),
	_config(config), _handler(_config)
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
		if (poll(&_fds[0], _fds.size(), -1) < 0)
			continue;

		for (size_t i = 0; i < _fds.size(); i++)
		{
			int		fd = _fds[i].fd;
			short	revents = _fds[i].revents;

			if (revents == 0)
                continue;

			if (fd == _serverSocket)
			{
				if (revents & POLLIN)
					acceptClient();

				continue ;
			}
			
			if (revents & POLLIN)
			{
				std::map<int, Client>::iterator it = _clients.find(fd);

				if (it != _clients.end())
					handleClientRead(it->second);
			}

			std::map<int, Client>::iterator it = _clients.find(fd);

            if ((revents & POLLOUT) && it != _clients.end())
                handleClientWrite(it->second);
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

	if (bytes == 0)
	{
		removeClient(client.getFd());
		return ;
	}

	if (bytes < 0)
	{
		if (errno == EAGAIN ||errno == EWOULDBLOCK)
			return ;
		
		removeClient(client.getFd());
		return ;
	}

	client.appendToReadBuffer(std::string(buffer, bytes));

	if (!client.hasCompleteRequest())
        return;

	std::string rawRequest = client.extractRequest();

    HttpRequest request;
	request.parse(rawRequest);

    std::cout << "Method: " << request.getMethod() << std::endl;
    std::cout << "Path: " << request.getUri() << std::endl;
    std::cout << "Version: " << request.getVersion() << std::endl;

	HttpResponse	response = _handler.handle(request);

	std::string rawResponse = response.serialize();

	std::cout << "Append response to client's _writeBuffer: " << client.getFd() << std::endl;
	
	client.appendToWriteBuffer(rawResponse);
	enableClientWrite(client.getFd());
}

void Server::handleClientWrite(Client& client)
{
    const std::string&	data = client.getWriteBuffer();

    if (data.empty())
        return ;

    ssize_t	bytesSent = send(client.getFd(), data.c_str(), data.size(), 0);

	if (bytesSent < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		return ;
		
		removeClient(client.getFd());
	}
	else if (bytesSent > 0)
	{
		client.removeSentBytes(static_cast<std::size_t>(bytesSent));
	}
	else // bytesSent == 0
		return ;

	if (!client.hasPendingWriteData())
    {
        int fd = client.getFd();
        disableClientWrite(fd);
        removeClient(fd);
    }
}

void	Server::enableClientWrite(int fd)
{
    for (std::size_t i = 0; i < _fds.size(); ++i)
    {
        if (_fds[i].fd == fd)
        {
            _fds[i].events |= POLLOUT;
            return ;
        }
    }
}

void	Server::disableClientWrite(int fd)
{
    for (std::size_t i = 0; i < _fds.size(); ++i)
    {
        if (_fds[i].fd == fd)
        {
            _fds[i].events &= ~POLLOUT;
            return ;
        }
    }
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
