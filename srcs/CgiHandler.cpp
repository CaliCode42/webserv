/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 16:46:54 by tcali             #+#    #+#             */
/*   Updated: 2026/08/27 17:02:27 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"
#include "Utils.hpp"
#include <cctype>

std::string	CgiHandler::getExtension(const std::string& uri)
{
	std::string	path = uri;

	std::string::size_type	queryPos = path.find('?');

	if (queryPos != std::string::npos)
		path.erase(queryPos);

	std::string::size_type	slashPos = path.find_last_of('/');
	std::string::size_type	dotPos = path.find_last_of('.');

	if (dotPos == std::string::npos)
		return ("");

	if (slashPos != std::string::npos && dotPos < slashPos)
		return ("");

	return (path.substr(dotPos));
}

std::string	CgiHandler::toCgiHeaderName(const std::string& headerName)
{
	std::string	name = "HTTP_";

	for (std::size_t i = 0; i < headerName.size(); ++i)
	{
		char	c = headerName[i];

		if (c == '-')
			name += '_';
		else
			name += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
	}

	return (name);
}

bool	CgiHandler::isCgiRequest(const std::string& uri, const LocationConfig& location)
{
	std::string	extension = getExtension(uri);

	if (extension.empty())
		return (false);

	return (location.isCgiExtension(extension));
}

std::string	CgiHandler::getInterpreter(const std::string& uri, const LocationConfig& location)
{
	std::string	extension = getExtension(uri);

	if (extension.empty())
		return ("");

	return (location.getCgiPath(extension));
}

std::string	CgiHandler::getUriPath(const std::string& uri)
{
	std::string::size_type	pos = uri.find('?');

	if (pos == std::string::npos)
		return (uri);

	return (uri.substr(0, pos));
}

std::string	CgiHandler::getQueryString(const std::string& uri)
{
	std::string::size_type	pos = uri.find('?');

	if (pos == std::string::npos)
		return ("");

	return (uri.substr(pos + 1));
}

std::string	CgiHandler::resolveScriptPath(const std::string& uri, const LocationConfig& location)
{
	std::string	path = getUriPath(uri);
	const std::string&	locationPath = location.getPath();
	const std::string&	root = location.getRoot();

	if (path.compare(0, locationPath.size(), locationPath) != 0)
		return ("");

	std::string relativePath = path.substr(locationPath.size());

	if (!relativePath.empty() && relativePath[0] != '/')
		relativePath = "/" + relativePath;

	if (!root.empty() && root[root.size() - 1] == '/' && !relativePath.empty() && relativePath[0] == '/')
		return (root + relativePath.substr(1));

	return (root + relativePath);
}

envMap	CgiHandler::buildEnvironment(const HttpRequest& request, const std::string& uri,
	const LocationConfig& location, const std::string& scriptPath)
{
	envMap env;

	(void)location;

	env["REQUEST_METHOD"] = request.getMethod();
	env["QUERY_STRING"] = getQueryString(uri);
	env["SCRIPT_FILENAME"] = scriptPath;
	env["SCRIPT_NAME"] = getUriPath(uri);
	env["SERVER_PROTOCOL"] = request.getVersion();

	if (!request.getBody().empty())
		env["CONTENT_LENGTH"] = turnIntoString(request.getBody().size());
	else
		env["CONTENT_LENGTH"] = "";

	env["CONTENT_TYPE"] = request.getHeader("content-type");

	const std::map<std::string, std::string>&	headers =
	request.getHeaders();

	for (std::map<std::string, std::string>::const_iterator it = headers.begin();
		it != headers.end(); ++it)
	{
		if (it->first == "content-length"
			|| it->first == "content-type")
		{
			continue ;
		}

		env[toCgiHeaderName(it->first)] = it->second;
	}

	return (env);
}
