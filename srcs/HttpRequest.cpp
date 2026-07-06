/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:41:34 by tcali             #+#    #+#             */
/*   Updated: 2026/07/06 14:15:13 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include <sstream>

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

void HttpRequest::parse(const std::string& raw)
{
	size_t pos = raw.find("\r\n\r\n");
	if (pos == std::string::npos)
		return ;

	std::string 		headerPart = raw.substr(0, pos);

	std::istringstream	stream(headerPart);
	std::string			line;

	std::getline(stream, line);
	
	if (!line.empty() && line[line.size() - 1] == '\r')
    	line.erase(line.size() - 1);
	
	std::istringstream requestLine(line);
	requestLine >> _method >> _path >> _version;

	while (std::getline(stream, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		size_t sep = line.find(":");
		if (sep != std::string::npos)
		{
			std::string key = line.substr(0, sep);
			std::string value = line.substr(sep + 1);

			if (!value.empty() && value[0] == ' ')
				value.erase(0, 1);

			_headers[key] = value;
		}
	}

	return ;
}

const std::string&	HttpRequest::getMethod()const
{
	return (_method);
}

const std::string&	HttpRequest::getPath()const
{
	return (_path);
}

const std::string&	HttpRequest::getVersion()const
{
	return (_version);
}

std::string	HttpRequest::getHeader(const std::string& key) const
{
	std::map<std::string, std::string>::const_iterator it = _headers.find(key);

    if (it != _headers.end())
        return (it->second);

    return ("");
}