/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:16:17 by tcali             #+#    #+#             */
/*   Updated: 2026/05/29 18:40:57 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>

class Client
{
private:
	

public:
	int			_fd;
	std::string	_buffer;

	Client(int fd);
	// Client(const Client& other);
	// Client& operator=(const Client& other);
	~Client();
};

#endif