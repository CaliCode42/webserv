/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/17 10:31:09 by tcali             #+#    #+#             */
/*   Updated: 2026/08/17 12:20:57 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"

LocationConfig::LocationConfig(): _autoIndex(false) {}

LocationConfig::~LocationConfig() {}

const string&	LocationConfig::getPath()const
{
	return (_path);
}

const string&	LocationConfig::getRoot()const
{
	return (_root);
}

const string&	LocationConfig::getIndex()const
{
	return (_index);
}

const stringVector&	LocationConfig::getAllowedMethods()const
{
	return (_allowedMethods);
}

bool	LocationConfig::getAutoIndex()const
{
	return (_autoIndex);
}

const stringMap&	LocationConfig::getCgiHandlers()const
{
	return (_cgiHandlers);
}

string	LocationConfig::getCgiPath(const string& extension)const
{
	stringMap::const_iterator	it;

	it = _cgiHandlers.find(extension);
	if (it  == _cgiHandlers.end())
		return ("");

	return (it->second);
}

void	LocationConfig::setPath(const string& path)
{
	_path = path;
}

void	LocationConfig::setRoot(const string& root)
{
	_root = root;
}

void	LocationConfig::setIndex(const string& index)
{
	_index = index;
}

void	LocationConfig::setAllowedMethods(const stringVector& methods)
{
	_allowedMethods = methods;
}
void	LocationConfig::setAutoIndex(bool value)
{
	_autoIndex = value;
}

void	LocationConfig::setCgiHandlers(const stringMap& handlers)
{
	_cgiHandlers = handlers;
}

void	LocationConfig::addCgiHandler(const string& extension, const string& executable)
{
	_cgiHandlers[extension] = executable;
}

bool	LocationConfig::isCgiEnabled()const
{
	return (!_cgiHandlers.empty());
}

bool	LocationConfig::isMethodAllowed(const string& method)const
{
	stringVector::const_iterator	it;

	for (it = _allowedMethods.begin(); it != _allowedMethods.end(); ++it)
	{
		if (*it == method)
			return (true);
	}

	return (false);
}

bool	LocationConfig::isCgiExtension(const string& extension)const
{
    return (_cgiHandlers.find(extension) != _cgiHandlers.end());
}