/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:40:20 by tcali             #+#    #+#             */
/*   Updated: 2026/07/29 19:20:58 by tcali            ###   ########.fr       */
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

const std::string&	Client::getReadBuffer() const
{
	return (_readBuffer);
}

const std::string&	Client::getWriteBuffer() const
{
	return (_writeBuffer);
}

void		Client::appendToReadBuffer(const std::string &data)
{
	_readBuffer += data;
}

void		Client::appendToWriteBuffer(const std::string &data)
{
	_writeBuffer += data;
}

bool Client::hasPendingWriteData() const
{
    return (!_writeBuffer.empty());
}

void Client::removeSentBytes(std::size_t count)
{
    if (count >= _writeBuffer.size())
        _writeBuffer.clear();
    else
        _writeBuffer.erase(0, count);
}

void Client::setRequest(const HttpRequest& request)
{
    _request = request;
}

// hasCompleteRequest et extractRequest ne gèrent pour l'instant que les requêtes GET,
// il faudra les mettre à jour pour gérer les requêtes POST.
bool			Client::hasCompleteRequest() const
{
	return (_readBuffer.find("\r\n\r\n") != std::string::npos);
}

std::string		Client::extractRequest()
{
	size_t	end = _readBuffer.find("\r\n\r\n");

    if (end == std::string::npos)
        return ("");

    std::string	request = _readBuffer.substr(0, end + 4);

    _readBuffer.erase(0, end + 4);

    return (request);
}
