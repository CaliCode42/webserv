/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MethodHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdossa <sdossa@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/25 19:51:12 by sdossa            #+#    #+#             */
/*   Updated: 2026/07/28 19:00:56 by sdossa           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef METHODHANDLER_HPP
#define METHODHANDLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "../includes/ServerConfig.hpp"
#include <string>

class MethodHandler 
{
public:
    // point d'entree called by event loop qd 1 req est complete
    MethodHandler(const ServerConfig& config);
    HttpResponse handle(const HttpRequest& req);
    
private:
    const ServerConfig& _config;
    HttpResponse handleGet(const std::string& path);

};

#endif
