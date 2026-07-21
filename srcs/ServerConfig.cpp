/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/20 15:40:51 by tcali             #+#    #+#             */
/*   Updated: 2026/07/20 15:49:22 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

ServerConfig::ServerConfig(): _root("www")
{
	std::cout << "[ServerConfig] Default constructor called" << std::endl;
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
	std::cout << "[ServerConfig] Destructor called" << std::endl;
}

std::string	ServerConfig::getRoot()const
{
	return (_root);
}
