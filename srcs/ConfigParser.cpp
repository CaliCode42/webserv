/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/17 12:26:19 by tcali             #+#    #+#             */
/*   Updated: 2026/08/24 19:40:49 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>

ServerConfig	ConfigParser::parse(const std::string& filename)
{
	std::ifstream	file(filename.c_str());

	if (!file.is_open())
		throw std::runtime_error("Could not open configuration file");

	std::ostringstream	buffer;

	buffer << file.rdbuf();

	if (file.bad())
		throw std::runtime_error("Could not read configuration file");

	tokenize(buffer.str());

	_pos = 0;
	ServerConfig	config;

	while (hasMoreTokens())
	{
		if (currentToken() == "location")
			parseLocation(config);
		else
			throw std::runtime_error("Unexpected token :" + currentToken());
	}
	return (config);
}

const stringVector&	ConfigParser::getTokens() const
{
	return (_tokens);
}

const locationVector&	ConfigParser::getLocations()const
{
	return (_locations);
}

void	ConfigParser::tokenize(const std::string& content)
{
	_tokens.clear();

	std::string	current;

	for (std::size_t i = 0; i < content.size(); ++i)
	{
		char	c = content[i];

		if (std::isspace(static_cast<unsigned char>(c)))
		{
			if (!current.empty())
			{
				_tokens.push_back(current);
				current.clear();
			}
		}
		else if (c == '{' || c == '}' || c == ';')
		{
			if (!current.empty())
			{
				_tokens.push_back(current);
				current.clear();
			}

			_tokens.push_back(std::string(1, c));
		}
		else
		{
			current += c;
		}
	}

	if (!current.empty())
		_tokens.push_back(current);
}

bool	ConfigParser::hasMoreTokens() const
{
	return (_pos < _tokens.size());
}

const	std::string& ConfigParser::currentToken() const
{
	if (!hasMoreTokens())
		throw std::runtime_error("Unexpected end of configuration");

	return (_tokens[_pos]);
}

const std::string&	ConfigParser::nextToken()
{
	advance();
	
	return (currentToken());
}

void	ConfigParser::advance()
{
	if (hasMoreTokens())
		++_pos;
}

void	ConfigParser::expect(const std::string& expected)
{
	if (!hasMoreTokens() || currentToken() != expected)
		throw std::runtime_error("Expected '" + expected + "' in configuration");

	advance();
}

void	ConfigParser::parseLocation(ServerConfig& config)
{
	LocationConfig location;

	expect("location");

	if (!hasMoreTokens())
		throw std::runtime_error("Missing location path");

	location.setPath(currentToken());
	advance();

	expect("{");

	while (hasMoreTokens() && currentToken() != "}")
		parseLocationDirective(location);

	expect("}");

	config.addLocations(location);
}

void	ConfigParser::parseLocationDirective(LocationConfig& location)
{
	if (currentToken() == "root")
		parseRootDirective(location);
	else if (currentToken() == "cgi")
		parseCgiDirective(location);
	else
		throw std::runtime_error("Unknown location directive: " + currentToken());
}

void	ConfigParser::parseRootDirective(LocationConfig& location)
{
	expect("root");

	if (!hasMoreTokens())
		throw std::runtime_error("Missing root value");

	location.setRoot(currentToken());
	advance();

	expect(";");
}

void	ConfigParser::parseCgiDirective(LocationConfig& location)
{
	std::string extension;
	std::string executable;

	expect("cgi");

	if (!hasMoreTokens())
		throw std::runtime_error("Missing CGI extension");

	extension = currentToken();
	advance();

	if (!hasMoreTokens())
		throw std::runtime_error("Missing CGI executable");

	executable = currentToken();
	advance();

	expect(";");

	location.addCgiHandler(extension, executable);
}