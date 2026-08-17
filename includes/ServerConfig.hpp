/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/20 15:35:09 by tcali             #+#    #+#             */
/*   Updated: 2026/08/17 18:32:02 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <iostream>
#include <string>

#include "LocationConfig.hpp"

typedef std::vector<LocationConfig> locationVector;

class ServerConfig
{
private:
	std::string		_root;
	locationVector	_locations;

public:
	ServerConfig();
	// ServerConfig(const ServerConfig& other);
	// ServerConfig& operator=(const ServerConfig& other);
	~ServerConfig();

	std::string				getRoot()const;
	void					setRoot(const std::string& root);

	const locationVector&	getLocations()const;
	void					setLocations(const locationVector& locations);
	void					addLocations(const LocationConfig& location);

	const LocationConfig*	findLocation(const std::string& uri)const;
};

#endif