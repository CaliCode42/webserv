/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:33:43 by tcali             #+#    #+#             */
/*   Updated: 2026/05/29 18:40:05 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(int serverSocket) : _serverSocket(serverSocket)
{
	std::cout << "[Server] Default constructor called" << std::endl;
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
	for (int i = _fds.size(); i >= 0; i--)
		_fds.erase(_fds[i]);
}
