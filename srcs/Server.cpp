/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:33:43 by tcali             #+#    #+#             */
/*   Updated: 2026/09/01 15:40:59 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Utils.hpp"

#include <unistd.h>
#include <netinet/in.h>
#include <string>
#include <cstring>
#include <cerrno>
#include <signal.h>

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

Server::Server(const std::vector<ServerConfig>& configs): _configs(configs)
{
	for (std::size_t i = 0; i < _configs.size(); ++i)
	{
		_handlers[&_configs[i]] = new MethodHandler(_configs[i]);
	}

	std::cout << "[Server] constructor called: server created" << std::endl;
}

Server::~Server()
{
	std::cout << "[Server] Destructor called" << std::endl;

	for (std::map<const ServerConfig*, MethodHandler*>::iterator it =
			_handlers.begin(); it != _handlers.end(); ++it)
		delete it->second;

	_handlers.clear();

	for (std::vector<pollfd>::iterator it = _fds.begin();
		it != _fds.end(); ++it)
		close(it->fd);

	_clients.clear();

	for (std::vector<ListeningSocket>::iterator it = _listeningSockets.begin();
			it != _listeningSockets.end(); ++it)
    {
        if (it->fd >= 0)
        {
            close(it->fd);
            it->fd = -1;
        }
    }

    _listeningSockets.clear();
    _fds.clear();
}

int	Server::createListeningSocket(unsigned int port)
{
	int	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket < 0 )
		throw std::runtime_error("failed to init socket.");

	sockaddr_in	addr;
	
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	int opt = 1;

	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,
		&opt, sizeof(opt)) == -1)
	{
		close(serverSocket);
		throw std::runtime_error("failed to set SO_REUSEADDR");
	}

	if (bind(serverSocket, reinterpret_cast<sockaddr*>(&addr),
		sizeof(addr)) == -1)
	{
		close(serverSocket);
		throw std::runtime_error("failed to bind socket.");
	}

	if (listen(serverSocket, 10) == -1)
	{
		close(serverSocket);
		throw std::runtime_error("socket failed to listen.");
	}

	if (!setNonBlocking(serverSocket))
	{
		close(serverSocket);
		throw std::runtime_error("failed to set server socket as non-blocking");
	}
	
	return (serverSocket);
}

void	Server::initSockets()
{	
	for (std::size_t i = 0; i < _configs.size(); ++i)
	{
		ListeningSocket	*listener = findListeningSocketByPort(_configs[i].getPort());

		if (listener != NULL)
		{
			listener->configs.push_back(&_configs[i]);
			continue ;
		}
		
		int	fd = createListeningSocket(_configs[i].getPort());

		ListeningSocket	newListener;

		newListener.fd = fd;
		newListener.port = _configs[i].getPort();
		newListener.configs.push_back(&_configs[i]);

		_listeningSockets.push_back(newListener);

		pollfd	pfd;

		pfd.fd = fd;
		pfd.events = POLLIN;
		pfd.revents = 0;

		_fds.push_back(pfd);
	}
}


const ListeningSocket *Server::findListeningSocket(int fd) const
{
	for (std::size_t i = 0; i < _listeningSockets.size(); ++i)
	{
		if (_listeningSockets[i].fd == fd)
			return (&_listeningSockets[i]);
	}

	return (NULL);
}

ListeningSocket	*Server::findListeningSocketByPort(unsigned int port)
{
	for (std::size_t i = 0; i < _listeningSockets.size(); ++i)
	{
		if (_listeningSockets[i].port == port)
			return (&_listeningSockets[i]);
	}

	return (NULL);
}

const ServerConfig	*Server::selectServerConfig(
	int clientFd,
	const HttpRequest& request) const
{
	std::map<int, const ListeningSocket*>::const_iterator	it =
		_clientListeners.find(clientFd);

	if (it == _clientListeners.end() || it->second == NULL)
		return (NULL);

	const ListeningSocket	*listener = it->second;

	if (listener->configs.empty())
		return (NULL);

	std::string	host = request.getHeader("Host");

	std::size_t	colon = host.find(':');
	if (colon != std::string::npos)
		host = host.substr(0, colon);

	for (std::size_t i = 0; i < listener->configs.size(); ++i)
	{
		if (listener->configs[i]->getServerName() == host)
			return (listener->configs[i]);
	}

	return (listener->configs[0]);
}

const ServerConfig	*Server::getClientConfig(int clientFd) const
{
	std::map<int, const ServerConfig*>::const_iterator	it =
		_clientConfigs.find(clientFd);

	if (it == _clientConfigs.end())
		return (NULL);

	return (it->second);
}

MethodHandler	*Server::getClientHandler(int clientFd)
{
	const ServerConfig	*config = getClientConfig(clientFd);

	if (config == NULL)
		return (NULL);

	std::map<const ServerConfig*, MethodHandler*>::iterator	it =
		_handlers.find(config);

	if (it == _handlers.end())
		return (NULL);

	return (it->second);
}

void	Server::run()
{
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		throw std::runtime_error("failed to ignore SIGPIPE");

	while (g_running)
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

			const ListeningSocket	*listener = findListeningSocket(fd);

			if (listener != NULL)
			{
				if (revents & (POLLERR | POLLHUP | POLLNVAL))
					throw std::runtime_error("server socket poll error");

				if (revents & POLLIN)
					acceptClient(*listener);

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

void	Server::acceptClient(const ListeningSocket& listener)
{
	if (listener.configs.empty())
		return ;

	while (true)
	{
		int	clientFd = accept(listener.fd, NULL, NULL);

		if (clientFd < 0)
			return ;

		if (!setNonBlocking(clientFd))
		{
			close(clientFd);
			continue ;
		}

		_clients.insert(std::make_pair(clientFd, Client(clientFd)));

		_clientConfigs[clientFd] = listener.configs[0];
		_clientListeners[clientFd] = &listener;

		std::map<int, Client>::iterator clientIt = _clients.find(clientFd);

		if (clientIt != _clients.end())
		{
			std::size_t maxBodySize = listener.configs[0]->getClientMaxBodySize();

			for (std::size_t i = 1; i < listener.configs.size(); ++i)
			{
				if (listener.configs[i]->getClientMaxBodySize() > maxBodySize)
					maxBodySize = listener.configs[i]->getClientMaxBodySize();
			}

			clientIt->second.getRequest().setMaxBodySize(maxBodySize);
		}

		pollfd	client;

		client.fd = clientFd;
		client.events = POLLIN;
		client.revents = 0;

		_fds.push_back(client);
	}
}

void	Server::handleClientRead(Client& client)
{
	char buffer[4096];

	ssize_t	bytes = recv(client.getFd(), buffer, sizeof(buffer), 0);

	if (bytes <= 0)
	{
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
		return ;

	try {
		const ServerConfig	*config = selectServerConfig(client.getFd(), request);

		if (config == NULL)
		{
			markClientForRemoval(client.getFd());
			return ;
		}

		_clientConfigs[client.getFd()] = config;

		if (request.getBody().size() > config->getClientMaxBodySize())
		{
			HttpResponse response = buildErrorResponse(413);

			client.appendToWriteBuffer(response.serialize());
			enableClientWrite(client.getFd());
			return ;
		}
		
		const LocationConfig	*location = config->findLocation(request.getUri());

		if (location != NULL && !location->getAllowedMethods().empty() && !location->isMethodAllowed(request.getMethod()))
		{
			HttpResponse	response = buildErrorResponse(405);

			client.appendToWriteBuffer(response.serialize());
			enableClientWrite(client.getFd());
			return ;
		}

		if (location != NULL && CgiHandler::isCgiRequest(request.getUri(), *location))
		{
			if (!startCgiProcess(client, *location))
			{
				HttpResponse	response = buildErrorResponse(500);

				client.appendToWriteBuffer(response.serialize());
				enableClientWrite(client.getFd());
			}
			else
				disableClientEvents(client.getFd());

			return ;
		}

		if (location == NULL)
		{
			HttpResponse	response = buildErrorResponse(404);

			client.appendToWriteBuffer(response.serialize());
			enableClientWrite(client.getFd());
			return ;
		}

		MethodHandler	*handler = getClientHandler(client.getFd());

		if (handler == NULL)
		{
			markClientForRemoval(client.getFd());
			return ;
		}

		HttpResponse	response = handler->handle(request);

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

	if (bytesSent <= 0)
	{
		markClientForRemoval(client.getFd());
		return ;
	}
	
	client.updateActivity();
	client.removeSentBytes(static_cast<std::size_t>(bytesSent));

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
		it != _clientsToRemove.end(); ++it)
	{
		if (*it == fd)
			return true;
	}

	return false;
}

void Server::removeMarkedClients()
{
	for (std::vector<int>::const_iterator it = _clientsToRemove.begin();
		it != _clientsToRemove.end(); ++it)
		removeClient(*it);

	_clientsToRemove.clear();
}

void	Server::removeClient(int fd)
{
	// std::cout << "Client disconnected: " << fd << std::endl;

	removeCgiProcess(fd);

	close(fd);
	_clients.erase(fd);
	_clientConfigs.erase(fd);
	_clientListeners.erase(fd);

	removePollFd(fd);
}

void Server::checkClientTimeouts()
{
	std::time_t	now = std::time(NULL);

	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (_cgiProcesses.find(it->first) != _cgiProcesses.end())
			continue ;

		if (now - it->second.getLastActivity() >= _CLIENT_TIMEOUT)
			markClientForRemoval(it->first);
	}
}

void	Server::checkCgiProcesses()
{
	std::map<int, CgiProcess*>::iterator	it = _cgiProcesses.begin();
	std::time_t								now = std::time(NULL);

	while (it != _cgiProcesses.end())
	{
		int			clientFd = it->first;
		CgiProcess	*process = it->second;

		++it;

		if (process == NULL)
		{
			removeCgiProcess(clientFd);
			continue ;
		}

		process->checkProcessStatus();

		if (!process->isFinished() && process->getStartTime() != 0
			&& now - process->getStartTime() >= _CGI_TIMEOUT)
		{
			std::map<int, Client>::iterator	clientIt = _clients.find(clientFd);

			if (clientIt != _clients.end())
			{
				HttpResponse	response = buildErrorResponse(500);

				clientIt->second.appendToWriteBuffer(response.serialize());
				enableClientWrite(clientFd);
			}

			removeCgiProcess(clientFd);
			continue ;
		}

		if (!process->isFinished())
			continue ;

		std::map<int, Client>::iterator	clientIt = _clients.find(clientFd);

		if (clientIt == _clients.end())
		{
			removeCgiProcess(clientFd);
			continue ;
		}

		HttpResponse	response;

		if (!process->exitedNormally() || process->getExitStatus() != 0)
			response = buildErrorResponse(500);

		else if (!buildCgiResponse(process->getOutput(), response))
			response = buildErrorResponse(500);

		clientIt->second.appendToWriteBuffer(response.serialize());
		enableClientWrite(clientFd);

		removeCgiProcess(clientFd);
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

	std::map<int, CgiProcess*>::iterator	processIt = _cgiProcesses.find(clientFd);

	if (processIt == _cgiProcesses.end())
	{
		removePollFd(fd);
		_cgiStdinFds.erase(stdinIt);
		return ;
	}

	CgiProcess	*process = processIt->second;

	if (revents & (POLLERR | POLLHUP | POLLNVAL))
	{
		process->closeInput();
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

	std::map<int, CgiProcess*>::iterator	processIt = _cgiProcesses.find(clientFd);

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

	if ((revents & POLLIN))
	{
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

		return ;
	}

	if (revents & POLLHUP)
	{
		process->closeOutput();
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

bool	Server::buildCgiResponse(const std::string& output,
	HttpResponse& response)
{
	std::string::size_type	separatorPos = output.find("\r\n\r\n");
	std::size_t				separatorLength = 4;

	if (separatorPos == std::string::npos)
	{
		separatorPos = output.find("\n\n");
		separatorLength = 2;
	}

	if (separatorPos == std::string::npos)
		return (false);

	std::string	headersPart = output.substr(0, separatorPos);
	std::string	body = output.substr(separatorPos + separatorLength);
	std::string	contentType;

	std::istringstream	headersStream(headersPart);
	std::string			line;

	while (std::getline(headersStream, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		std::string::size_type	colonPos = line.find(':');

		if (colonPos == std::string::npos)
			return (false);

		std::string	name = line.substr(0, colonPos);
		std::string	value = line.substr(colonPos + 1);

		while (!name.empty() && (name[name.size() - 1] == ' '
			|| name[name.size() - 1] == '\t'))
			name.erase(name.size() - 1);

		while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
			value.erase(0, 1);

		if (name.empty())
			return (false);

		std::string	lowerName = name;

		for (std::size_t i = 0; i < lowerName.size(); ++i)
		{
			lowerName[i] = static_cast<char>(
				std::tolower(static_cast<unsigned char>(lowerName[i])));
		}

		if (lowerName == "status")
		{
			std::istringstream	statusStream(value);
			int					statusCode;

			if (!(statusStream >> statusCode) || statusCode < 100 || statusCode > 599)
				return (false);

			response.setStatus(statusCode);
		}
		else if (lowerName == "content-type")
			contentType = value;
		else if (lowerName != "content-length")
			response.setHeader(name, value);
	}

	if (contentType.empty())
		return (false);

	response.setBody(body, contentType);

	return (true);
}

void	Server::removeCgiProcess(int clientFd)
{
	std::map<int, CgiProcess*>::iterator	processIt = _cgiProcesses.find(clientFd);

	if (processIt == _cgiProcesses.end())
		return ;

	CgiProcess	*process = processIt->second;

	if (process != NULL)
	{
		int	stdinFd = process->getStdinFd();
		int	stdoutFd = process->getStdoutFd();

		if (stdinFd != -1)
		{
			removePollFd(stdinFd);
			_cgiStdinFds.erase(stdinFd);
		}

		if (stdoutFd != -1)
		{
			removePollFd(stdoutFd);
			_cgiStdoutFds.erase(stdoutFd);
		}

		delete process;
	}

	_cgiProcesses.erase(processIt);
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
