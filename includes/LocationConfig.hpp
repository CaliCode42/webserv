/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/16 15:29:36 by sdossa            #+#    #+#             */
/*   Updated: 2026/09/25 12:27:32 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>

class LocationConfig
{
private:
	std::string _path;
	std::string _root;
	std::string _index;
	std::vector<std::string> _allowedMethods;
	bool _autoindex;
	int _redirectCode;
	std::string _redirect;
	bool _uploadEnabled;
	bool _uploadEnabledIsExplicit;
	std::string _uploadPath;
	std::map<std::string, std::string> _cgiExtensions;
	bool _rootIsExplicit;

public:
	LocationConfig();
	~LocationConfig();

	std::string getPath() const;
	void setPath(const std::string& path);

	std::string getRoot() const;
	void setRoot(const std::string& root);

	std::string getIndex() const;
	void setIndex(const std::string& index);

	const std::vector<std::string>& getAllowedMethods() const;
	void addAllowedMethod(const std::string& method);
	bool isMethodAllowed(const std::string& method) const;

	bool getAutoindex() const;
	void setAutoindex(bool value);

	std::string getRedirect() const;
	int getRedirectCode() const;
	void setRedirect(const std::string& target, int code);
	bool hasRedirect() const;

	bool getUploadEnabled() const;
	void setUploadEnabled(bool value);
	bool isUploadEnabledExplicit() const;

	std::string getUploadPath() const;
	void setUploadPath(const std::string& path);

	const std::map<std::string, std::string>& getCgiExtensions() const;
	void addCgiExtension(const std::string& ext, const std::string& interpreterPath);
	bool isCgiExtension(const std::string& ext) const;
	std::string getCgiPath(const std::string& extension) const;

	bool hasExplicitRoot() const;
	void setRootExplicit(bool value);
};

#endif
