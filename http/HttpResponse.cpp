/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdossa <sdossa@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 23:52:39 by sdossa            #+#    #+#             */
/*   Updated: 2026/09/07 00:44:08 by sdossa           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include <sstream>

HttpResponse::HttpResponse() : _status(200)
{}

HttpResponse::~HttpResponse()
{}

void HttpResponse::setStatus(int code)
{
	_status = code;
} 

void HttpResponse::setBody(const std::string& body, const std::string& contentType)
{
	_body = body;
	setHeader("Content-Type", contentType);
}

void HttpResponse::setHeader(const std::string& key, const std::string&  value)
{
	//replace existing header intead of duplicating it
	for (size_t i = 0; i < _headers.size(); ++i)
	{
		if (_headers[i].first == key)
		{
			_headers[i].second = value;
			return;
		}
	}
	_headers.push_back(std::make_pair(key, value));
}

std::string HttpResponse::serialize() const
{
	std::ostringstream out;

	out << "HTTP/1.1 " << _status << " " << reasonPhrase(_status) << "\r\n";
	out << "Content-Length: " << _body.size() << "\r\n";
	out << "Server: webserv/1.0\r\n";

	for (size_t i = 0; i < _headers.size(); ++i)
		out << _headers[i].first << ": " << _headers[i].second << "\r\n";

	out << "\r\n";
	out << _body;
	
	return out.str();
}

std::string HttpResponse::reasonPhrase(int code)
{
	switch (code)
	{
		case 200: return "OK";
		case 201: return "Created";
		case 204: return "No Content";
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 400: return "Bad Request";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 413: return "Payload Too Large";
		case 414: return "URI Too Long";
		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 505: return "HTTP Version Not Supported";
		default:  return "Unknown";
	}
}