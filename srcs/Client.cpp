/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:40:20 by tcali             #+#    #+#             */
/*   Updated: 2026/06/25 11:23:54 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client()
    : _fd(-1)
{
}

Client::Client(int fd) : _fd(fd), _isComplete(false)
{
	// std::cout << "[Client] Default constructor called" << std::endl;
}

// Client::Client(const Client& other)
// {
// 	std::cout << "[Client] Copy constructor called" << std::endl;
// 	*this = other;
// }

// Client& Client::operator=(const Client& other)
// {
// 	if (this != &other)
// 	{
// 		// copy attributes here
// 	}
// 	std::cout << "[Client] Copy assignment operator called" << std::endl;
// 	return (*this);
// }

Client::~Client()
{
	// std::cout << "[Client] Destructor called" << std::endl;
}

int			Client::getFd() const
{
	return (_fd);
}

std::string&	Client::getBuffer()
{
	return (_buffer);
}

void		Client::appendToBuffer(const std::string &data)
{
	_buffer += data;
}

void		Client::clearBuffer()
{
	_buffer.clear();
}
