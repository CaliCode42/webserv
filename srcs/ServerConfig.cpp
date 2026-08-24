/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdossa <sdossa@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/20 15:40:51 by tcali             #+#    #+#             */
/*   Updated: 2026/08/16 15:10:18 by sdossa           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

ServerConfig::ServerConfig(): _root("www"), _port(8080), _clientMaxBodySize(1 * 1024 * 1024)
{
	std::cout << "[ServerConfig] Default constructor called" << std::endl;
}

ServerConfig::~ServerConfig()
{
	std::cout << "[ServerConfig] Destructor called" << std::endl;
}

std::string	ServerConfig::getRoot()const
{
	return (_root);
}

void ServerConfig::setRoot(const std::string& root)
{
	_root = root;
}

unsigned int ServerConfig::getPort() const 
{
	return (_port);
}

void ServerConfig::setPort(unsigned int port)
{
	_port = port;
}

void ServerConfig::setClientMaxBodySize(std::size_t size)
{
	_clientMaxBodySize = size;
}

const std::map<int, std::string>& ServerConfig::getErrorPages() const
{
	return (_errorPages);
}

void ServerConfig::setErrorPage(int code, const std::string& path)
{
	_errorPages[code] = path;
}

std::size_t ServerConfig::getClientMaxBodySize() const
{
	return (_clientMaxBodySize);
}

const std::vector<LocationConfig>& ServerConfig::getLocations() const
{
	return (_locations);
}

void ServerConfig::addLocation(const LocationConfig& loc)
{
	_locations.push_back(loc);
}


// ServerConfig::ServerConfig(const ServerConfig& other)
// {
// 	std::cout << "[ServerConfig] Copy constructor called" << std::endl;
// 	*this = other;
// }

// ServerConfig& ServerConfig::operator=(const ServerConfig& other)
// {
// 	if (this != &other)
// 	{
// 		// copy attributes here
// 	}
// 	std::cout << "[ServerConfig] Copy assignment operator called" << std::endl;
// 	return (*this);
// }

