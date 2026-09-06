/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionManager.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdossa <sdossa@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 16:37:14 by sdossa            #+#    #+#             */
/*   Updated: 2026/09/06 23:00:49 by sdossa           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SessionManager.hpp"
#include <cstdlib>
#include <sstream>

SessionManager::SessionManager(long ttlSeconds) : _ttl(ttlSeconds)
{}

SessionManager::~SessionManager()
{}

std::string SessionManager::create()
{
	std::string sid = generateId();
	while (_sessions.find(sid) != _sessions.end()) //avoid collisions
		sid = generateId();
	_sessions[sid].lastSeen = std::time(0);
	return sid;
}

bool SessionManager::exists(const std::string& sid)
{
	if (sid.empty())
		return false;
	std::map<std::string, Session>::iterator sessionIt = _sessions.find(sid);
	if (sessionIt == _sessions.end())
		return false;

	std::time_t now = std::time(0);
	if (now - sessionIt->second.lastSeen > _ttl)
	{
		_sessions.erase(sessionIt);
		return false;
	}
	sessionIt->second.lastSeen = now; //sliding expiration
	return true;
}

void SessionManager::set(const std::string& sid, const std::string& key,
						const std::string& value)
{
	std::map<std::string, Session>::iterator sessionIt = _sessions.find(sid);
	if (sessionIt != _sessions.end())
		sessionIt->second.data[key] = value;
}

std::string SessionManager::get(const std::string& sid, const std::string& key)
{
	std::map<std::string, Session>::iterator sessionIt = _sessions.find(sid);
	if (sessionIt == _sessions.end())
		return "";
	std::map<std::string, std::string>::iterator dataIt = sessionIt->second.data.find(key);
	if (dataIt == sessionIt->second.data.end())
		return "";
	return dataIt->second;
}

std::string SessionManager::generateId()
{
	static const char* hex = "0123456789abcdef";
	std::string id;
	id.reserve(32);
	for (int i = 0; i < 32; i++)
		id += hex[std::rand() % 16];
	return id;
}

void SessionManager::destroy(const std::string& sid)
{
	_sessions.erase(sid);
}