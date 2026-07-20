/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:16:35 by tcali             #+#    #+#             */
/*   Updated: 2026/07/20 13:51:21 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <iostream>
#include <map>

class HttpResponse
{
private:
	int									_statusCode;
	std::string 						_statusMessage;
	std::map<std::string, std::string>	_headers;
	std::string							_body;

public:
	HttpResponse();
	// HttpResponse(const HttpResponse& other);
	// HttpResponse& operator=(const HttpResponse& other);
	~HttpResponse();

	void		setStatus(int code, const std::string& message);
    void		setHeader(const std::string& key, const std::string& value);
    void		setBody(const std::string& body);

	std::string	toString() const;
};

#endif