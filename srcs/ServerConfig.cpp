/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdossa <sdossa@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/20 15:40:51 by tcali             #+#    #+#             */
/*   Updated: 2026/09/02 14:04:03 by sdossa           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

ServerConfig::ServerConfig(): _root("www"), _port(8080), _clientMaxBodySize(1 * 1024 * 1024)
{
}

ServerConfig::~ServerConfig()
{
}

std::string ServerConfig::getRoot()const
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

const LocationConfig* ServerConfig::findLocation(const std::string& uri) const
{
	const LocationConfig* best = NULL;
	std::size_t bestLen = 0;

	for (std::vector<LocationConfig>::const_iterator it = _locations.begin();
		it != _locations.end(); ++it)
	{
		const std::string& path = it->getPath();

		// la location doit etre un prefixe de l'uri demandee
		if (uri.compare(0, path.size(), path) != 0)
			continue;

		bool boundaryOk = (uri.size() == path.size())
			|| (path[path.size() - 1] == '/')
			|| (uri[path.size()] == '/');
		
		if (!boundaryOk)
			continue;
		
		if (path.size() > bestLen)
		{
			bestLen = path.size();
			best = &(*it);
		}
	}
	return (best);
}
