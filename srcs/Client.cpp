/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:40:20 by tcali             #+#    #+#             */
/*   Updated: 2026/08/14 12:25:08 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client() : _fd(-1), _lastActivity(std::time(NULL)) {}

Client::Client(int fd) : _fd(fd), _lastActivity(std::time(NULL)) {}

Client::~Client() {}

int	Client::getFd() const
{
	return (_fd);
}

const std::string&	Client::getWriteBuffer() const
{
	return (_writeBuffer);
}

HttpRequest&	Client::getRequest()
{
	return (_request);
}

void	Client::appendToWriteBuffer(const std::string &data)
{
	_writeBuffer += data;
}

bool	Client::hasPendingWriteData() const
{
	return (!_writeBuffer.empty());
}

void	Client::removeSentBytes(std::size_t count)
{
	if (count >= _writeBuffer.size())
		_writeBuffer.clear();
	else
		_writeBuffer.erase(0, count);
}

void	Client::updateActivity()
{
	_lastActivity = std::time(NULL);
}

std::time_t	Client::getLastActivity() const
{
	return (_lastActivity);
}
