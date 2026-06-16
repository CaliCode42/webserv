/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:16:17 by tcali             #+#    #+#             */
/*   Updated: 2026/06/16 12:16:25 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include "HttpRequest.hpp"

class Client
{
private:
	int			_fd;
	std::string	_buffer;
	HttpRequest _request;
	bool		_isComplete = false;

public:

	Client();
	Client(int fd);
	// Client(const Client& other);
	// Client& operator=(const Client& other);
	~Client();

	int			getFd() const;

	std::string &getBuffer();
	
	void		appendToBuffer(const std::string &data);
	
	void		clearBuffer();

	void		setRequest(HttpRequest &request);

	bool		hasCompleteRequest() const;

	std::string	extractRequest();
};

#endif