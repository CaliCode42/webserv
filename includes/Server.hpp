/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:16:04 by tcali             #+#    #+#             */
/*   Updated: 2026/07/31 16:28:54 by tcali            ###   ########.fr       */
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
#include "Client.hpp"
#include "../http/MethodHandler.hpp"
#include "../http/HttpRequest.hpp"
#include "ServerConfig.hpp"

class Server
{
private:
	int						_port;
	int						_serverSocket;
	std::vector<pollfd>		_fds;
	std::vector<int>		_clientsToRemove;
	std::map<int, Client>	_clients;
	ServerConfig			_config;
	MethodHandler			_handler;

public:
	// Server(int port);
	Server(int port, const ServerConfig& config);
	// Server(const Server& other);
	// Server& operator=(const Server& other);
	~Server();

	void	initSocket();

	void	run();

	void	acceptClient();

	void	handleClientRead(Client& client);
	void	handleClientWrite(Client& client);

	void	enableClientWrite(int fd);
	void	disableClientWrite(int fd);

	void	markClientForRemoval(int fd);
	bool	isMarkedForRemoval(int fd)const;
	void	removeMarkedClients();
	void	removeClient(int fd);
};

bool	setNonBlocking(int fd);

#endif