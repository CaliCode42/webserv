/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/22 14:50:12 by tcali             #+#    #+#             */
/*   Updated: 2026/08/24 19:28:46 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "MethodHandler.hpp"
#include "HttpRequest.hpp"
#include "ConfigParser.hpp"

int main()
{
	ConfigParser	parser;
	ServerConfig	config = parser.parse("test.conf");
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