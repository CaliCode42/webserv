/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:33:43 by tcali             #+#    #+#             */
/*   Updated: 2026/07/31 16:29:02 by tcali            ###   ########.fr       */
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
#include <cstring>
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

Server::~Server()
{
	std::cout << "[Server] Destructor called" << std::endl;
	for (std::vector<pollfd>::iterator it = _fds.begin();
		it != _fds.end(); ++it)
	{
		close(it->fd);
	}
}

void	Server::initSocket()
{
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket == -1)
		throw std::runtime_error("failed to init socket.");

	sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_port);
	addr.sin_addr.s_addr = INADDR_ANY;

	int opt = 1;

	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR,
		&opt, sizeof(opt)) == -1)
	{
		close(_serverSocket);
		_serverSocket = -1;
		throw std::runtime_error("failed to set SO_REUSEADDR");
	}

	if (bind(_serverSocket, reinterpret_cast<sockaddr*>(&addr),
		sizeof(addr)) == -1)
	{
		close(_serverSocket);
		_serverSocket = -1;
		throw std::runtime_error("failed to bind socket.");
	}

	if (listen(_serverSocket, 10) == -1)
	{
		close(_serverSocket);
		_serverSocket = -1;
		throw std::runtime_error("socket failed to listen.");
	}

	if (!setNonBlocking(_serverSocket))
	{
		close(_serverSocket);
		_serverSocket = -1;
		throw std::runtime_error("failed to set server socket as non-blocking");
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
		int result = poll(&_fds[0], static_cast<nfds_t>(_fds.size()), -1);
		if (result < 0)
		{
			if (errno == EINTR)
				continue ;

			throw (std::runtime_error("poll failed"));
		}

		for (size_t i = 0; i < _fds.size(); i++)
		{
			int		fd = _fds[i].fd;
			short	revents = _fds[i].revents;

			if (revents == 0)
				continue ;

			if (fd == _serverSocket)
			{
				if (revents & (POLLERR | POLLHUP | POLLNVAL))
					throw std::runtime_error("server socket poll error");

				if (revents & POLLIN)
					acceptClient();

				continue ;
			}

			if (revents & (POLLERR | POLLNVAL))
			{
				markClientForRemoval(fd);
				continue ;
			}

			if (revents & POLLIN)
			{
				std::map<int, Client>::iterator it = _clients.find(fd);

				if (it != _clients.end())
					handleClientRead(it->second);
			}

			std::map<int, Client>::iterator	it = _clients.find(fd);
			
			if ((revents & POLLHUP) && !isMarkedForRemoval(fd))
			{

				if (it == _clients.end() || !it->second.hasPendingWriteData())
				{
					markClientForRemoval(fd);
					continue;
				}
			}

			if ((revents & POLLOUT) && it != _clients.end() && !isMarkedForRemoval(fd))
				handleClientWrite(it->second);
		}
		removeMarkedClients();
	}
}

void	Server::acceptClient()
{
	while (true)
	{
		int clientFd = accept(_serverSocket, NULL, NULL);

		if (clientFd < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return ;

			if (errno == EINTR)
				continue ;
			
			std::cerr << "accept() failed: " << strerror(errno) << std::endl;
			return ;
		}

		if (!setNonBlocking(clientFd))
		{
			close(clientFd);
			continue ;
		}
		pollfd	client;
		
		client.fd = clientFd;
		client.events = POLLIN;
		client.revents = 0;

		_fds.push_back(client);
		_clients.insert(std::make_pair(clientFd, Client(clientFd)));

		std::cout << "Client connected: " << clientFd << std::endl;
	}
}

void	Server::handleClientRead(Client& client)
{
	char buffer[4096];

	ssize_t	bytes = recv(client.getFd(), buffer, sizeof(buffer), 0);

	if (bytes == 0)
	{
		markClientForRemoval(client.getFd());
		return ;
	}

	if (bytes < 0)
	{
		if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK)
			return ;
		
		markClientForRemoval(client.getFd());
		return ;
	}

	client.appendToReadBuffer(std::string(buffer, bytes));

	if (!client.hasCompleteRequest())
		return;

	std::string rawRequest = client.extractRequest();

	try {
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
	catch (const std::exception& e)
	{
		// temporary, just to remove warnings
		std::cerr << "HTTP processing failed for client "
              << client.getFd()
              << ": "
              << e.what()
              << std::endl;

		// Build 400 or 500 http response
		markClientForRemoval(client.getFd());
	}
}

void Server::handleClientWrite(Client& client)
{
	const std::string&	data = client.getWriteBuffer();

	if (data.empty())
	{
		markClientForRemoval(client.getFd());
		return ;
	}

	ssize_t	bytesSent = send(client.getFd(), data.c_str(), data.size(), MSG_NOSIGNAL);

	if (bytesSent < 0)
	{
		if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK)
			return ;
		
		markClientForRemoval(client.getFd());
		return ;
	}
	else if (bytesSent > 0)
	{
		client.removeSentBytes(static_cast<std::size_t>(bytesSent));
	}
	else // bytesSent == 0
	{
		markClientForRemoval(client.getFd());
		return ;
	}

	if (!client.hasPendingWriteData())
	{
		int fd = client.getFd();
		disableClientWrite(fd);
		markClientForRemoval(fd);
	}
}

void	Server::enableClientWrite(int fd)
{
	for (std::size_t i = 0; i < _fds.size(); ++i)
	{
		if (_fds[i].fd == fd)
		{
			_fds[i].events = POLLOUT;
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

void	Server::markClientForRemoval(int fd)
{
	if (!isMarkedForRemoval(fd))
		_clientsToRemove.push_back(fd);
}

bool Server::isMarkedForRemoval(int fd) const
{
	for (std::vector<int>::const_iterator it = _clientsToRemove.begin();
		 it != _clientsToRemove.end();
		 ++it)
	{
		if (*it == fd)
			return true;
	}

	return false;
}

void Server::removeMarkedClients()
{
	for (std::vector<int>::const_iterator it = _clientsToRemove.begin();
		 it != _clientsToRemove.end();
		 ++it)
		removeClient(*it);

	_clientsToRemove.clear();
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

bool	setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);

	if (flags == -1)
	{
		throw std::runtime_error("fcntl(F_GETFL) failed");
		return (false);
	}

	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
		throw std::runtime_error("fcntl(F_SETFL) failed");
		return (false);
	}
	return (true);
		
}