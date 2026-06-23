/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:16:25 by tcali             #+#    #+#             */
/*   Updated: 2026/06/22 17:14:47 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <iostream>
#include <map>

class HttpRequest
{
private:
	

public:
	std::string	_method;
	std::string	_path;
	std::string	_version;
	std::string	_host;

	std::map<std::string, std::string> _headers;

	std::string _body;

	HttpRequest();
	// HttpRequest(const HttpRequest& other);
	// HttpRequest& operator=(const HttpRequest& other);
	~HttpRequest();

	HttpRequest parseRequest(const std::string& raw);
};

#endif