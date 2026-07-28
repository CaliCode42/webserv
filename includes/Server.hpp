/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdossa <sdossa@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:16:04 by tcali             #+#    #+#             */
/*   Updated: 2026/07/28 16:37:17 by sdossa           ###   ########.fr       */
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

	void	removeClient(int fd);
};

#endif