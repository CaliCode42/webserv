/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 23:53:01 by sdossa            #+#    #+#             */
/*   Updated: 2026/09/02 15:43:42 by tcali            ###   ########.fr       */
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
	~HttpResponse();

	void setStatus(int code);
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