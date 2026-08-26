/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:16:17 by tcali             #+#    #+#             */
/*   Updated: 2026/08/24 19:29:25 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include <ctime>
#include "HttpRequest.hpp"

class	Client
{
private:
	int							_fd;
	std::string					_writeBuffer;
	HttpRequest 				_request;
	std::time_t					_lastActivity;

public:

	Client() : _fd(-1), _lastActivity(std::time(NULL)) {};
	Client(int fd) : _fd(fd), _lastActivity(std::time(NULL)) {}
	~Client() {};

	int					getFd() const;
	const std::string&	getWriteBuffer()const;
	HttpRequest&		getRequest();
	std::time_t 		getLastActivity() const;
	
	void				appendToWriteBuffer(const std::string &data);
	
	bool 				hasPendingWriteData() const;
	void				removeSentBytes(std::size_t count);

	void        		updateActivity();
};

#endif