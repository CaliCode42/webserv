/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdossa <sdossa@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/23 15:14:58 by sdossa            #+#    #+#             */
/*   Updated: 2026/08/27 04:27:28 by sdossa           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <cctype>
#include <stdexcept>
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"

class ConfigParser
{
private:
	std::vector<std::string> _tokens;
	size_t _pos;
	
	std::vector<std::string> tokenize(const std::string& content);
	std::string readFile(const std::string& path);
	
	std::string peek() const;
	std::string peekServerRoot() const;
	std::string next();
	void expect(const std::string& expected);

	std::size_t parseSize(const std::string& value);

	ServerConfig parseServer();
	LocationConfig parseLocation(const std::string& serverRoot);

public:
	ConfigParser();
	~ConfigParser();
	
	std::vector<ServerConfig> parse(const std::string& path);
};

#endif
