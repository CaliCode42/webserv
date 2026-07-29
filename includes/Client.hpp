/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:16:17 by tcali             #+#    #+#             */
/*   Updated: 2026/07/29 17:03:04 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include "HttpRequest.hpp"

enum	Buffer {READ, WRITE};

class	Client
{
private:
	int			_fd;
	std::string	_readBuffer;
	std::string	_writeBuffer;
	HttpRequest _request;
	bool		_isComplete;

public:

	Client();
	Client(int fd);
	// Client(const Client& other);
	// Client& operator=(const Client& other);
	~Client();

	int					getFd() const;

	const std::string&	getReadBuffer()const;

	const std::string&	getWriteBuffer()const;
	
	void				appendToReadBuffer(const std::string &data);
	
	void				appendToWriteBuffer(const std::string &data);
	
	bool 				hasPendingWriteData() const;
    void				removeSentBytes(std::size_t count);

	void				setRequest(const HttpRequest &request);

	bool				hasCompleteRequest() const;
	std::string			extractRequest();
};

#endif