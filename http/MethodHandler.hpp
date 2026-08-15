/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MethodHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdossa <sdossa@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/25 19:51:12 by sdossa            #+#    #+#             */
/*   Updated: 2026/08/15 12:15:17 by sdossa           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef METHODHANDLER_HPP
#define METHODHANDLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "../includes/ServerConfig.hpp"
#include <string>

class MethodHandler 
{
public:
	//entry point called by event loop when 1 req is complete
	MethodHandler(const ServerConfig& config);
	HttpResponse handle(const HttpRequest& req);
	
private:
	const ServerConfig& _config;
	HttpResponse handleGet(const std::string& path);
	HttpResponse handlePost(const HttpRequest& req);
	HttpResponse handleDelete(const std::string& path);
	
	static std::string contentTypeFor(const std::string& path);
	static HttpResponse makeError(int code);
	
};

#endif
