/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/22 14:50:12 by tcali             #+#    #+#             */
/*   Updated: 2026/08/27 23:36:58 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// Real main to test the server
#include "Server.hpp"
#include "../http/MethodHandler.hpp"
#include "../http/HttpRequest.hpp"
#include "ConfigParser.hpp"

// int main()
// {
// 	ConfigParser	parser;
// 	//ServerConfig	config = parser.parse("test.conf");
//     //Server			server(8080, config);
// 	std::vector<ServerConfig> configs = parser.parse("test.conf");
// 	ServerConfig config = configs[0];
// 	Server server(config.getPort(), config);
//     try {
// 		server.initSocket();
//     	server.run();
// 	}
// 	catch (std::runtime_error&	e)
// 	{
// 		std::cerr << e.what();
// 	}
//     return (0);
// }


int main()
{
	ConfigParser	parser;

	try {
		const std::vector<ServerConfig>& configs = parser.parse("test.conf");

		Server server(configs);

		server.initSockets();
		server.run();
	}
	catch (std::runtime_error&	e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return (1);
	}
	return (0);
}
