/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdossa <sdossa@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 22:09:59 by sdossa            #+#    #+#             */
/*   Updated: 2026/08/02 15:07:07 by sdossa           ###   ########.fr       */
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

bool HttpRequest::parse(const std::string& raw)
{
	
	std::string::size_type eol = raw.find("\r\n");
	if (eol == std::string::npos)
		return false;
    std::string line = raw.substr(0, eol);

	std::istringstream iss(line);
	std::string extra;
	if (!(iss >> _method >> _uri >> _version))
		return false;
	if (iss >> extra)
		return false;
	
	if (_method != "GET" && _method != "POST" && _method != "DELETE")
		return false;

	std::string::size_type pos = eol + 2; //saute 1er \r\n
	while (pos < raw.size())
	{
		std::string::size_type end = raw.find("\r\n", pos);
		if (end == std::string::npos) 
			break;
		std::string header = raw.substr(pos, end - pos);
		if (header.empty()) 
			break;// ligne vide dc fin headers ^^

		std::string::size_type colon = header.find(':');
		if (colon == std::string::npos)
			return false;
			
		std::string key = toLower(header.substr(0, colon));
		std::string val = trim(header.substr(colon + 1)); // saute ":"
		_headers[key] = val; 
		
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

std::string HttpRequest::trim(const std::string& s)
{
	std::string::size_type a = s.find_first_not_of(" \t");
	if (a == std::string::npos)
		return "";
	std::string::size_type b = s.find_last_not_of(" \t");
	return s.substr(a, b - a + 1);
}