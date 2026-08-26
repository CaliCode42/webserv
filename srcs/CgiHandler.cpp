/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 16:46:54 by tcali             #+#    #+#             */
/*   Updated: 2026/08/18 18:07:36 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"

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

    if (!root.empty() && root[root.size() - 1] == '/'
        && !relativePath.empty() && relativePath[0] == '/')
    {
        return (root + relativePath.substr(1));
    }

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

    env["CONTENT_LENGTH"] = request.getHeader("content-length");
    env["CONTENT_TYPE"] = request.getHeader("content-type");

    return (env);
}
