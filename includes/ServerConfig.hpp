/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/20 15:35:09 by tcali             #+#    #+#             */
/*   Updated: 2026/08/27 17:01:55 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "LocationConfig.hpp"

typedef std::vector<LocationConfig> locationVector;

class ServerConfig
{
private:
	std::string _root;
	unsigned int _port;
	std::map<int, std::string> _errorPages;
	std::size_t _clientMaxBodySize;
	std::vector<LocationConfig> _locations;

public:
	ServerConfig();
	~ServerConfig();

	std::string getRoot() const;
	void setRoot(const std::string& root);

	unsigned int getPort() const;
	void setPort(unsigned int port);

	const std::map<int, std::string>& getErrorPages() const;
	void setErrorPage(int code, const std::string& path);

	std::size_t getClientMaxBodySize() const;
	void setClientMaxBodySize(std::size_t size);

	const std::vector<LocationConfig>& getLocations() const;
	void addLocation(const LocationConfig& loc);

	const LocationConfig* findLocation(const std::string& uri) const;
};

#endif
