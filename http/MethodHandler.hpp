/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MethodHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/25 19:51:12 by sdossa            #+#    #+#             */
/*   Updated: 2026/08/25 17:57:49 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef METHODHANDLER_HPP
#define METHODHANDLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "ServerConfig.hpp"
#include <string>

class MethodHandler 
{
public:
	// point d'entree called by event loop qd 1 req est complete
	MethodHandler(const ServerConfig& config);
	HttpResponse handle(const HttpRequest& req, const LocationConfig& location);
	
private:
	const ServerConfig& _config;
	HttpResponse handleGet(const std::string& path, const LocationConfig& location);
	HttpResponse handlePost(const HttpRequest& req);
	HttpResponse handleDelete(const std::string& path);
	
	std::string	getContentType(const std::string& path) const;

	std::string	resolvePath(const HttpRequest& req, const LocationConfig& location) const;
};

#endif