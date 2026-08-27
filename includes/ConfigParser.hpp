/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/23 15:14:58 by sdossa            #+#    #+#             */
/*   Updated: 2026/08/27 16:59:02 by tcali            ###   ########.fr       */
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
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/17 12:22:29 by tcali             #+#    #+#             */
/*   Updated: 2026/08/24 19:29:28 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <string>
#include <vector>
#include "LocationConfig.hpp"
#include "ServerConfig.hpp"

typedef std::vector<std::string>	stringVector;
typedef std::vector<LocationConfig>	locationVector;

class ConfigParser
{
private:
	stringVector		_tokens;
	std::size_t			_pos;
	locationVector		_locations;

	void				tokenize(const std::string& content);

	bool				hasMoreTokens()const;
	const std::string&	currentToken()const;
	const std::string&	nextToken();
	void				advance();

	void				expect(const std::string& expected);

	void				parseLocation(ServerConfig& config);
	void				parseLocationDirective(LocationConfig& location);
	void				parseRootDirective(LocationConfig& location);
	void				parseCgiDirective(LocationConfig& location);

public:
	ConfigParser() : _pos(0) {};
	~ConfigParser() {};

	ServerConfig			parse(const std::string& filename);

	const stringVector&		getTokens()const;
	const locationVector&	getLocations()const;
};

#endif