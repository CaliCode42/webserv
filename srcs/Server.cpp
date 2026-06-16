/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:33:43 by tcali             #+#    #+#             */
/*   Updated: 2026/06/16 18:11:54 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

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
	if (serverSocket == -1)
        throw std::runtime_error("failed to init socket.");

	sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(_serverSocket, (sockaddr*)&addr, sizeof(addr)) == -1)
        throw std::runtime_error("failed to bind socket.");

    if (listen(_serverSocket, 10) == -1)
        throw std::runtime_error("socket failed to listen.");

	pollfd pfd;

	pfd.fd = _serverSocket;
	pfd.events = POLLIN;
	pfd.revents = 0;

	_fds.push_back(pfd);
}
