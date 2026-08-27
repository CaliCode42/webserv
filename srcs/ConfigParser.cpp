/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/23 15:14:34 by sdossa            #+#    #+#             */
/*   Updated: 2026/08/27 22:27:37 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

ConfigParser::ConfigParser() : _pos(0) {}

ConfigParser::~ConfigParser() {}

std::string ConfigParser::readFile(const std::string& path)
{
	std::ifstream file(path.c_str());
	if (!file.is_open())
		throw std::runtime_error("ConfigParser: cannot open " + path);
	
	std::ostringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

std::vector<std::string> ConfigParser::tokenize(const std::string& content)
{
	std::vector<std::string> tokens;
	std::string current;

	for (size_t i = 0; i < content.size(); ++i)
	{
		char c = content[i];
		
		//ignore until the EOL
		if (c == '#')
		{
			while (i < content.size() && content[i] != '\n')
				++i;
			continue;
		}

		if (c == '{' || c == '}' || c == ';')
		{
			if (!current.empty())
			{
				tokens.push_back(current);
				current.clear();
			}
			tokens.push_back(std::string(1, c));
			continue;
		}

		if (std::isspace(static_cast<unsigned char>(c)))
		{
			if (!current.empty())
			{
				tokens.push_back(current);
				current.clear();
			}
			continue;
		}

		current += c;
	}
	
	if (!current.empty())
		tokens.push_back(current);
	
	return tokens;
}

std::string ConfigParser::peek() const
{
	if (_pos >= _tokens.size())
		throw std::runtime_error("ConfigParser: unexpected end of file");
	return _tokens[_pos];
}

std::string ConfigParser::next()
{
	if (_pos >= _tokens.size())
		throw std::runtime_error("ConfigParser: unexpected end of file");
	return _tokens[_pos++];
}

void ConfigParser::expect(const std::string& expected)
{
	std::string tok = next();
	if (tok != expected)
		throw std::runtime_error("ConfigParser: expected '" + expected + "' but find '" + tok + "'");
}

//Converts "10M , 500K, 2G" or a raw nb (octets) in octets
std::size_t ConfigParser::parseSize(const std::string& value)
{
	if (value.empty())
		throw std::runtime_error("ConfigParser: size empty");
	
	std::string digits = value;
	std::size_t multiplier = 1;
	char suffix = static_cast<char>(std::toupper(static_cast<unsigned char>(value[value.size() - 1])));
	
	if (suffix == 'K' || suffix == 'M' || suffix == 'G')
	{
		digits = value.substr(0, value.size() - 1);
		if (suffix == 'K')
			multiplier = 1024;
		else if (suffix == 'M')
			multiplier = 1024 * 1024;
		else
			multiplier = 1024 * 1024 * 1024;
	}
	
	std::istringstream iss(digits);
	long number;
	std::string extra;
	if (!(iss >> number) || (iss >> extra) || number < 0)
		throw std::runtime_error("ConfigParser: invalid size '" + value + "'");
	
	return static_cast<std::size_t>(number) * multiplier;
}


std::string ConfigParser::peekServerRoot() const
{
	std::size_t pos = _pos;
	int depth = 0;

	while (pos < _tokens.size())
	{
		const std::string& tok = _tokens[pos];
		
		if (tok == "{")
		{
			++depth;
		}
		else if (tok == "}")
		{
			if (depth == 0)
				break;
			--depth;
		}
		else if (tok == "root" && depth == 0 && pos + 1 < _tokens.size())
		{
			return _tokens[pos + 1];
		}
		++pos;
	}
	return "www";
}

LocationConfig ConfigParser::parseLocation(const std::string& serverRoot)
{
	LocationConfig loc;
	
	loc.setPath(next());
	//inherit root from default server, erase further down if 'root' explicit in location bloc
	loc.setRoot(serverRoot); 
	expect("{");

	while (peek() != "}")
	{
		std::string directive = next();
		
		if (directive == "root")
		{
			loc.setRoot(next());
			expect(";");
		}
		else if (directive == "index")
		{
			loc.setIndex(next());
			expect(";");
		}
		else if (directive == "allow_methods")
		{
			while (peek() != ";")
				loc.addAllowedMethod(next());
			expect(";");
		}
		else if (directive == "redirect")
		{
			loc.setRedirect(next());
			expect(";");
		}
		else if (directive == "autoindex")
		{
			std::string val = next();
			loc.setAutoindex(val == "on");
			expect(";");
		}
		else if (directive == "upload_enabled")
		{
			std::string val = next();
			loc.setUploadEnabled(val == "on");
			expect(";");
		}
		else if (directive == "upload_path")
		{
			loc.setUploadPath(next());
			expect(";");
		}
		else if (directive == "cgi_extension")
		{
			std::string ext = next();
			std::string interpreter = next();
			loc.addCgiExtension(ext, interpreter);
			expect(";");
		}
		else
			throw std::runtime_error("ConfigParser: unknown directive in location: " + directive);
	}
	expect("}");
	
	return loc;
}

ServerConfig ConfigParser::parseServer()
{
	ServerConfig server;
	std::set<std::string> seenDirectives;

	expect("{");

	std::string serverRoot = peekServerRoot();

	while (peek() != "}")
	{
		std::string directive = next();

		if (directive == "listen" || directive == "root" || directive == "client_max_body_size")
		{
			if (seenDirectives.find(directive) != seenDirectives.end())
				throw std::runtime_error("ConfigParser: duplicate directive '" + directive + "' in server block");
			seenDirectives.insert(directive);
		}
		
		if (directive == "listen")
		{
			std::istringstream iss(next());
			unsigned int port;
			std::string extra;
			if (!(iss >> port) || (iss >> extra))
				throw std::runtime_error("ConfigParser: invalid port");
			if (port < 1 || port > 65535)
				throw std::runtime_error("ConfigParser: port out of range (1-65535)");
			server.setPort(port);
			expect(";");
		}
		else if (directive == "root")
		{
			server.setRoot(next());
			expect(";");
		}
		else if (directive == "client_max_body_size")
		{
			server.setClientMaxBodySize(parseSize(next()));
			expect(";");
		}
		else if (directive == "error_page")
		{
			int code;
			std::string extra;
			std::istringstream iss(next());
			if (!(iss >> code) || (iss >> extra))
				throw std::runtime_error("ConfigParser: invalid error code");
			std::string errorPath = next();
			server.setErrorPage(code, errorPath);
			expect(";");
		}
		else if (directive == "location")
		{
			server.addLocation(parseLocation(serverRoot));
		}
		else
			throw std::runtime_error("ConfigParser: unknown directive in server: " + directive);
	}
	expect("}");
	
	return server;
}

std::vector<ServerConfig>	ConfigParser::parse(const std::string& path)
{
	std::string content = readFile(path);
	_tokens = tokenize(content);
	_pos = 0;

	std::vector<ServerConfig> servers;

	while (_pos < _tokens.size())
	{
		std::string tok = next();
		if (tok != "server")
			throw std::runtime_error("ConfigParser: expecting 'server' but find '" + tok + "'");
		servers.push_back(parseServer());
	}
	
	if (servers.empty())
		throw std::runtime_error("ConfigParser: no server bloc found");
	
	return servers;
}