/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/15 17:14:42 by tcali             #+#    #+#             */
/*   Updated: 2026/07/20 15:43:38 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include <iostream>
#include <exception>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "ServerConfig.hpp"

class RequestHandler
{
private:
	const ServerConfig&	_config;

	HttpResponse		handleGet(const HttpRequest& request);
	bool 				hasExtension(const std::string& path,
		const std::string& extension) const;
public:
	RequestHandler(const ServerConfig& config);
	// RequestHandler(const RequestHandler& other);
	// RequestHandler& operator=(const RequestHandler& other);
	~RequestHandler();

	HttpResponse	handle(const HttpRequest& request);
};

#endif