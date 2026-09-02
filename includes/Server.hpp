/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:16:04 by tcali             #+#    #+#             */
/*   Updated: 2026/09/01 14:49:36 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <exception>
#include <iostream>
#include <vector>
#include <map>
#include <sys/poll.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sstream>
#include <cctype>
#include <signal.h>

#include "Client.hpp"
#include "MethodHandler.hpp"
#include "HttpRequest.hpp"
#include "ServerConfig.hpp"
#include "CgiProcess.hpp"
#include "CgiHandler.hpp"

extern sig_atomic_t g_running;

struct ListeningSocket
{
	int									fd;
	unsigned int						port;
	std::vector<const ServerConfig*>	configs;
};

class Server
{
private:
	std::vector<ServerConfig>						_configs;
	std::vector<ListeningSocket>					_listeningSockets;

	std::vector<pollfd>								_fds;
	std::vector<int>								_clientsToRemove;
	std::map<int, Client>							_clients;
	std::map<int, const ServerConfig*>				_clientConfigs;
	std::map<int, const ListeningSocket*>	_clientListeners;

	std::map<int, CgiProcess*>						_cgiProcesses;
	std::map<int, int>								_cgiStdinFds;
	std::map<int, int>								_cgiStdoutFds;

	std::map<const ServerConfig*, MethodHandler*>	_handlers;

	static const std::time_t						_CLIENT_TIMEOUT = 10;
	static const std::time_t						_POLL_TIMEOUT = 1000;
	static const std::time_t						_CGI_TIMEOUT = 5;

	HttpResponse	buildErrorResponse(int statusCode);

public:
	Server(const std::vector<ServerConfig>& configs);
	~Server();

	int						createListeningSocket(unsigned int port);
	void					initSockets();
	const ListeningSocket	*findListeningSocket(int fd) const;
	ListeningSocket			*findListeningSocketByPort(unsigned int port);

	const ServerConfig		*selectServerConfig(int clientFd, const HttpRequest& request) const;
	
	const ServerConfig		*getClientConfig(int clientFd) const;
	MethodHandler			*getClientHandler(int clientFd);

	void					run();

	void					acceptClient(const ListeningSocket& listener);

	void					handleClientRead(Client& client);
	void					handleClientWrite(Client& client);

	void					enableClientWrite(int fd);
	void					disableClientWrite(int fd);
	void					disableClientEvents(int fd);

	void					markClientForRemoval(int fd);
	bool					isMarkedForRemoval(int fd)const;
	void					removeMarkedClients();
	void					removeClient(int fd);

	void					checkClientTimeouts();
	void					checkCgiProcesses();

	void					handleCgiEvents(int fd, short revents);
	void					handleCgiStdinEvent(int fd, short revents);
	void					handleCgiStdoutEvent(int fd, short revents);

	bool					isCgiStdinFd(int fd) const;
	bool					isCgiStdoutFd(int fd) const;

	void					addPollFd(int fd, short events);
	void					removePollFd(int fd);

	bool					startCgiProcess(Client& client, const LocationConfig& location);
	bool					buildCgiResponse(const std::string& output, HttpResponse& response);
	void					removeCgiProcess(int clientFd);
};

#endif