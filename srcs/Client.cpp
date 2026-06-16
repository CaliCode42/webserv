/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:40:20 by tcali             #+#    #+#             */
/*   Updated: 2026/06/15 16:29:58 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(int fd) : _fd(fd)
{
	std::cout << "[Client] Default constructor called" << std::endl;
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
	std::cout << "[Client] Destructor called" << std::endl;
}

int			Client::getFd()const
{
	return (_fd);
}

std::string &getBuffer()
{
	return (_buffer);
}

void		appendToBuffer(const std::string &data)
{
	_buffer += data;
}

void		clearBuffer()
{
	_buffer.clear();
}
