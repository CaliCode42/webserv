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