/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MethodHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdossa <sdossa@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/25 19:50:52 by sdossa            #+#    #+#             */
/*   Updated: 2026/08/24 09:31:36 by sdossa           ###   ########.fr       */
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
#include <cerrno>

MethodHandler::MethodHandler(const ServerConfig& config) : _config(config)
{}

MethodHandler::~MethodHandler()
{}

HttpResponse MethodHandler::makeError(int code)
{
	HttpResponse res;
	res.setStatus(code);
	res.setBody("<h1>" + HttpResponse::reasonPhrase(code) + "</h1>", "text/html");
	return res;
}

HttpResponse MethodHandler::handle(const HttpRequest& req)
{
	if (req.hasError())
		return makeError(req.errorCode());

	//decode before the 1st test anti path-traversal or "%2e%2e" pass through
	std::string decodedUri = decodeUrl(req.getUri());

	if (decodedUri.find("..") != std::string::npos)
		return makeError(403);

	if (req.getMethod() == "GET")
	{
		std::string uri = req.getUri();
		std::string::size_type qpos = uri.find('?');
		if (qpos != std::string::npos)
			uri = uri.substr(0, qpos);
		if (uri.empty() || uri[uri.size() - 1] == '/')
			uri += "index.html";
		std::string path = _config.getRoot() + uri;
		return handleGet(path);
	}
	if (req.getMethod() == "POST")
	{
		return handlePost(req, decodedUri);
	}
	if (req.getMethod() == "DELETE")
	{
		std::string uri = req.getUri();
		std::string::size_type qpos = uri.find('?');
		if (qpos != std::string::npos)
			uri = uri.substr(0, qpos);
		std::string path = _config.getRoot() + uri;
		return handleDelete(path);
	}
	return makeError(501);
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

HttpResponse MethodHandler::handleGet(const std::string& path)
{

	struct stat st;
	if (stat(path.c_str(), &st) != 0)
		return makeError(404);
	
	if (S_ISDIR(st.st_mode))
		return makeError(403);

	std::ifstream file(path.c_str());
	std::ostringstream ss;
	ss << file.rdbuf();
		
	HttpResponse res;
	res.setStatus(200);// OK
	res.setBody(ss.str(), contentTypeFor(path));
	return res;
	
}

HttpResponse MethodHandler::handlePost(const HttpRequest& req, const std::string& decodedUri)
{
	//path to save the file
	std::string uploadDir = _config.getRoot() + "/uploads/";

	struct stat st;
	if (stat(uploadDir.c_str(), &st) != 0)
	{
		if (mkdir(uploadDir.c_str(), 0755) != 0)
			return makeError(500);
	}	

	
	std::string::size_type slashPos = decodedUri.find_last_of('/');
	std::string filename;
	if (slashPos != std::string::npos)
		filename = decodedUri.substr(slashPos + 1);
	
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


