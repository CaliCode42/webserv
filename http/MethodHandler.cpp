/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MethodHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/25 19:50:52 by sdossa            #+#    #+#             */
/*   Updated: 2026/09/02 15:44:01 by tcali            ###   ########.fr       */
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
#include <dirent.h>
#include <fstream>
#include <cstdio>
#include <cerrno>

MethodHandler::MethodHandler(const ServerConfig& config) : _config(config), _sessions(1800)
{}

MethodHandler::~MethodHandler()
{}

HttpResponse MethodHandler::makeError(int code)
{
	HttpResponse res;
	res.setStatus(code);

	const std::map<int, std::string>& errorPages = _config.getErrorPages();
	std::map<int, std::string>::const_iterator errIt = errorPages.find(code);

	if (errIt != errorPages.end())
	{
		std::string errorPath = _config.getRoot() + errIt->second;
		struct stat st;
		if (stat(errorPath.c_str(), &st) == 0 && !S_ISDIR(st.st_mode))
		{
			std::ifstream file(errorPath.c_str());
			if (file.is_open())
			{
				std::ostringstream ss;
				ss << file.rdbuf();
				res.setBody(ss.str(), "text/html");
				return res;
			}
		}
	}
	res.setBody("<h1>" + HttpResponse::reasonPhrase(code) + "</h1>", "text/html");
	return res;
}

HttpResponse MethodHandler::handle(const HttpRequest& req)
{
	if (req.hasError())
		return makeError(req.errorCode());
		
	//reuse cookies if it's valid or create a new one
	std::map<std::string, std::string> cookies = req.getCookies();
	std::map<std::string, std::string>::const_iterator cookieIt = cookies.find("session_id");
	std::string sessionId;
	bool isNewSession = false;

	if (cookieIt != cookies.end() && _sessions.exists(cookieIt->second))
		sessionId = cookieIt->second;
	else
	{
		sessionId = _sessions.create();
		isNewSession =  true;
	}

	//set/get : count visits per session
	std::string visitCountStr = _sessions.get(sessionId, "visits");
	int visitCount = visitCountStr.empty() ? 0 : std::atoi(visitCountStr.c_str());
	visitCount++;
	std::ostringstream visitOss;
	visitOss << visitCount;
	_sessions.set(sessionId, "visits", visitOss.str());
	
	
	//decode before the 1st test anti path-traversal or "%2e%2e" pass through
	std::string decodedUri = decodeUrl(req.getUri());

	//query string not part of the file path
	std::string uriPath = decodedUri;
	std::string::size_type qpos = uriPath.find('?');
	if (qpos != std::string::npos)
		uriPath = uriPath.substr(0, qpos);
	
	const LocationConfig* location = _config.findLocation(uriPath);


	HttpResponse response;

	if (decodedUri.find("..") != std::string::npos)
	{
		response = makeError(403);
	}
	else if (location != NULL && location ->hasRedirect())
	{
		response.setStatus(301);
		response.setHeader("Location", location->getRedirect());
		response.setBody("<h1>301 - Moved Permanently</h1>", "text/html");
	}
	else if (req.getMethod() == "GET")
	{
		response = handleGet(resolvePath(uriPath, location), location, uriPath);
	}
	else if (req.getMethod() == "POST")
	{
		response = handlePost(req, uriPath, location);
	}
	else if (req.getMethod() == "DELETE")
	{
		response = handleDelete(resolvePath(uriPath, location));
	}
	else
	{
		response = makeError(501);
	}
	
	if (isNewSession)
	{
		response.setHeader("Set-Cookie", "session_id=" + sessionId + "; Path=/; HttpOnly");
	}

	return response;
}

std::string MethodHandler::resolvePath(const std::string& uriPath, const LocationConfig* location) const
{
	if (location == NULL)
		return _config.getRoot() + uriPath;

	std::string root = location->getRoot();
	
	if (!location->hasExplicitRoot())
	{
		//root inherit server : keep uri complete
		if (!root.empty() && root[root.size() - 1] == '/' && !uriPath.empty() && uriPath[0]== '/')
			return root + uriPath.substr(1);
		return root + uriPath;
	}

	//explicit root on this location : withdraw prefix
	std::string locPath = location->getPath();
	std::string relative = uriPath;
	
	//strip location prefix: /cgi/test.py under location /cgi -> /test.py
	if (uriPath.compare(0, locPath.size(), locPath) == 0)
		relative = uriPath.substr(locPath.size());
		
	if (relative.empty())
		relative = "/";
	else if (relative[0] != '/')
		relative = "/" + relative;

	//avoid dbl slash when roo already ends with one
	if (!root.empty() && root[root.size() - 1] == '/')
		return root + relative.substr(1);

	return root + relative;

}


std::string MethodHandler::buildAutoindex(const std::string& path, const std::string& uriPath) const
{
	DIR* dir = opendir(path.c_str());
	if (dir == NULL)
		return "";
	
	std::ostringstream html;
	html << "<html><head><title>Index of " << uriPath << "</title></head><body>";
	html << "<h1>Index of " << uriPath << "</h1><hr><ul>";

	std::string base = uriPath;
	if (base.empty() || base[base.size() - 1] != '/')
		base += "/";
	
	struct  dirent* entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;
		if (name == ".")
			continue;
		html << "<li><a href=\"" << base << name << "\">" << name << "</a></li>";
	}
	
	closedir(dir);
	html << "</ul><hr></body></html>";
	return html.str();
}


std::string MethodHandler::decodeUrl(const std::string& uri)
{
	std::string result;
	result.reserve(uri.size());

	for (std::string::size_type i = 0; i < uri.size(); ++i)
	{
		if (uri[i] == '%' && i + 2 < uri.size()
			&& std::isxdigit(static_cast<unsigned char>(uri[i + 1]))
			&& std::isxdigit(static_cast<unsigned char>(uri[i + 2])))
		{
			std::string hex = uri.substr(i + 1, 2);
			int value = std::strtol(hex.c_str(), NULL, 16);
			result += static_cast<char>(value);
			i += 2;
		}
		else
			result += uri[i];
	}
	return result;
}

HttpResponse MethodHandler::handleGet(const std::string& path, const LocationConfig* location,
									const std::string& uriPath)
{

	struct stat st;
	if (stat(path.c_str(), &st) != 0)
		return makeError(404);
	
	if (S_ISDIR(st.st_mode))
	{
		std::string dirPath = path;
		if (dirPath[dirPath.size() - 1] != '/')
			dirPath += "/";
		
		//default file to serve comes from location
		std::string indexName = (location != NULL) ? location->getIndex() : "index.html";
		std::string indexPath = dirPath + indexName;

		struct stat indexSt;
		if (!indexName.empty() && stat(indexPath.c_str(), &indexSt) == 0 && !S_ISDIR(indexSt.st_mode))
		{
			std::ifstream file(indexPath.c_str());
			std::ostringstream ss;
			ss << file.rdbuf();
				
			HttpResponse res;
			res.setStatus(200);// OK
			res.setBody(ss.str(), contentTypeFor(indexPath));
			return res;
		}
		if (location != NULL && location->getAutoindex())
		{
			std::string listing = buildAutoindex(dirPath, uriPath);
			if (listing.empty())
				return makeError(403);
				
			HttpResponse res;
			res.setStatus(200);// OK
			res.setBody(listing, "text/html");
			return res;
		}
		return makeError(403);
	}	
	
	std::ifstream file(path.c_str());
	std::ostringstream ss;
	ss << file.rdbuf();
		
	HttpResponse res;
	res.setStatus(200);// OK
	res.setBody(ss.str(), contentTypeFor(path));
	return res;
	
}

HttpResponse MethodHandler::handlePost(const HttpRequest& req, const std::string& uriPath,
										const LocationConfig* location)
{
	//path to save the file
	std::string uploadDir;
	if (location != NULL && location->getUploadEnabled() 
		&& !location->getUploadPath().empty())
		uploadDir = location->getUploadPath();
	else
		uploadDir = _config.getRoot() + "/uploads";
	
	if (uploadDir[uploadDir.size() - 1] != '/')
		uploadDir += "/";
		
	struct stat st;
	if (stat(uploadDir.c_str(), &st) != 0)
	{
		if (mkdir(uploadDir.c_str(), 0755) != 0)
			return makeError(500);
	}	

	
	std::string::size_type slashPos = uriPath.find_last_of('/');
	std::string filename;
	if (slashPos != std::string::npos)
		filename = uriPath.substr(slashPos + 1);
	
	if (filename.empty())
	{
		//avoid silent ecrasing if 2 Posts w/t file names arrive at the same time
		static unsigned long uploadCounter = 0;
		std::ostringstream oss;
		oss << "upload_" << std::time(NULL) << "_" << uploadCounter++ << ".bin";
		filename = oss.str();
	}
	uploadDir += filename;

	//open the file
	std::ofstream file(uploadDir.c_str(), std::ios::binary);
	if (!file.is_open())
		return makeError(500);
	
	//write body into the file
	file << req.getBody();
	file.close();

	//send 201 Created
	HttpResponse res;
	res.setStatus(201);
	res.setBody("<h1>201 - Created</h1>", "text/html");
	return res;
	
}

HttpResponse MethodHandler::handleDelete(const std::string& path)
{
	//check existing file
	struct stat st;
	if (stat(path.c_str(), &st) != 0)
		return makeError(404);

	//delete file
	if (std::remove(path.c_str()) != 0)
		return makeError(500);

	//SUCCESS: 204 No Content
	HttpResponse res;
	res.setStatus(204);
	return res;
	
}

std::string MethodHandler::contentTypeFor(const std::string& path)
{
	std::string::size_type dotPos = path.find_last_of('.');
	if (dotPos == std::string::npos)
		return "application/octet-stream";
	std::string ext = path.substr(dotPos);
	
	if (ext == ".html" || ext == ".htm")	return "text/html";
	if (ext == ".css")						return "text/css";
	if (ext == ".js")						return "application/javascript";
	if (ext == ".png")						return "image/png";
	if (ext == ".jpg" || ext == ".jpeg")	return "image/jpeg";
	if (ext == ".gif")						return "image/gif";
	if (ext == ".txt")						return "text/plain";
	if (ext == ".ico")						return "image/x-icon";
	if (ext == ".pdf")						return "application/pdf";
	return "application/octet-stream";

}


