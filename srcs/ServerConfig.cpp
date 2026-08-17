/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/20 15:40:51 by tcali             #+#    #+#             */
/*   Updated: 2026/08/17 18:33:04 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

ServerConfig::ServerConfig(): _root("www")
{
	// std::cout << "[ServerConfig] Default constructor called" << std::endl;
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

ServerConfig::~ServerConfig()
{
	// std::cout << "[ServerConfig] Destructor called" << std::endl;
}

std::string	ServerConfig::getRoot()const
{
	return (_root);
}

void	ServerConfig::setRoot(const std::string& root)
{
	_root = root;
}

const locationVector&	ServerConfig::getLocations()const
{
	return (_locations);
}

void	ServerConfig::setLocations(const locationVector& locations)
{
	_locations = locations;
}

void	ServerConfig::addLocations(const LocationConfig& location)
{
	_locations.push_back(location);
}

const LocationConfig*	ServerConfig::findLocation(const std::string& uri)const
{
	const LocationConfig*	bestMatch = NULL;
	std::size_t	bestLength = 0;

    for (locationVector::const_iterator it = _locations.begin(); it != _locations.end(); ++it)
    {
        const std::string& path = it->getPath();

        if (path.empty())
            continue;

        if (uri.compare(0, path.size(), path) != 0)
            continue;

        bool validMatch = false;

        if (uri.size() == path.size())
            validMatch = true;
        else if (path == "/")
            validMatch = true;
        else if (uri.size() > path.size()
                 && uri[path.size()] == '/')
            validMatch = true;

        if (validMatch && path.size() > bestLength)
        {
            bestMatch = &(*it);
            bestLength = path.size();
        }
    }

    return (bestMatch);
}
