/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:33:43 by tcali             #+#    #+#             */
/*   Updated: 2026/08/20 18:09:55 by tcali            ###   ########.fr       */
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
		int result = poll(&_fds[0], static_cast<nfds_t>(_fds.size()), _POLL_TIMEOUT);
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

			if (isCgiStdinFd(fd) || isCgiStdoutFd(fd))
			{
				handleCgiEvents(fd, revents);
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
					continue ;
				}
			}

			if ((revents & POLLOUT) && it != _clients.end() && !isMarkedForRemoval(fd))
				handleClientWrite(it->second);
		}
		checkCgiProcesses();
		checkClientTimeouts();
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

// Temporary :
// Just to test Error handling,
// Later this must not be the responsibility of Server.
HttpResponse	Server::buildErrorResponse(int statusCode)
{
	HttpResponse response;

	response.setStatus(statusCode);
	response.setBody(
		"<html><body><h1>" +
		turnIntoString(statusCode) + " " +
		HttpResponse::reasonPhrase(statusCode) +
		"</h1></body></html>",
		"text/html"
	);

	return (response);
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

	client.updateActivity();
	
	HttpRequest& request = client.getRequest();

	request.appendData(buffer, static_cast<std::size_t>(bytes));

	if (request.hasError())
	{
		int	statusCode = request.errorCode();

		HttpResponse	response = buildErrorResponse(statusCode);

		client.appendToWriteBuffer(response.serialize());
		enableClientWrite(client.getFd());
		return ;
	}

	if (!request.isComplete())
	{
		return ;
	}

	try {
		std::cout << "Method: " << request.getMethod() << std::endl;
		std::cout << "Path: " << request.getUri() << std::endl;
		std::cout << "Version: " << request.getVersion() << std::endl;

		const LocationConfig	*location = _config.findLocation(request.getUri());

		if (location != NULL && CgiHandler::isCgiRequest(request.getUri(), *location))
		{
			if (!startCgiProcess(client, *location))
			{
				HttpResponse	response = buildErrorResponse(500);

				client.appendToWriteBuffer(response.serialize());
				enableClientWrite(client.getFd());
			}
			else
			{
				disableClientEvents(client.getFd());
			}
			return ;
		}

		HttpResponse	response = _handler.handle(request);

		std::cout << "Append response to client's _writeBuffer: " << client.getFd() << std::endl;
		
		client.appendToWriteBuffer(response.serialize());
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

void	Server::handleClientWrite(Client& client)
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
		client.updateActivity();
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

void	Server::disableClientEvents(int fd)
{
	for (std::size_t i = 0; i < _fds.size(); ++i)
	{
		if (_fds[i].fd == fd)
		{
			_fds[i].events = 0;
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

	removePollFd(fd);
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

void Server::checkClientTimeouts()
{
	std::time_t	now = std::time(NULL);

	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (_cgiProcesses.find(it->first) != _cgiProcesses.end())
			continue ;

		std::cout << "fd " << it->first
			<< " inactive for "
			<< now - it->second.getLastActivity()
			<< " seconds"
			<< std::endl;

		if (now - it->second.getLastActivity() >= _CLIENT_TIMEOUT)
		{
			std::cout << "Timeout fd " << it->first << std::endl;
			markClientForRemoval(it->first);
		}
	}
}

void	Server::checkCgiProcesses()
{
	for (std::map<int, CgiProcess*>::iterator it = _cgiProcesses.begin();
			it != _cgiProcesses.end(); ++it)
	{
		if (it->second != NULL)
			it->second->checkProcessStatus();
	}
}

void	Server::handleCgiEvents(int fd, short revents)
{
	if (isCgiStdinFd(fd))
		handleCgiStdinEvent(fd, revents);

	else if (isCgiStdoutFd(fd))
		handleCgiStdoutEvent(fd, revents);
}

void	Server::handleCgiStdinEvent(int fd, short revents)
{
	std::map<int, int>::iterator	stdinIt = _cgiStdinFds.find(fd);

	if (stdinIt == _cgiStdinFds.end())
		return ;

	int	clientFd = stdinIt->second;

	std::map<int, CgiProcess*>::iterator	processIt =
		_cgiProcesses.find(clientFd);

	if (processIt == _cgiProcesses.end())
	{
		removePollFd(fd);
		_cgiStdinFds.erase(stdinIt);
		return ;
	}

	CgiProcess	*process = processIt->second;

	if (revents & (POLLERR | POLLHUP | POLLNVAL))
	{
		removePollFd(fd);
		_cgiStdinFds.erase(stdinIt);
		return ;
	}

	if (!(revents & POLLOUT))
		return ;

	if (!process->writeInput())
	{
		process->closeInput();
		removePollFd(fd);
		_cgiStdinFds.erase(stdinIt);
		return ;
	}

	if (process->getStdinFd() == -1)
	{
		removePollFd(fd);
		_cgiStdinFds.erase(stdinIt);
	}
}

void	Server::handleCgiStdoutEvent(int fd, short revents)
{
	std::map<int, int>::iterator	stdoutIt = _cgiStdoutFds.find(fd);

	if (stdoutIt == _cgiStdoutFds.end())
		return ;

	int	clientFd = stdoutIt->second;

	std::map<int, CgiProcess*>::iterator	processIt =
		_cgiProcesses.find(clientFd);

	if (processIt == _cgiProcesses.end())
	{
		removePollFd(fd);
		_cgiStdoutFds.erase(stdoutIt);
		return ;
	}

	CgiProcess	*process = processIt->second;

	if (revents & (POLLERR | POLLNVAL))
	{
		process->closeOutput();
		removePollFd(fd);
		_cgiStdoutFds.erase(stdoutIt);
		return ;
	}

	if (!(revents & (POLLIN | POLLHUP)))
		return ;

	if (!process->readOutput())
	{
		removePollFd(fd);
		_cgiStdoutFds.erase(stdoutIt);
		return ;
	}

	if (process->getStdoutFd() == -1)
	{
		removePollFd(fd);
		_cgiStdoutFds.erase(stdoutIt);
	}
}

bool	Server::isCgiStdinFd(int fd) const
{
	return (_cgiStdinFds.find(fd) != _cgiStdinFds.end());
}

bool	Server::isCgiStdoutFd(int fd) const
{
	return (_cgiStdoutFds.find(fd) != _cgiStdoutFds.end());
}

void	Server::addPollFd(int fd, short events)
{
	pollfd	pfd;

	pfd.fd = fd;
	pfd.events = events;
	pfd.revents = 0;

	_fds.push_back(pfd);
}

void	Server::removePollFd(int fd)
{
	for (std::vector<pollfd>::iterator it = _fds.begin();
			it != _fds.end(); ++it)
	{
		if (it->fd == fd)
		{
			_fds.erase(it);
			return ;
		}
	}
}

bool	Server::startCgiProcess(Client& client, const LocationConfig& location)
{
	HttpRequest&	request = client.getRequest();
	std::string		uri = request.getUri();
	std::string		interpreter = CgiHandler::getInterpreter(uri, location);
	std::string		scriptPath = CgiHandler::resolveScriptPath(uri, location);

	if (interpreter.empty() || scriptPath.empty())
		return (false);

	CgiProcess	*process = new CgiProcess();

	try
	{
		CgiProcess::envMap	env = CgiHandler::buildEnvironment(request, uri,
			location, scriptPath);

		if (!process->start(interpreter, scriptPath, env, request.getBody()))
		{
			delete process;
			return (false);
		}
	}
	catch (...)
	{
		delete process;
		throw;
	}

	int	clientFd = client.getFd();

	_cgiProcesses[clientFd] = process;

	if (process->getStdinFd() != -1)
	{
		int	stdinFd = process->getStdinFd();

		_cgiStdinFds[stdinFd] = clientFd;
		addPollFd(stdinFd, POLLOUT);
	}

	if (process->getStdoutFd() != -1)
	{
		int	stdoutFd = process->getStdoutFd();

		_cgiStdoutFds[stdoutFd] = clientFd;
		addPollFd(stdoutFd, POLLIN);
	}

	return (true);
}