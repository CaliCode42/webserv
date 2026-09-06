/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdossa <sdossa@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/16 15:29:03 by sdossa            #+#    #+#             */
/*   Updated: 2026/09/06 23:11:15 by sdossa           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"

LocationConfig::LocationConfig()
	: _path(""), _root("www"), _index("index.html"), _autoindex(false),
	  _redirect(""), _uploadEnabled(false), _uploadPath(""), _rootIsExplicit(false)
{}

LocationConfig::~LocationConfig()
{}

std::string LocationConfig::getPath() const
{
	return (_path);
}

void LocationConfig::setPath(const std::string& path)
{
	_path = path;
}

std::string LocationConfig::getRoot() const
{
	return (_root);
}

void LocationConfig::setRoot(const std::string& root)
{
	_root = root;
}

bool LocationConfig::hasExplicitRoot() const
{
	return (_rootIsExplicit);
}

void LocationConfig::setRootExplicit(bool value)
{
	_rootIsExplicit = value;
}

std::string LocationConfig::getIndex() const
{
	return (_index);
}

void LocationConfig::setIndex(const std::string& index)
{
	_index = index;
}

const std::vector<std::string>& LocationConfig::getAllowedMethods() const
{
	return (_allowedMethods);
}

void LocationConfig::addAllowedMethod(const std::string& method)
{
	_allowedMethods.push_back(method);
}

bool LocationConfig::isMethodAllowed(const std::string& method) const
{
	for (std::vector<std::string>::const_iterator methodIt = _allowedMethods.begin();
		methodIt != _allowedMethods.end(); ++methodIt)
	{
		if (*methodIt == method)
			return true;
	}
	return false;
}

bool LocationConfig::getAutoindex() const
{
	return (_autoindex);
}

void LocationConfig::setAutoindex(bool value)
{
	_autoindex = value;
}

std::string LocationConfig::getRedirect() const
{
	return (_redirect);
}

void LocationConfig::setRedirect(const std::string& target, int code)
{
	_redirect = target;
	_redirectCode = code;
}

int LocationConfig::getRedirectCode() const
{
	return _redirectCode;
}

bool LocationConfig::hasRedirect() const
{
	return (!_redirect.empty());
}

bool LocationConfig::getUploadEnabled() const
{
	return (_uploadEnabled);
}

void LocationConfig::setUploadEnabled(bool value)
{
	_uploadEnabled = value;
}

std::string LocationConfig::getUploadPath() const
{
	return (_uploadPath);
}

void LocationConfig::setUploadPath(const std::string& path)
{
	_uploadPath = path;
}

const std::map<std::string, std::string>& LocationConfig::getCgiExtensions() const
{
	return (_cgiExtensions);
}

void LocationConfig::addCgiExtension(const std::string& ext, const std::string& interpreterPath)
{
	_cgiExtensions[ext] = interpreterPath;
}

bool LocationConfig::isCgiExtension(const std::string& ext) const
{
	return (_cgiExtensions.find(ext) != _cgiExtensions.end());
}

std::string LocationConfig::getCgiPath(const std::string& extension) const
{
	std::map<std::string, std::string>::const_iterator cgiIt = _cgiExtensions.find(extension);
	if (cgiIt == _cgiExtensions.end())
		return "";
	return cgiIt->second;
}
