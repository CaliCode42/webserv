/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdossa <sdossa@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 23:53:01 by sdossa            #+#    #+#             */
/*   Updated: 2026/07/28 19:05:20 by sdossa           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <vector>
#include <utility>

class HttpResponse
{
public:
    HttpResponse();

    void setStatus(int code);  // 201 ^^
    void setBody(const std::string& body, const std::string& contentType);
    void setHeader(const std::string& key, const std::string&  value);

    std::string serialize() const;
    
    static std::string reasonPhrase(int code);

private:
    int _status;
    std::vector<std::pair<std::string, std::string> > _headers; // "> >"

    std::string _body;
};


#endif