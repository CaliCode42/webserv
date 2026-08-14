/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 10:11:50 by tcali             #+#    #+#             */
/*   Updated: 2026/08/14 10:15:26 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <string>
#include <vector>

class LocationConfig
{
private:
	std::string						_path;
	std::string						_root;
	std::string						_index;
	std::vector<std::string>		_allowedMethods;
	bool							_autoindex;

public:
	LocationConfig();
	~LocationConfig();

	const std::string& 				getPath()const;
	const std::string& 				getRoot()const;
	const std::string& 				getIndex()const;
	const std::vector<std::string>&	getAllowedMethods()const;
	bool							getAutoindex()const;

	void							setPath(const std::string& path);
	void							setRoot(const std::string& root);
	void							setIndex(const std::string& index);
	void							setAllowedMethods(const std::vector<std::string>& methods);
	void							setAutoindex(bool value);

	bool							isMethodAllowed(const std::string& method)const;
};

#endif