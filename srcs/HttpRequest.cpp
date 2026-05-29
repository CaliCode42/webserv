/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/29 18:41:34 by tcali             #+#    #+#             */
/*   Updated: 2026/05/29 18:41:51 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

HttpRequest::HttpRequest()
{
	std::cout << "[HttpRequest] Default constructor called" << std::endl;
}

// HttpRequest::HttpRequest(const HttpRequest& other)
// {
// 	std::cout << "[HttpRequest] Copy constructor called" << std::endl;
// 	*this = other;
// }

// HttpRequest& HttpRequest::operator=(const HttpRequest& other)
// {
// 	if (this != &other)
// 	{
// 		// copy attributes here
// 	}
// 	std::cout << "[HttpRequest] Copy assignment operator called" << std::endl;
// 	return (*this);
// }

HttpRequest::~HttpRequest()
{
	std::cout << "[HttpRequest] Destructor called" << std::endl;
}
