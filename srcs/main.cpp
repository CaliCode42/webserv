/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/22 14:50:12 by tcali             #+#    #+#             */
/*   Updated: 2026/09/02 11:53:42 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "../http/MethodHandler.hpp"
#include "../http/HttpRequest.hpp"
#include "ConfigParser.hpp"
#include <signal.h>

sig_atomic_t	g_running = 1;

void	handleSignal(int)
{
    g_running = 0;
}

int	main(int ac, char **av)
{
	if (ac > 2)
	{
		std::cout << "Error : Usage ./webserv [file.conf]" << std::endl;
		return (1);
	}

	try {
		ConfigParser	parser;

		std::string configPath;
        if (ac == 2)
            configPath = av[1];
        else
            configPath = "webserv.conf";

		const std::vector<ServerConfig>& configs = parser.parse(configPath);

		Server server(configs);

		server.initSockets();

		if (signal(SIGINT, handleSignal) == SIG_ERR)
			throw std::runtime_error("failed to set SIGINT handler");

		if (signal(SIGTERM, handleSignal) == SIG_ERR)
			throw std::runtime_error("failed to set SIGTERM handler");
		
		server.run();
	}
	catch (std::runtime_error&	e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return (1);
	}
	return (0);
}
