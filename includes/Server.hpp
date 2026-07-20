/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:16:04 by tcali             #+#    #+#             */
/*   Updated: 2026/07/20 13:58:07 by tcali            ###   ########.fr       */
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
#include "RequestHandler.hpp"

class Server
{
private:
	int						_port;
	int						_serverSocket;
	std::vector<pollfd>		_fds;
	std::map<int, Client>	_clients;
	RequestHandler			_handler;

public:
	Server(int port);
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