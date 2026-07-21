/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/15 17:16:14 by tcali             #+#    #+#             */
/*   Updated: 2026/07/20 15:33:13 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler.hpp"
#include <fstream>
#include <sstream>

RequestHandler::RequestHandler(const ServerConfig& config): _config(config)
{
	std::cout << "[RequestHandler] Default constructor called" << std::endl;
}

// RequestHandler::RequestHandler(const RequestHandler& other)
// {
// 	std::cout << "[RequestHandler] Copy constructor called" << std::endl;
// 	*this = other;
// }

// RequestHandler& RequestHandler::operator=(const RequestHandler& other)
// {
// 	if (this != &other)
// 	{
// 		// copy attributes here
// 	}
// 	std::cout << "[RequestHandler] Copy assignment operator called" << std::endl;
// 	return (*this);
// }

RequestHandler::~RequestHandler()
{
	std::cout << "[RequestHandler] Destructor called" << std::endl;
}

HttpResponse	RequestHandler::handle(const HttpRequest& request)
{
	if (request.getMethod() == "GET")
		return (handleGet(request));

	HttpResponse response;
	response.setStatus(405, "Method Not Allowed");
	response.setBody("Method Not Allowed");
	return (response);
}

HttpResponse RequestHandler::handleGet(const HttpRequest& request)
{
    HttpResponse response;

    // get the path from URL
    std::string path = request.getPath();

    // "/" = home page
    if (path == "/")
        path = "/index.html";

    // Avoid dangerous path (easy version, not checking every cases)
    if (path.find("..") != std::string::npos)
    {
        response.setStatus(403, "Forbidden");
        response.setHeader("Content-Type", "text/html");
        response.setBody("<h1>403 Forbidden</h1>");
        return (response);
    }

    // Turn the path into a local path
    std::string localPath = _config.getRoot() + path;

    // Open the file
    std::ifstream file(localPath.c_str(), std::ios::in | std::ios::binary);

    // If open fails, return response 404
    if (!file.is_open())
    {
        response.setStatus(404, "Not Found");
        response.setHeader("Content-Type", "text/html");
        response.setBody("<h1>404 Not Found</h1>");
        return (response);
    }

    // Read the content of the file
    std::ostringstream content;
    content << file.rdbuf();

    file.close();

    // Find MIME type from the extension
    std::string contentType = "application/octet-stream";

    if (hasExtension(path, ".html") || hasExtension(path, ".htm"))
        contentType = "text/html";
    else if (hasExtension(path, ".css"))
        contentType = "text/css";
    else if (hasExtension(path, ".js"))
        contentType = "application/javascript";
    else if (hasExtension(path, ".txt"))
        contentType = "text/plain";
    else if (hasExtension(path, ".png"))
        contentType = "image/png";
    else if (hasExtension(path, ".jpg")
        || hasExtension(path, ".jpeg"))
        contentType = "image/jpeg";
    else if (hasExtension(path, ".gif"))
        contentType = "image/gif";

    // Build success response
    response.setStatus(200, "OK");
    response.setHeader("Content-Type", contentType);
    response.setBody(content.str());

    // Return response object to Server
    return (response);
}

bool RequestHandler::hasExtension(const std::string& path,
    const std::string& extension) const
{
    if (path.size() < extension.size())
        return (false);

    return (path.compare(path.size() - extension.size(),
        extension.size(), extension) == 0);
}

