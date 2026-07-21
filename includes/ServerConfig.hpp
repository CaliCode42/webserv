/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/20 15:35:09 by tcali             #+#    #+#             */
/*   Updated: 2026/07/20 15:48:59 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <iostream>
#include <string>

class ServerConfig
{
private:
	std::string	_root;

public:
	ServerConfig();
	// ServerConfig(const ServerConfig& other);
	// ServerConfig& operator=(const ServerConfig& other);
	~ServerConfig();

	std::string	getRoot() const;
};

#endif