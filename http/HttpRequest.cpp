/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdossa <sdossa@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 22:09:59 by sdossa            #+#    #+#             */
/*   Updated: 2026/07/27 00:47:08 by sdossa           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cctype>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include "HttpRequest.hpp"
#include <sstream>

// "GET /index.html HTTP/1.1\r\n"
// "GET /path?query HTTP/1.1\r\n"

bool HttpRequest::parseRequest(const std::string& raw)
{
	
	std::string::size_type eol = raw.find("\r\n");
    std::string line = raw.substr(0, eol);

	std::istringstream iss(line);
	std::string extra;
	if (!(iss >> _method >> _uri >> _version))
		return false;
	if (iss >> extra)
		return false;

	std::string::size_type pos = eol +2; //saute 1er \r\n
	while (pos < raw.size())
	{
		std::string::size_type end = raw.find("\r\n", pos);
		if (end == std::string::npos) break;
		std::string header = raw.substr(pos, end - pos);
		if (header.empty()) break;// ligne vide dc fin headers ^^

		std::string::size_type colon = header.find(':');
		if (colon != std::string::npos)
		{
			std::string key = header.substr(0, colon);
			std::string val = header.substr(colon + 2); // saute ":"
			_headers[key] = val; 
		}
		pos = end + 2;
	}
	return true;
}

// GETTERS AND HELPERS
std::string HttpRequest::getHeader(const std::string& key) const 
{
	std::map<std::string, std::string>::const_iterator it = _headers.find(toLower(key));
	if (it == _headers.end())
		return "";
	return it->second;
}

std::string HttpRequest::toLower(const std::string& s)
{
	std::string out(s);
	for (size_t i = 0; i < out.size(); ++i)
		out[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(out[i])));
	return out;
}