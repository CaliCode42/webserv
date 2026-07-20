/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/15 17:14:42 by tcali             #+#    #+#             */
/*   Updated: 2026/07/20 13:27:27 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include <iostream>
#include <exception>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

class RequestHandler
{
private:
	HttpResponse	handleGet(const HttpRequest& request);
	bool 			hasExtension(const std::string& path,
		const std::string& extension) const;
public:
	RequestHandler();
	// RequestHandler(const RequestHandler& other);
	// RequestHandler& operator=(const RequestHandler& other);
	~RequestHandler();

	HttpResponse	handle(const HttpRequest& request);
};

#endif