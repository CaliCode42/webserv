/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 10:11:50 by tcali             #+#    #+#             */
/*   Updated: 2026/08/24 19:29:33 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <string>
#include <vector>
#include <map>

typedef std::string					string;
typedef std::map<string, string>	stringMap;
typedef std::vector<string>			stringVector;

class LocationConfig
{
private:
	string				_path;
	string				_root;
	string				_index;
	stringVector		_allowedMethods;
	bool				_autoIndex;
	
	stringMap			_cgiHandlers;

public:
	LocationConfig() : _autoIndex(false) {};
	~LocationConfig() {};

	const string& 				getPath()const;
	const string& 				getRoot()const;
	const string& 				getIndex()const;
	const stringVector&			getAllowedMethods()const;
	bool						getAutoIndex()const;

	const stringMap&			getCgiHandlers()const;
	string						getCgiPath(const string& extension)const;

	void						setPath(const string& path);
	void						setRoot(const string& root);
	void						setIndex(const string& index);
	void						setAllowedMethods(const stringVector& methods);
	void						setAutoIndex(bool value);

	void						setCgiHandlers(const stringMap& extensions);

	void						addCgiHandler(const string& extension, const string& executable);

	bool						isCgiEnabled()const;
	bool						isMethodAllowed(const string& method)const;
	bool						isCgiExtension(const string& extension)const;
};

#endif