/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:41:34 by tcali             #+#    #+#             */
/*   Updated: 2026/06/16 12:09:00 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

HttpRequest::HttpRequest()
{
	std::cout << "[HttpRequest] Default constructor called" << std::endl;
}

// HttpRequest::HttpRequest(const HttpRequest& other)
// {
// 	std::cout << "[HttpRequest] Copy constructor called" << std::endl;
// 	*this = other;
// }

// HttpRequest& HttpRequest::operator=(const HttpRequest& other)
// {
// 	if (this != &other)
// 	{
// 		// copy attributes here
// 	}
// 	std::cout << "[HttpRequest] Copy assignment operator called" << std::endl;
// 	return (*this);
// }

HttpRequest::~HttpRequest()
{
	std::cout << "[HttpRequest] Destructor called" << std::endl;
}

HttpRequest parseRequest(const std::string& raw)
{
	HttpRequest	req;

	size_t pos = raw.find("\r\n\r\n");
	if (pos == std::string::npos)
		return (req);

	std::string 		headerPart = raw.substr(0, pos);

	std::istringstream	stream(headerPart);
	std::string			line;

	std::getline(stream, line);
	{
		std::istringstream requestLine(line);
		requestLine >> req.method >> req.path >> req.version;
	}

	while (std::getline(stream, line))
	{
		size_t sep = line.find(":");
		if (sep != std::string::npos)
		{
			std::string key = line.substr(0, sep);
			std::string value = line.substr(sep + 1);

			if (!value.empty() && value[0] == ' ')
				value.erase(0, 1);

			req.headers[key] = value;
		}
	}

	return (req);
}