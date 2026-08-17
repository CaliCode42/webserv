/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/22 14:50:12 by tcali             #+#    #+#             */
/*   Updated: 2026/08/17 18:48:05 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// #include "Server.hpp"
// #include "../http/MethodHandler.hpp"
// #include "../http/HttpRequest.hpp"

// int main()
// {
// 	ServerConfig	config;
//     Server			server(8080, config);
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

#include "ConfigParser.hpp"

#include <iostream>

int main()
{
	struct LocationTest
	{
		const char* uri;
		const char* expected;
	};

	LocationTest tests[] = {
		{"/",                         "/"},
		{"/index.html",               "/"},
		{"/images",                   "/images"},
		{"/images/",                  "/images"},
		{"/images/cat.png",           "/images"},
		{"/images/private",           "/images/private"},
		{"/images/private/cat.png",   "/images/private"},
		{"/images2/cat.png",          "/"},
		{"/cgi/test.py",              "/cgi"},
		{"/cgibin/test.py",           "/"},
		{"/unknown",                  "/"}
	};
    try
    {
        ConfigParser parser;
		ServerConfig config = parser.parse("test.conf");

		const locationVector& locations = config.getLocations();

		std::cout << "locations: " << locations.size() << std::endl;

		for (locationVector::const_iterator it = locations.begin();
			it != locations.end();
			++it)
		{
			std::cout << "path: " << it->getPath() << std::endl;
			std::cout << "root: " << it->getRoot() << std::endl;
		}
		
		std::size_t count = sizeof(tests) / sizeof(tests[0]);

		for (std::size_t i = 0; i < count; ++i)
		{
			const LocationConfig* location =
				config.findLocation(tests[i].uri);

			std::cout << tests[i].uri << " -> ";

			if (location == NULL)
			{
				std::cout << "NULL";
			}
			else
			{
				std::cout << location->getPath();

				if (location->getPath() == tests[i].expected)
					std::cout << " [OK]";
				else
					std::cout << " [FAIL: " << location->getPath()
							<< " (expected "
							<< tests[i].expected << "])";
			}

			std::cout << std::endl;
		}
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return (1);
    }

    return (0);
}