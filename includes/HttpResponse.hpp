/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:16:35 by tcali             #+#    #+#             */
/*   Updated: 2026/05/29 18:43:06 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <iostream>

class HttpResponse
{
private:
	

public:
	int			_statusCode;
	std::string _body;

	std::map<std::string, std::string> _headers;

	std::string toString();

	HttpResponse();
	// HttpResponse(const HttpResponse& other);
	// HttpResponse& operator=(const HttpResponse& other);
	~HttpResponse();
};

#endif