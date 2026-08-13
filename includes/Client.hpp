/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:16:17 by tcali             #+#    #+#             */
/*   Updated: 2026/08/13 17:50:18 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include "HttpRequest.hpp"

class	Client
{
private:
	int			_fd;
	std::string	_writeBuffer;
	HttpRequest _request;

public:

	Client();
	Client(int fd);
	// Client(const Client& other);
	// Client& operator=(const Client& other);
	~Client();

	int					getFd() const;

	const std::string&	getWriteBuffer()const;

	HttpRequest&		getRequest();
	
	void				appendToWriteBuffer(const std::string &data);
	
	bool 				hasPendingWriteData() const;
    void				removeSentBytes(std::size_t count);
};

#endif