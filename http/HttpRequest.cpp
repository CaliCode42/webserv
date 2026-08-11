/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdossa <sdossa@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 22:09:59 by sdossa            #+#    #+#             */
/*   Updated: 2026/08/06 14:51:05 by sdossa           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cctype>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include "HttpRequest.hpp"
#include <sstream>

HttpRequest::HttpRequest()
	: _state(STATE_REQUEST_LINE), _errorCode(0), _contentLength(0), _chunkSize(0)
		
{}


void HttpRequest::appendData(const char* data, size_t len)
{
	if (_state == STATE_COMPLETE || _state == STATE_ERROR)
		return;
	
	_buffer.append(data, len);
	
	bool progress = true;
	while (progress)
	{
		switch (_state)
		{
			case STATE_REQUEST_LINE: progress = parseRequestLine();
				break;
			case STATE_HEADERS: progress = parseHeaders();
				break;
			case STATE_BODY: progress = parseBody();
				break;
			case STATE_CHUNK_DATA: progress = parseChunkData();
				break;
			case STATE_CHUNK_SIZE: progress = parseChunkSize();
				break;
			default: progress = false;
				break;
		}
	}
}

// "GET /index.html HTTP/1.1\r\n"
bool HttpRequest::parseRequestLine()
{
	
	std::string::size_type eol = _buffer.find("\r\n");
	if (eol == std::string::npos)
	{
		if (_buffer.size() > 8192)
			setError(414); // URI too long 
		return false;
	}
		
 	std::string line = _buffer.substr(0, eol);
	_buffer.erase(0, eol + 2); //enlève le "GET /..\r\n"

	std::istringstream iss(line);
	std::string extra;
	if (!(iss >> _method >> _uri >> _version) || (iss >> extra))
	{
		setError(400);// CHECK
		return false;
	}
	
	if (_method != "GET" && _method != "POST" && _method != "DELETE")
	{
		setError(501); //CHECK
		return false;
	}
	
	_state = STATE_HEADERS;
	return true;
}



// "Host: localhost\r\n" ... "\r\n"
bool HttpRequest::parseHeaders()
{
	while (true)
	{
		std::string::size_type eol = _buffer.find("\r\n");
		if (eol == std::string::npos) 
		{
			if (_buffer.size() > 32768)
				setError(431); // Header trop large 
			return false;
		}

		std::string line = _buffer.substr(0, eol);
		_buffer.erase(0, eol + 2); //enlève le "\r\n"

		if (line.empty()) // ligne vide dc fin headers ^^
		{
			onHeadersComplete();
			return true;
		}

		std::string::size_type colon = line.find(':');
		if (colon == std::string::npos || colon == 0)
		{
			setError(400); //
			return false;
		}
			
		std::string key = toLower(trim(line.substr(0, colon)));
		std::string val = trim(line.substr(colon + 1)); // saute ":"
		_headers[key] = val; 
	}
}

//Transfer-Encoding: Chunked wins vs Content-length ^^
void HttpRequest::onHeadersComplete()
{
	//T-E CHUNKED
	std::string transferEncoding = getHeader("Transfer-Encoding");
	if (transferEncoding == "chunked")
	{
		_state = STATE_CHUNK_SIZE;
		return;
	}

	//Content-Length
	std::string contentLengthStr = getHeader("Content-Length");
		if (contentLengthStr.empty())
		{
			_state = STATE_COMPLETE;
			return;
		}
		std::istringstream iss(contentLengthStr);
		iss >> _contentLength;
		_state = STATE_BODY;
}

// Sized body: wait til _contentLength bytes are buffered
bool HttpRequest::parseBody()
{
	if (_buffer.size() < _contentLength)
	{
		//pas assez d'octets, on attend
		return false;
	}
	//si assez, on copie
	_body = _buffer.substr(0, _contentLength);
	_buffer.erase(0, _contentLength);
	_state = STATE_COMPLETE;
	return true;
	
}

// Chunked body 
// "1a\r\n" <26 bytes> "\r\n" "0\r\n" "\r\n"
bool HttpRequest::parseChunkSize()
{
	std::string::size_type eol = _buffer.find("\r\n");
	if (eol == std::string::npos)
		return false;
	
	std::string line = _buffer.substr(0, eol);
	
	//if last chunk ="0", need the next 2 octets "\r\n"
	if (line == "0")
	{
		//need "0\r\n\r\n" = 5 octets minimum
		if (_buffer.size() < 5)
			return false;
		//check final \r\n
		if (_buffer.substr(3, 2) != "\r\n")
			return false;
		_buffer.erase(0, 5);
		_chunkSize = 0;
		_state = STATE_COMPLETE;
		return true;
	}

	//extraire line, convertir en hexa
	_buffer.erase(0, eol + 2);
	std::istringstream iss(line);
	iss >> std::hex >> _chunkSize;
	if(iss.fail())
	{
		setError(400);
		return false;
	}
	
	_state = STATE_CHUNK_DATA;
	return true;
}


bool HttpRequest::parseChunkData()
{
	//si buffer trop court, return false
	if (_buffer.size() < _chunkSize + 2)
		return false;
	_body.append(_buffer, 0, _chunkSize);
	_buffer.erase(0, _chunkSize + 2);
	_state = STATE_CHUNK_SIZE;
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

void HttpRequest::setError(int code)
{
	_errorCode = code;
	_state = STATE_ERROR;
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