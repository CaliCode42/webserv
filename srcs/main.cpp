/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/22 14:50:12 by tcali             #+#    #+#             */
/*   Updated: 2026/08/18 21:08:05 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// Real main to test the server
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

// Tests for CGI implementation

#include "ConfigParser.hpp"
#include "CgiHandler.hpp"
#include "HttpRequest.hpp"
#include "CgiProcess.hpp"

#include <iostream>
#include <string>
#include <stdexcept>
#include <fcntl.h>
#include <ctime>
#include <unistd.h>

#define GREEN "\033[32m"
#define RED   "\033[31m"
#define RESET "\033[0m"

struct CgiTest
{
    std::string uri;
    bool        expectedCgi;
    std::string expectedInterpreter;
};

struct ScriptTest
{
    std::string uri;
    std::string expectedScript;
    std::string expectedQuery;
};

struct EnvTest
{
    std::string key;
    std::string expected;
};

static bool hasCgiLocation(const ServerConfig& config)
{
    const locationVector& locations = config.getLocations();

    for (locationVector::const_iterator it = locations.begin();
         it != locations.end();
         ++it)
    {
        if (it->getPath() == "/cgi")
        {
            std::cout << "CGI location found:" << std::endl;
            std::cout << "path: " << it->getPath() << std::endl;
            std::cout << "root: " << it->getRoot() << std::endl;
            std::cout << std::endl;

            return (true);
        }
    }

    return (false);
}

static void testCgiInterpreter(
    const ServerConfig& config,
    const CgiTest tests[],
    std::size_t count)
{
    std::cout << "===== CGI detection / interpreter =====" << std::endl;

    for (std::size_t i = 0; i < count; ++i)
    {
        const LocationConfig* location =
            config.findLocation(tests[i].uri);

        std::cout << tests[i].uri << std::endl;

        if (location == NULL)
        {
            std::cout << RED
                      << "[FAIL] no matching location"
                      << RESET
                      << std::endl;
            continue;
        }

        bool cgi =
            CgiHandler::isCgiRequest(tests[i].uri, *location);

        std::string interpreter =
            CgiHandler::getInterpreter(tests[i].uri, *location);

        if (cgi == tests[i].expectedCgi
            && interpreter == tests[i].expectedInterpreter)
        {
            std::cout << GREEN
                      << "[OK]"
                      << RESET
                      << std::endl;
        }
        else
        {
            std::cout << RED
                      << "[FAIL]"
                      << RESET
                      << std::endl;

            std::cout << "  CGI: "
                      << cgi
                      << " (expected "
                      << tests[i].expectedCgi
                      << ")"
                      << std::endl;

            std::cout << "  interpreter: \""
                      << interpreter
                      << "\" (expected \""
                      << tests[i].expectedInterpreter
                      << "\")"
                      << std::endl;
        }
    }

    std::cout << std::endl;
}

static void testScriptAndQuery(
    const ServerConfig& config,
    const ScriptTest tests[],
    std::size_t count)
{
    std::cout << "===== CGI script / query resolution =====" << std::endl;

    for (std::size_t i = 0; i < count; ++i)
    {
        const LocationConfig* location =
            config.findLocation(tests[i].uri);

        std::cout << tests[i].uri << std::endl;

        if (location == NULL)
        {
            std::cout << RED
                      << "[FAIL] no matching location"
                      << RESET
                      << std::endl;
            continue;
        }

        std::string script =
            CgiHandler::resolveScriptPath(
                tests[i].uri,
                *location);

        std::string query =
            CgiHandler::getQueryString(
                tests[i].uri);

        if (script == tests[i].expectedScript)
        {
            std::cout << GREEN
                      << "  script [OK]: "
                      << script
                      << RESET
                      << std::endl;
        }
        else
        {
            std::cout << RED
                      << "  script [FAIL]: "
                      << script
                      << " (expected "
                      << tests[i].expectedScript
                      << ")"
                      << RESET
                      << std::endl;
        }

        if (query == tests[i].expectedQuery)
        {
            std::cout << GREEN
                      << "  query [OK]: "
                      << query
                      << RESET
                      << std::endl;
        }
        else
        {
            std::cout << RED
                      << "  query [FAIL]: "
                      << query
                      << " (expected "
                      << tests[i].expectedQuery
                      << ")"
                      << RESET
                      << std::endl;
        }
    }

    std::cout << std::endl;
}

static void checkEnvValue(
    const envMap& env,
    const std::string& key,
    const std::string& expected)
{
    envMap::const_iterator it = env.find(key);

    if (it == env.end())
    {
        std::cout << RED
                  << "[FAIL] "
                  << key
                  << " is missing"
                  << RESET
                  << std::endl;
        return;
    }

    if (it->second == expected)
    {
        std::cout << GREEN
                  << "[OK] "
                  << key
                  << "="
                  << it->second
                  << RESET
                  << std::endl;
    }
    else
    {
        std::cout << RED
                  << "[FAIL] "
                  << key
                  << "=\""
                  << it->second
                  << "\" (expected \""
                  << expected
                  << "\")"
                  << RESET
                  << std::endl;
    }
}

static bool isNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);

	if (flags == -1)
		return (false);

	return ((flags & O_NONBLOCK) != 0);
}

static bool waitForCgiProcess(CgiProcess& process)
{
    std::time_t startTime = std::time(NULL);
    const std::time_t timeout = 3;

    while (!process.isFinished())
    {
        if (!process.readOutput())
            return (false);

        process.checkProcessStatus();

        if (std::time(NULL) - startTime >= timeout)
            return (false);

        usleep(1000);
    }

    return (true);
}

static void testCgiProcessStart()
{
	std::cout << "===== CGI process start =====" << std::endl;

	CgiProcess process;
	CgiProcess::envMap env;

	env["REQUEST_METHOD"] = "GET";
	env["QUERY_STRING"] = "";
	env["SCRIPT_FILENAME"] = "www/cgi-bin/test.py";
	env["SCRIPT_NAME"] = "/cgi/test.py";
	env["SERVER_PROTOCOL"] = "HTTP/1.1";
	env["CONTENT_LENGTH"] = "";
	env["CONTENT_TYPE"] = "";

	bool started = process.start(
		"/usr/bin/python3",
		"www/cgi-bin/test.py",
		env,
		"");

	if (!started)
	{
		std::cout << RED
				  << "[FAIL] CgiProcess::start() returned false"
				  << RESET
				  << std::endl;
		return;
	}

	if (process.isStarted())
		std::cout << GREEN << "[OK] process is started" << RESET << std::endl;
	else
		std::cout << RED << "[FAIL] process is not marked as started"
				  << RESET << std::endl;

	if (process.getPid() > 0)
		std::cout << GREEN << "[OK] child PID: "
				  << process.getPid()
				  << RESET << std::endl;
	else
		std::cout << RED << "[FAIL] invalid child PID"
				  << RESET << std::endl;

	if (process.getStdinFd() == -1)
	{
		std::cout << GREEN
				<< "[OK] CGI stdin is closed for empty body"
				<< RESET
				<< std::endl;
	}
	else
	{
		std::cout << RED
				<< "[FAIL] CGI stdin should be closed for empty body"
				<< RESET
				<< std::endl;
	}

	if (process.getStdoutFd() >= 0)
		std::cout << GREEN << "[OK] CGI stdout FD: "
				  << process.getStdoutFd()
				  << RESET << std::endl;
	else
		std::cout << RED << "[FAIL] invalid CGI stdout FD"
				  << RESET << std::endl;

	bool secondStart = process.start(
		"/usr/bin/python3",
		"www/cgi-bin/test.py",
		env,
		"");

	if (!secondStart)
		std::cout << GREEN
				  << "[OK] second start is rejected"
				  << RESET
				  << std::endl;
	else
		std::cout << RED
				  << "[FAIL] second start was accepted"
				  << RESET
				  << std::endl;

	std::cout << std::endl;

	// if (isNonBlocking(process.getStdinFd()))
	// std::cout << GREEN
	// 		  << "[OK] CGI stdin is non-blocking"
	// 		  << RESET
	// 		  << std::endl;
	// else
	// 	std::cout << RED
	// 			<< "[FAIL] CGI stdin is blocking"
	// 			<< RESET
	// 			<< std::endl;

	if (isNonBlocking(process.getStdoutFd()))
		std::cout << GREEN
				<< "[OK] CGI stdout is non-blocking"
				<< RESET
				<< std::endl;
	else
		std::cout << RED
				<< "[FAIL] CGI stdout is blocking"
				<< RESET
				<< std::endl;
	if (!waitForCgiProcess(process))
	{
		std::cout << RED
				<< "[FAIL] CGI process cleanup failed"
				<< RESET
				<< std::endl;
	}
}

static void testCgiProcessOutput()
{
	std::cout << "===== CGI process output / completion =====" << std::endl;

	CgiProcess process;
	CgiProcess::envMap env;

	env["REQUEST_METHOD"] = "GET";
	env["QUERY_STRING"] = "";
	env["SCRIPT_FILENAME"] = "www/cgi-bin/test.py";
	env["SCRIPT_NAME"] = "/cgi/test.py";
	env["SERVER_PROTOCOL"] = "HTTP/1.1";
	env["CONTENT_LENGTH"] = "";
	env["CONTENT_TYPE"] = "";

	if (!process.start(
			"/usr/bin/python3",
			"www/cgi-bin/test.py",
			env,
			""))
	{
		std::cout << RED
				  << "[FAIL] could not start CGI process"
				  << RESET
				  << std::endl;
		return;
	}

	std::time_t startTime = std::time(NULL);
	const std::time_t timeout = 3;

	while (!process.isFinished())
	{
		if (!process.readOutput())
		{
			std::cout << RED
					  << "[FAIL] error while reading CGI output"
					  << RESET
					  << std::endl;
			return;
		}

		process.checkProcessStatus();

		if (std::time(NULL) - startTime >= timeout)
		{
			std::cout << RED
					  << "[FAIL] CGI process did not finish before timeout"
					  << RESET
					  << std::endl;
			return;
		}

		usleep(1000);
	}

	std::cout << GREEN
			  << "[OK] CGI process finished"
			  << RESET
			  << std::endl;

	const std::string& output = process.getOutput();

	if (!output.empty())
	{
		std::cout << GREEN
				  << "[OK] CGI output was captured"
				  << RESET
				  << std::endl;

		std::cout << "----- CGI output -----" << std::endl;
		std::cout << output;
		std::cout << "----------------------" << std::endl;
	}
	else
	{
		std::cout << RED
				  << "[FAIL] CGI output is empty"
				  << RESET
				  << std::endl;
	}

	std::cout << std::endl;
}

static void testGetEnvironment(const ServerConfig& config)
{
    std::cout << "===== CGI environment: GET =====" << std::endl;

    HttpRequest request;

    std::string rawRequest =
        "GET /cgi/test.py?name=Bob HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";

    request.appendData(
        rawRequest.c_str(),
        rawRequest.size());

    if (request.hasError() || !request.isComplete())
        throw std::runtime_error(
            "Could not build GET request for CGI environment test");

    const std::string& uri = request.getUri();

    const LocationConfig* location =
        config.findLocation(uri);

    if (location == NULL)
        throw std::runtime_error(
            "No location found for GET CGI environment test");

    std::string scriptPath =
        CgiHandler::resolveScriptPath(uri, *location);

    envMap env =
        CgiHandler::buildEnvironment(
            request,
            uri,
            *location,
            scriptPath);

    checkEnvValue(env, "REQUEST_METHOD", "GET");
    checkEnvValue(env, "QUERY_STRING", "name=Bob");
    checkEnvValue(env, "CONTENT_LENGTH", "");
    checkEnvValue(env, "CONTENT_TYPE", "");
    checkEnvValue(
        env,
        "SCRIPT_FILENAME",
        "www/cgi-bin/test.py");
    checkEnvValue(
        env,
        "SCRIPT_NAME",
        "/cgi/test.py");
    checkEnvValue(
        env,
        "SERVER_PROTOCOL",
        "HTTP/1.1");

    std::cout << std::endl;
}

static void testPostEnvironment(const ServerConfig& config)
{
    std::cout << "===== CGI environment: POST =====" << std::endl;

    HttpRequest request;

    std::string rawRequest =
        "POST /cgi/test.py HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "Hello";

    request.appendData(
        rawRequest.c_str(),
        rawRequest.size());

    if (request.hasError() || !request.isComplete())
        throw std::runtime_error(
            "Could not build POST request for CGI environment test");

    const std::string& uri = request.getUri();

    const LocationConfig* location =
        config.findLocation(uri);

    if (location == NULL)
        throw std::runtime_error(
            "No location found for POST CGI environment test");

    std::string scriptPath =
        CgiHandler::resolveScriptPath(uri, *location);

    envMap env =
        CgiHandler::buildEnvironment(
            request,
            uri,
            *location,
            scriptPath);

    checkEnvValue(env, "REQUEST_METHOD", "POST");
    checkEnvValue(env, "QUERY_STRING", "");
    checkEnvValue(env, "CONTENT_LENGTH", "5");
    checkEnvValue(env, "CONTENT_TYPE", "text/plain");
    checkEnvValue(
        env,
        "SCRIPT_FILENAME",
        "www/cgi-bin/test.py");
    checkEnvValue(
        env,
        "SCRIPT_NAME",
        "/cgi/test.py");
    checkEnvValue(
        env,
        "SERVER_PROTOCOL",
        "HTTP/1.1");

    std::cout << std::endl;
}

int main()
{
    CgiTest cgiTests[] = {
        {"/cgi/test.py",          true,  "/usr/bin/python3"},
        {"/cgi/test.php",         true,  "/usr/bin/php-cgi"},
        {"/cgi/test.html",        false, ""},
        {"/cgi/test",             false, ""},
        {"/cgi/test.py?name=Bob", true,  "/usr/bin/python3"},
        {"/cgi.v1/test.py",       false, ""},
        {"/cgi.v1/script",        false, ""}
    };

    ScriptTest scriptTests[] = {
        {"/cgi/test.py",
         "www/cgi-bin/test.py",
         ""},

        {"/cgi/test.py?name=Bob",
         "www/cgi-bin/test.py",
         "name=Bob"},

        {"/cgi/subdir/test.py",
         "www/cgi-bin/subdir/test.py",
         ""}
    };

    try
    {
        ConfigParser parser;
        ServerConfig config = parser.parse("test.conf");

        std::cout << "locations: "
                  << config.getLocations().size()
                  << std::endl
                  << std::endl;

        if (!hasCgiLocation(config))
            throw std::runtime_error(
                "No /cgi location defined");

        std::size_t cgiTestCount =
            sizeof(cgiTests) / sizeof(cgiTests[0]);

        std::size_t scriptTestCount =
            sizeof(scriptTests) / sizeof(scriptTests[0]);

        testCgiInterpreter(
            config,
            cgiTests,
            cgiTestCount);

        testScriptAndQuery(
            config,
            scriptTests,
            scriptTestCount);

        testGetEnvironment(config);
        testPostEnvironment(config);

		testCgiProcessStart();
		testCgiProcessOutput();
    }
    catch (const std::exception& e)
    {
        std::cerr << RED
                  << "Test error: "
                  << e.what()
                  << RESET
                  << std::endl;

        return (1);
    }

    return (0);
}

// Tests for LocationConfig & ConfigParser (CGI path oriented)

// int main()
// {
// 	struct LocationTest
// 	{
// 		const char* uri;
// 		const char* expected;
// 	};

// 	LocationTest tests[] = {
// 		{"/",                         "/"},
// 		{"/index.html",               "/"},
// 		{"/images",                   "/images"},
// 		{"/images/",                  "/images"},
// 		{"/images/cat.png",           "/images"},
// 		{"/images/private",           "/images/private"},
// 		{"/images/private/cat.png",   "/images/private"},
// 		{"/images2/cat.png",          "/"},
// 		{"/cgi/test.py",              "/cgi"},
// 		{"/cgibin/test.py",           "/"},
// 		{"/unknown",                  "/"}
// 	};
//     try
//     {
//         ConfigParser parser;
// 		ServerConfig config = parser.parse("test.conf");

// 		const locationVector& locations = config.getLocations();

// 		std::cout << "locations: " << locations.size() << std::endl;

// 		for (locationVector::const_iterator it = locations.begin();
// 			it != locations.end();
// 			++it)
// 		{
// 			std::cout << "path: " << it->getPath() << std::endl;
// 			std::cout << "root: " << it->getRoot() << std::endl;
// 		}
		
// 		std::size_t count = sizeof(tests) / sizeof(tests[0]);

// 		for (std::size_t i = 0; i < count; ++i)
// 		{
// 			const LocationConfig* location =
// 				config.findLocation(tests[i].uri);

// 			std::cout << tests[i].uri << " -> ";

// 			if (location == NULL)
// 			{
// 				std::cout << "NULL";
// 			}
// 			else
// 			{
// 				std::cout << location->getPath();

// 				if (location->getPath() == tests[i].expected)
// 					std::cout << " [OK]";
// 				else
// 					std::cout << " [FAIL: " << location->getPath()
// 							<< " (expected :"
// 							<< tests[i].expected << ")]";
// 			}

// 			std::cout << std::endl;
// 		}
//     }
//     catch (const std::exception& e)
//     {
//         std::cerr << e.what() << std::endl;
//         return (1);
//     }

//     return (0);
// }