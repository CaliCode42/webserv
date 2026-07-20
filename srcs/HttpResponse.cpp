/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:42:55 by tcali             #+#    #+#             */
/*   Updated: 2026/07/20 14:09:33 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include "Utils.hpp"

HttpResponse::HttpResponse() : _statusCode(0)
{
	// std::cout << "[HttpResponse] Default constructor called" << std::endl;
}

// HttpResponse::HttpResponse(const HttpResponse& other)
// {
// 	std::cout << "[HttpResponse] Copy constructor called" << std::endl;
// 	*this = other;
// }

// HttpResponse& HttpResponse::operator=(const HttpResponse& other)
// {
// 	if (this != &other)
// 	{
// 		// copy attributes here
// 	}
// 	std::cout << "[HttpResponse] Copy assignment operator called" << std::endl;
// 	return (*this);
// }

HttpResponse::~HttpResponse()
{
	// std::cout << "[HttpResponse] Destructor called" << std::endl;
}

void		HttpResponse::setStatus(int code, const std::string& message)
{
	_statusCode = code;
	_statusMessage = message;
}

void		HttpResponse::setHeader(const std::string& key, const std::string& value)
{
	_headers[key] = value;
}

void		HttpResponse::setBody(const std::string& body)
{
	_body = body;
}

std::string	HttpResponse::toString() const
{
	// Add status line at the beginning of the response
	std::string	response;

	response += "HTTP/1.1 ";
	response += turnIntoString(_statusCode);
	response += " ";
	response += _statusMessage;
	response += "\r\n";

	// Add headers 
	std::map<std::string, std::string>::const_iterator it;

	for (it = _headers.begin(); it != _headers.end(); ++it)
	{
		response += it->first;
		response += ": ";
		response += it->second;
		response += "\r\n";
	}

	// Add Content-Length
	response += "Content-Length: ";
	response += turnIntoString(_body.size());
	response += "\r\n";
	
	// Add empty line before body
	response += "\r\n";

	// Add body
	response += _body;

	return (response);
}