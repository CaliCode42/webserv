/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 16:39:17 by tcali             #+#    #+#             */
/*   Updated: 2026/08/18 18:30:21 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <string>
#include <map>

#include "HttpRequest.hpp"
#include "LocationConfig.hpp"

typedef std::map<std::string, std::string> envMap;

class CgiHandler
{
private:
	static std::string	getExtension(const std::string& uri);

public:
	CgiHandler() {};
	~CgiHandler() {};

	static bool			isCgiRequest(const std::string& uri, const LocationConfig& location);

	static std::string	getInterpreter(const std::string& uri, const LocationConfig& location);
	static std::string	getUriPath(const std::string& uri);
	static std::string	getQueryString(const std::string& uri);

	static std::string	resolveScriptPath(const std::string& uri, const LocationConfig& location);
	static envMap		buildEnvironment(const HttpRequest& request, const std::string& uri,
		const LocationConfig& location, const std::string& scriptPath);
};

#endif