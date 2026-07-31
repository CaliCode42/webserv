/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/22 14:50:12 by tcali             #+#    #+#             */
/*   Updated: 2026/07/31 16:26:33 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "../http/MethodHandler.hpp"
#include "../http/HttpRequest.hpp"

int main()
{
	ServerConfig	config;
    Server			server(8080, config);
    try {
		server.initSocket();
    	server.run();
	}
	catch (std::runtime_error&	e)
	{
		std::cerr << e.what();
	}
    return (0);
}
