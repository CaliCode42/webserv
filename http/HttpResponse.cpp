/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdossa <sdossa@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 23:52:39 by sdossa            #+#    #+#             */
/*   Updated: 2026/07/27 03:36:26 by sdossa           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include <sstream>


HttpResponse::HttpResponse() : _status(200) {}

void HttpResponse::setStatus(int code) { _status = code; } 

void HttpResponse::setBody(const std::string& body, const std::string& contentType)
{
    _body = body;
    addHeader("Content-Type", contentType);
}

void HttpResponse::addHeader(const std::string& key, const std::string&  value)
{
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
        case 400: return "Bad Request";
        case 404: return "Not Found";
        default:  return "Unknown";
        
    }
}