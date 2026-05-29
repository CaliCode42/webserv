/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:42:55 by tcali             #+#    #+#             */
/*   Updated: 2026/05/29 18:43:42 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

HttpResponse::HttpResponse() : _statusCode(0)
{
	std::cout << "[HttpResponse] Default constructor called" << std::endl;
}

// HttpResponse::HttpResponse(const HttpResponse& other)
// {
// 	std::cout << "[HttpResponse] Copy constructor called" << std::endl;
// 	*this = other;
// }

// HttpResponse& HttpResponse::operator=(const HttpResponse& other)
// {
// 	if (this != &other)
// 	{
// 		// copy attributes here
// 	}
// 	std::cout << "[HttpResponse] Copy assignment operator called" << std::endl;
// 	return (*this);
// }

HttpResponse::~HttpResponse()
{
	std::cout << "[HttpResponse] Destructor called" << std::endl;
}
