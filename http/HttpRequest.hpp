/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdossa <sdossa@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 22:09:53 by sdossa            #+#    #+#             */
/*   Updated: 2026/07/29 04:38:23 by sdossa           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>


class HttpRequest
{
public:
	bool parse(const std::string& raw);

	const std::string& getMethod()  const { return _method; } //GET
	const std::string& getUri()     const { return _uri; } //index.html
	const std::string& getVersion() const { return _version; } //HTTP/1.1
	
	std::string getHeader(const std::string& key) const;
	const std::map<std::string, std::string>& getHeaders() const { return _headers; }

private:
	std::string _method;
	std::string _uri;
	std::string _version;
	std::map<std::string, std::string> _headers;
	std::string _body;
	
	static std::string toLower(const std::string& s);
	static std::string trim(const std::string& s);

};

#endif