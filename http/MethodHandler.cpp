/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MethodHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdossa <sdossa@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/25 19:50:52 by sdossa            #+#    #+#             */
/*   Updated: 2026/08/10 18:18:04 by sdossa           ###   ########.fr       */
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

HttpResponse MethodHandler::handle(const HttpRequest& req)
{
	if (req.getMethod() == "GET")
	{
		std::string path = _config.getRoot() + req.getUri();
		return handleGet(path);
	}
	if (req.getMethod() == "POST")
	{
		return handlePost(req);
	}
	if (req.getMethod() == "DELETE")
	{
		std::string path = _config.getRoot() + req.getUri();
		return handleDelete(path);
	}
	
	HttpResponse res;
	res.setStatus(501);
	res.setBody("<h1>A implémenter ok ^^ ?</h1>", "text/html");
	return res;
}

HttpResponse MethodHandler::handleGet(const std::string& path)
{

	struct stat st;
	if (stat(path.c_str(), &st) != 0)
	{
		HttpResponse res;
		res.setStatus(404);
		res.setBody("<h1>NOT FOUND</h1>", "text/html");
		return res;
	}

	std::ifstream file(path.c_str());
	std::ostringstream ss;
	ss << file.rdbuf();
		
	HttpResponse res;
	res.setStatus(200);
	res.setBody(ss.str(), "text/html");
	return res;
	
}

HttpResponse MethodHandler::handlePost(const HttpRequest& req)
{
	//path to save the file
	std::string uploadDir = _config.getRoot() + "/uploads/";
	std::string filename = uploadDir + "uploads.txt";

	//open the file
	std::ofstream file(filename.c_str(), std::ios::binary);
	if (!file.is_open())
	{
		HttpResponse res;
		res.setStatus(500);
		res.setBody("<h1>500 - Cannot open file</h1>", "text/html");
		return res;
	}
	
	//write body into the file
	file << req.getBody();
	file.close();

	//send 201 Created
	HttpResponse res;
	res.setStatus(201);
	res.setBody("<h1>201 - File uploaded</h1>", "text/html");
	return res;
	
}

HttpResponse MethodHandler::handleDelete(const std::string& path)
{
	//check existing file
	struct stat st;
	if (stat(path.c_str(), &st) != 0)
	{
		HttpResponse res;
		res.setStatus(404);
		res.setBody("<h1>404 - File not found</h1>", "text/html");
		return res;
	}

	//delete file
	if (std::remove(path.c_str()) != 0)
	{
		HttpResponse res;
		res.setStatus(500);
		res.setBody("<h1>500 - Cannot delete file</h1>", "text/html");
		return res;
	}

	//SUCCESS: 204 No Content
	HttpResponse res;
	res.setStatus(204);
	return res;
	
}

