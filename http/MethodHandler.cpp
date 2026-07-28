/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MethodHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdossa <sdossa@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/25 19:50:52 by sdossa            #+#    #+#             */
/*   Updated: 2026/07/28 19:11:44 by sdossa           ###   ########.fr       */
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
        res.setBody("<h1>NOT FOUND ?</h1>", "text/html");
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