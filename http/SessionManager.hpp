/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionManager.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdossa <sdossa@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 16:37:09 by sdossa            #+#    #+#             */
/*   Updated: 2026/08/12 20:18:08 by sdossa           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SESSIONMANAGER_HPP
#define SESSIONMANAGER_HPP

#include <string>
#include <map>
#include <ctime>

class SessionManager 
{
public:
	explicit SessionManager(long ttlSeconds = 3600);

	std::string create();// create sesion, returns id for future cookie value
	
	bool exists(const std::string& sid);//expires stale sessions

	//Pre-session key/value store
	void		set(const std::string& sid, const std::string& key,
				const std::string& value);
	std::string get(const std::string& sid, const std::string& key);
	
	void destroy(const std::string& sid);

private:
	struct Session
	{
		std::map<std::string, std::string>	data;
		std::time_t							lastSeen;
	};
	
	std::map<std::string, Session>	_sessions;
	long							_ttl; //time before expiration
	
	static std::string generateId();
};


#endif
