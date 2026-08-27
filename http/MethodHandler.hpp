/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MethodHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/25 19:51:12 by sdossa            #+#    #+#             */
/*   Updated: 2026/08/27 16:36:09 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef METHODHANDLER_HPP
#define METHODHANDLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "SessionManager.hpp"
#include "../includes/ServerConfig.hpp"
#include "../includes/LocationConfig.hpp"
#include <string>

class MethodHandler 
{
public:
	//entry point called by event loop when 1 req is complete
	MethodHandler(const ServerConfig& config);
	HttpResponse handle(const HttpRequest& req);
	
	~MethodHandler();
	
private:
	const ServerConfig& _config;
	SessionManager _sessions;
	
	HttpResponse handleGet(const std::string& path, const LocationConfig* location,
						const std::string& uriPath);
	HttpResponse handlePost(const HttpRequest& req, const std::string& uriPath, 
							const LocationConfig* location);
	HttpResponse handleDelete(const std::string& path);
	
	//maps an uri to a file path using the location root
	std::string resolvePath(const std::string& uriPath, const LocationConfig* location) const;
	std::string buildAutoindex(const std::string& path, const std::string& uriPath) const;
	
	static std::string decodeUrl(const std::string& uri);
	static std::string contentTypeFor(const std::string& path);
	
	HttpResponse makeError(int code);
	
};

#endif