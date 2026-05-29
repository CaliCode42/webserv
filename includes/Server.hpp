/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:16:04 by tcali             #+#    #+#             */
/*   Updated: 2026/05/29 18:36:50 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>

class Server
{
private:
	int					_serverSocket;
	std::vector<pollfd>	_fds;

public:
	Server(int serverSocket);
	// Server(const Server& other);
	// Server& operator=(const Server& other);
	~Server();
};

#endif