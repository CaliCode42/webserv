/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MethodHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/25 19:50:52 by sdossa            #+#    #+#             */
/*   Updated: 2026/09/25 12:35:09 by tcali            ###   ########.fr       */
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
#ifndef PATH_MAX
# define PATH_MAX 4096
#endif


MethodHandler::MethodHandler(const ServerConfig& config) : _config(config), _sessions(1800)
{}

MethodHandler::~MethodHandler()
{}

//RE
bool MethodHandler::isWithinRoot(const std::string& resolvedPath, const std::string& root) const
{
	char realResolved[PATH_MAX];
	char realRoot[PATH_MAX];

	if (realpath(resolvedPath.c_str(), realResolved) == NULL)
		return false;
	if (realpath(root.c_str(), realRoot) == NULL)
		return false;

	std::string resolvedStr(realResolved);
	std::string rootStr(realRoot);

	if (resolvedStr == rootStr)
		return true;
	if (resolvedStr.compare(0, rootStr.size(), rootStr) == 0
		&& resolvedStr.size() > rootStr.size()
		&& resolvedStr[rootStr.size()] == '/')
		return true;
	return false;
}

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
	res.setBody(buildStyledPage(code, HttpResponse::reasonPhrase(code), "Une erreur est survenue."), "text/html");
	return res;
}

std::string MethodHandler::buildStyledPage(int code, const std::string& title, const std::string& desc)
{
	std::ostringstream html;
	html << "<!DOCTYPE html>\n"
		<< "<html lang=\"fr\">\n"
		<< "<meta charset=\"UTF-8\">\n"
		<< "<title>" << code << " " << HttpResponse::reasonPhrase(code) << "</title>\n"
		<< "<body>\n"
		<< "  <div class=\"card\">\n"
		<< "    <p class=\"code\">" << code << "</p>\n"
		<< "    <p class=\"title\">" << title << "</p>\n"
		<< "    <p class=\"desc\">" << desc << "</p>\n"
		<< "    <p class=\"foot\">webserv/1.1</p>\n"
		<< "  </div>\n"
		<< "</body>\n"
		<< "</html>\n";
	return html.str();
}

HttpResponse MethodHandler::handle(const HttpRequest& req)
{
	if (req.hasError())
		return makeError(req.errorCode());
		
	//reuse cookies if valid or create a new one
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

	//count visits per session
	std::string visitCountStr = _sessions.get(sessionId, "visits");
	int visitCount = visitCountStr.empty() ? 0 : std::atoi(visitCountStr.c_str());
	visitCount++;
	std::ostringstream visitOss;
	visitOss << visitCount;
	_sessions.set(sessionId, "visits", visitOss.str());
	
	// to comment during siege if necessary
	const std::map<std::string, std::string>& headers = req.getHeaders();
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
		std::cout << it->first << ": " << it->second << std::endl;
	
	//decode before anti path-traversal test or "%2e%2e" pass through
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
	else if (location != NULL && !location->getAllowedMethods().empty()
			&& !location->isMethodAllowed(req.getMethod()))
	{
		response = makeError(405);
	}
	else if (location != NULL && location ->hasRedirect())
	{
		int redirectCode = location->getRedirectCode();
		response.setStatus(redirectCode);
		response.setHeader("Location", location->getRedirect());
		response.setBody(buildStyledPage(redirectCode, HttpResponse::reasonPhrase(redirectCode), 
		"Cette ressource a été déplacée vers une nouvelle adresse."), "text/html");
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
		response = handleDelete(resolvePath(uriPath, location), location);
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
		if (!root.empty() && root[root.size() - 1] == '/' && !uriPath.empty() && uriPath[0]== '/')
			return root + uriPath.substr(1);
		return root + uriPath;
	}

	std::string locPath = location->getPath();
	std::string relative = uriPath;
	
	//strip location prefix: /cgi/test.py -> /test.py
	if (uriPath.compare(0, locPath.size(), locPath) == 0)
		relative = uriPath.substr(locPath.size());
		
	if (relative.empty())
		relative = "/";
	else if (relative[0] != '/')
		relative = "/" + relative;

	//avoid double slash
	if (!root.empty() && root[root.size() - 1] == '/')
		return root + relative.substr(1);

	return root + relative;
}

std::string MethodHandler::htmlEscape(const std::string& str)
{
	std::string result;
	result.reserve(str.size());
	
	for (std::string::size_type i = 0; i < str.size(); ++i)
	{
		switch (str[i])
		{
		case '&': result += "&amp;";
			break;
		case '<': result += "&alt;";
			break;
		case '>': result += "&gt;";
			break;
		case '"': result += "&quot;";
			break;
		case '\'': result += "&#39;";
			break;
		default: result += str[i];
			break;
		}
	}
	return result;
}

std::string MethodHandler::buildAutoindex(const std::string& path, const std::string& uriPath) const
{
	DIR* dir = opendir(path.c_str());
	if (dir == NULL)
		return "";
	
	std::string base = uriPath;
	if (base.empty() || base[base.size() - 1] != '/')
		base += "/";
	
	std::ostringstream list;
	struct  dirent* entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;
		if (name == ".")
			continue;
		list << "<li><a href=\"" << htmlEscape(base) << htmlEscape(name) << "\">" << htmlEscape(name) << "</a></li>\n";
	}
	closedir(dir);

	std::ostringstream html;
	html << "<!DOCTYPE html>\n"
		<< "<html lang=\"fr\">\n"
		<< "<meta charset=\"UTF-8\">\n"
		<< "<title>Index of " << htmlEscape(uriPath) << "</title>\n"
		<< "<body>\n"
		<< "<div class=\"card\">\n"
		<< "<p class=\"code\">Index</p>\n"
		<< "<p class=\"title\">Index of " << htmlEscape(uriPath) << "</p>\n"
		<< "<ul class=\"listing\">\n"
		<< list.str()
		<< "</ul>\n"
		<< "<p class=\"foot\">webserv/1.1</p>\n"
		<< "</div>\n"
		<< "</body>\n"
		<< "</html>\n";
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

	std::string root = (location != NULL) ? location->getRoot() : _config.getRoot();
	if (!isWithinRoot(path, root))
		return makeError(403);
	
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
			std::ifstream file(indexPath.c_str(), std::ios::binary);
			if (!file.is_open())
				return (makeError(403));

			std::ostringstream ss;
			ss << file.rdbuf();
				
			HttpResponse res;
			res.setStatus(200);
			res.setBody(ss.str(), contentTypeFor(indexPath));
			return res;
		}
		if (location != NULL && location->getAutoindex())
		{
			std::string listing = buildAutoindex(dirPath, uriPath);
			if (listing.empty())
				return makeError(403);
				
			HttpResponse res;
			res.setStatus(200);
			res.setBody(listing, "text/html");
			return res;
		}
		return makeError(403);
	}	
	
	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file.is_open())
		return (makeError(403));

	std::ostringstream ss;
	ss << file.rdbuf();
		
	HttpResponse res;
	res.setStatus(200);
	res.setBody(ss.str(), contentTypeFor(path));
	return res;
}

HttpResponse MethodHandler::handlePost(const HttpRequest& req, const std::string& uriPath,
										const LocationConfig* location)
{
	std::cout << "POST uri: " << uriPath << std::endl;

	if (location == NULL)
	{
		std::cout << "POST location: NULL" << std::endl;
	}
	else
	{
		std::cout << "POST location path: " << location->getPath() << std::endl;
		std::cout << "POST location root: " << location->getRoot() << std::endl;
		std::cout << "POST upload enabled: "
				<< location->getUploadEnabled() << std::endl;
		std::cout << "POST upload explicit: "
				<< location->isUploadEnabledExplicit() << std::endl;
		std::cout << "POST upload path: "
				<< location->getUploadPath() << std::endl;
	}

	if (location != NULL && location->isUploadEnabledExplicit() && !location->getUploadEnabled())
			return makeError(403);
	
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
		//avoid silent ecrasing if 2 file names arrive at the same time
		static unsigned long uploadCounter = 0;
		std::ostringstream oss;
		oss << "upload_" << std::time(NULL) << "_" << uploadCounter++ << ".bin";
		filename = oss.str();
	}
	uploadDir += filename;
	
	struct stat existSt;
	if (stat(uploadDir.c_str(), &existSt) == 0)
		return makeError(409);
	
	std::ofstream file(uploadDir.c_str(), std::ios::binary);
	if (!file.is_open())
		return makeError(403);
	
	file << req.getBody();
	file.close();

	HttpResponse res;
	res.setStatus(201);
	res.setBody(buildStyledPage(201, "Ressource créée", "Le fichier a été enregistré avec succès."), "text/html");
	return res;
}

HttpResponse MethodHandler::handleDelete(const std::string& path, const LocationConfig* location)
{
	struct stat st;
	if (stat(path.c_str(), &st) != 0)
		return makeError(404);

	std::string root = (location != NULL) ? location->getRoot() : _config.getRoot();
	if (!isWithinRoot(path, root))
		return makeError(403);

	if (std::remove(path.c_str()) != 0)
		return makeError(403);

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
	for (std::string::size_type i = 0; i < ext.size(); ++i)
		ext[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(ext[i])));
	
	if (ext == ".html" || ext == ".htm")	return "text/html";
	if (ext == ".css")						return "text/css";
	if (ext == ".js")						return "application/javascript";
	if (ext == ".png")						return "image/png";
	if (ext == ".jpg" || ext == ".jpeg")	return "image/jpeg";
	if (ext == ".gif")						return "image/gif";
	if (ext == ".txt")						return "text/plain";
	if (ext == ".ico")						return "image/x-icon";
	if (ext == ".pdf")						return "application/pdf";
	if (ext == ".svg")						return "image/svg+xml";
	if (ext == ".mp4")						return "video/mp4";
	if (ext == ".mp3")						return "audio/mpeg";
	return "application/octet-stream";
}
