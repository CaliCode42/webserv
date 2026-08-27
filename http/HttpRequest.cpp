/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 22:09:59 by sdossa            #+#    #+#             */
/*   Updated: 2026/08/27 16:30:54 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cctype>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include "HttpRequest.hpp"
#include <sstream>

HttpRequest::HttpRequest()
	: _state(STATE_REQUEST_LINE), _errorCode(0), _contentLength(0), _chunkSize(0), _maxBodySize(static_cast<std::size_t>(-1))	
{}

HttpRequest::~HttpRequest()
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
			setError(414); // URI Too Large
		return false;
	}
		
 	std::string line = _buffer.substr(0, eol);
	_buffer.erase(0, eol + 2); //remove "GET /..\r\n"

	std::istringstream iss(line);
	std::string extra;
	if (!(iss >> _method >> _uri >> _version) || (iss >> extra))
	{
		setError(400);// Bad Request
		return false;
	}
	
	if (_method != "GET" && _method != "POST" && _method != "DELETE")
	{
		setError(501); //Not Implemented
		return false;
	}
	_state = STATE_HEADERS;
	return true;
}



// "Host: localhost\r\n" ... "\r\n"
bool HttpRequest::parseHeaders()
{
	if (_buffer.size() > MAX_BUFFER_SIZE)
	{
		setError(431); // Header trop large 
		return false;
	}	

	while (true)
	{
		std::string::size_type eol = _buffer.find("\r\n");
		if (eol == std::string::npos) 
		{
			if (_buffer.size() > 32768)
				setError(431); // Header Too Long 
			return false;
		}

		std::string line = _buffer.substr(0, eol);
		_buffer.erase(0, eol + 2); //remove "\r\n"

		if (line.empty()) // end of headers ^^
		{
			onHeadersComplete();
			return true;
		}

		std::string::size_type colon = line.find(':');
		if (colon == std::string::npos || colon == 0)
		{
			setError(400);
			return false;
		}
			
		std::string key = toLower(trim(line.substr(0, colon)));
		std::string val = trim(line.substr(colon + 1)); // skip ":"
		if (key == "content-length" && _headers.find(key) != _headers.end())
		{
			setError(400);
			return false;
		}
		_headers[key] = val; 
	}	
}


//Transfer-Encoding: Chunked wins vs Content-length ^^
void HttpRequest::onHeadersComplete()
{
	//T-E CHUNKED
	std::string transferEncoding = getHeader("Transfer-Encoding");
	if (toLower(transferEncoding) == "chunked")
	if (toLower(transferEncoding) == "chunked")
	{
		_headers.erase("content-length"); //delete fantom C-L
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

	if (contentLengthStr[0] == '-')
	{
		setError(400);
		return;
	}

	std::istringstream iss(contentLengthStr);

	iss >> _contentLength;

	if (iss.fail())
	{
		setError(400);
		return;
	}

	char extra;
	if (iss >> extra)
	{
		setError(400);
		return;
	}

	if (_contentLength > _MAX_BODY_SIZE)
	{
		setError(413);
		return;
	}

	_state = STATE_BODY;
}

// Sized body: wait til _contentLength bytes are buffered
bool HttpRequest::parseBody()
{
	if (_buffer.size() < _contentLength)
	{
		//not enough octets = continue
		return false;
	}
	//otherwise copie
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

    // Last chunk
    if (line == "0")
    {
        // Remove "0\r\n"
        _buffer.erase(0, eol + 2);

        // Trailers end with an empty line: "\r\n"
        std::string::size_type trailersEnd = _buffer.find("\r\n\r\n");

        if (trailersEnd != std::string::npos)
        {
            _buffer.erase(0, trailersEnd + 4);
            _chunkSize = 0;
            _state = STATE_COMPLETE;
            return true;
        }

        // No trailer: just the final CRLF
        if (_buffer.size() >= 2 && _buffer.substr(0, 2) == "\r\n")
        {
            _buffer.erase(0, 2);
            _chunkSize = 0;
			//recalculate CL from the real body
			std::ostringstream lenOss;
			lenOss << _body.size();
			_headers["content-length"] = lenOss.str();
            _state = STATE_COMPLETE;
            return true;
        }

        return false;
    }

    std::istringstream iss(line);
    iss >> std::hex >> _chunkSize;

    if (iss.fail())
    {
        setError(400);
        return false;
    }

    char extra;

    if (iss >> extra)
    {
        setError(400);
        return false;
    }

	if (_body.size() > _MAX_BODY_SIZE || _chunkSize > _MAX_BODY_SIZE - _body.size())
	{
		setError(413);
		return false;
	}

    _buffer.erase(0, eol + 2);
    _state = STATE_CHUNK_DATA;

    return true;
}


bool HttpRequest::parseChunkData()
{
	// si body trop long, return false
	if (_body.size() > _MAX_BODY_SIZE
        || _chunkSize > _MAX_BODY_SIZE - _body.size())
    {
        setError(413);
        return false;
    }
	//si buffer trop court, return false
	if (_buffer.size() < _chunkSize + 2)
		return false;
	
	if (_buffer.substr(_chunkSize, 2) != "\r\n")
    {
        setError(400);
        return false;
    }
	
	_body.append(_buffer, 0, _chunkSize);
	_buffer.erase(0, _chunkSize + 2);
	if (_body.size() > _maxBodySize)
	{
		setError(413);
		return false;
	}
	_state = STATE_CHUNK_SIZE;
	return true;
}

// GETTERS AND HELPERS
std::string HttpRequest::getHeader(const std::string& key) const 
{
	std::map<std::string, std::string>::const_iterator headerIt = _headers.find(toLower(key));
	if (headerIt == _headers.end())
		return "";
	return headerIt->second;
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
	std::string::size_type start = s.find_first_not_of(" \t");
	if (start == std::string::npos)
		return "";
	std::string::size_type end = s.find_last_not_of(" \t");
	return s.substr(start, end - start + 1);
}


//Cookies
std::map<std::string, std::string> HttpRequest::getCookies() const
{
	std::string line = getHeader("Cookie");
	std::map<std::string, std::string> cookies;
	
	std::string::size_type pos = 0;	
	while (pos < line.size())
	{
		std::string::size_type end = line.find(";", pos);
		if (end == std::string::npos)
			end = line.size();
		
		std::string pair = trim(line.substr(pos, end - pos));
		std::string::size_type eq = pair.find("=");
		 
		if (eq != std::string::npos)
		{
			std::string key = trim(pair.substr(0, eq));
			std::string val = trim(pair.substr(eq + 1)); // skip "="
			cookies[key] = val;
		}
		pos = end + 1;	
	}
	return cookies;
}





