/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MethodHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/25 19:50:52 by sdossa            #+#    #+#             */
/*   Updated: 2026/08/25 18:04:49 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "MethodHandler.hpp"
#include "../includes/ServerConfig.hpp"
#include <cctype>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <ctime>
#include <sys/stat.h>
#include <fstream>
#include <cstdio>

MethodHandler::MethodHandler(const ServerConfig& config) : _config(config) {}

HttpResponse MethodHandler::handle(const HttpRequest& req, const LocationConfig& location)
{
	if (req.getMethod() == "GET")
	{
		std::string path = resolvePath(req, location);

		return handleGet(path, location);
	}

	if (req.getMethod() == "POST")
		return handlePost(req);

	if (req.getMethod() == "DELETE")
	{
		std::string path = _config.getRoot() + req.getUri();
		
		return handleDelete(path);
	}
	
	HttpResponse res;

	res.setStatus(501);
	res.setBody("<h1>Not Implemented</h1>", "text/html");

	return res;
}

HttpResponse MethodHandler::handleGet(const std::string& path, const LocationConfig& location)
{
	std::string	resourcePath = path;
	struct stat st;

	if (stat(resourcePath.c_str(), &st) != 0)
	{
		HttpResponse res;

		res.setStatus(404);
		res.setBody("<h1>NOT FOUND</h1>", "text/html");
		
		return res;
	}

	if (S_ISDIR(st.st_mode))
	{
		if (location.getIndex().empty())
		{
			HttpResponse res;
	
			res.setStatus(404);
			res.setBody("<h1>Not Found</h1>", "text/html");
		
			return res;
		}
		
		if (!resourcePath.empty() && resourcePath[resourcePath.size() - 1] != '/')
			resourcePath += '/';

		resourcePath += location.getIndex();

		if (stat(resourcePath.c_str(), & st) != 0 || S_ISDIR(st.st_mode))
		{
			HttpResponse res;
	
			res.setStatus(404);
			res.setBody("<h1>Not Found</h1>", "text/html");
		
			return res;
		}
	}

	std::ifstream file(resourcePath.c_str());

	if (!file.is_open())
	{
		HttpResponse res;

		res.setStatus(500);
		res.setBody("<h1>Internal Server Error</h1>", "text/html");
		
		return res;
	}

	std::ostringstream ss;

	ss << file.rdbuf();
		
	HttpResponse res;

	res.setStatus(200);
	res.setBody(ss.str(), getContentType(path));
	
	return res;
}

std::string MethodHandler::getContentType(const std::string& path) const
{
	std::string::size_type dot = path.rfind('.');

	if (dot == std::string::npos)
		return "application/octet-stream";

	std::string extension = path.substr(dot);

	if (extension == ".html" || extension == ".htm")
		return "text/html";
	if (extension == ".css")
		return "text/css";
	if (extension == ".js")
		return "application/javascript";
	if (extension == ".txt")
		return "text/plain";
	if (extension == ".jpg" || extension == ".jpeg")
		return "image/jpeg";
	if (extension == ".png")
		return "image/png";
	if (extension == ".gif")
		return "image/gif";

	return "application/octet-stream";
}

std::string	MethodHandler::resolvePath(const HttpRequest& req,
	const LocationConfig& location) const
{
	std::string	uri = req.getUri();
	std::string::size_type	queryPos = uri.find('?');

	if (queryPos != std::string::npos)
		uri.erase(queryPos);

	const std::string&	locationPath = location.getPath();

	if (locationPath != "/"
		&& uri.compare(0, locationPath.size(), locationPath) == 0)
	{
		uri.erase(0, locationPath.size());
	}

	if (!uri.empty() && uri[0] == '/')
		uri.erase(0, 1);

	std::string	root = location.getRoot();

	if (!root.empty() && root[root.size() - 1] != '/')
		root += '/';

	return (root + uri);
}

HttpResponse MethodHandler::handlePost(const HttpRequest& req)
{
	std::string	uploadDir = _config.getRoot() + "/uploads";
	
	struct stat	st;

	if (stat(uploadDir.c_str(), &st) != 0)
	{
		if (mkdir(uploadDir.c_str(), 0755) != 0)
		{
			HttpResponse res;
			res.setStatus(500);
			res.setBody("<h1>500 - Cannot create upload directory</h1>", "text/html");
			return res;
		}
	}
	else if (!S_ISDIR(st.st_mode))
	{
		HttpResponse res;
		res.setStatus(500);
		res.setBody("<h1>500 - Upload path is not a directory</h1>", "text/html");
		return res;
	}
	std::string uri = req.getUri();

	std::string::size_type slash = uri.rfind('/');
	std::string filename;

	if (slash == std::string::npos)
		filename = uri;
	else
		filename = uri.substr(slash + 1);

	if (filename.empty())
	{
		HttpResponse res;
		res.setStatus(400);
		res.setBody("<h1>400 - Invalid filename</h1>", "text/html");
		return res;
	}

	std::string path = uploadDir + "/" + filename;

	std::ofstream file(path.c_str(),
		std::ios::out | std::ios::binary | std::ios::trunc);

	if (!file.is_open())
	{
		HttpResponse res;
		res.setStatus(500);
		res.setBody("<h1>500 - Cannot open file</h1>", "text/html");
		return res;
	}

	file.write(req.getBody().data(), req.getBody().size());

	if (!file.good())
	{
		file.close();

		HttpResponse res;
		res.setStatus(500);
		res.setBody("<h1>500 - Cannot write file</h1>", "text/html");
		return res;
	}

	file.close();

	HttpResponse res;
	res.setStatus(201);
	res.setBody("<h1>201 - File uploaded</h1>", "text/html");
	return res;
}

HttpResponse MethodHandler::handleDelete(const std::string& path)
{
	struct stat st;

	if (stat(path.c_str(), &st) != 0)
	{
		HttpResponse res;
		res.setStatus(404);
		res.setBody("<h1>404 - File not found</h1>", "text/html");
		return res;
	}

	if (S_ISDIR(st.st_mode))
	{
		HttpResponse res;
		res.setStatus(404);
		res.setBody("<h1>404 - File not found</h1>", "text/html");
		return res;
	}

	if (std::remove(path.c_str()) != 0)
	{
		HttpResponse res;
		res.setStatus(500);
		res.setBody("<h1>500 - Cannot delete file</h1>", "text/html");
		return res;
	}

	HttpResponse res;
	res.setStatus(204);
	return res;
}