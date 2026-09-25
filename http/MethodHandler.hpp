/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MethodHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdossa <sdossa@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/25 19:51:12 by sdossa            #+#    #+#             */
/*   Updated: 2026/09/24 20:11:38 by sdossa           ###   ########.fr       */
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
#include <climits>

class MethodHandler 
{
public:
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
	HttpResponse handleDelete(const std::string& path, const LocationConfig* location);
	
	std::string resolvePath(const std::string& uriPath, const LocationConfig* location) const;
	std::string buildAutoindex(const std::string& path, const std::string& uriPath) const;
	
	static std::string decodeUrl(const std::string& uri);
	static std::string contentTypeFor(const std::string& path);
	static std::string htmlEscape(const std::string& str);
	static std::string buildStyledPage(int code, const std::string& title, const std::string& desc);
	//RE
	bool isWithinRoot(const std::string& resolvedPath, const std::string& root) const;
	
	HttpResponse makeError(int code);
};

#endif