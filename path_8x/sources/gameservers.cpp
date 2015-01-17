////////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
////////////////////////////////////////////////////////////////////////
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////
#include "otpch.h"
#include <iostream>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "gameservers.h"
#include "tools.h"

void GameServers::clear()
{
	for(GameServersMap::iterator it = serverList.begin(); it != serverList.end(); ++it)
		delete it->second;

	serverList.clear();
}

bool GameServers::reload()
{
	clear();
	return loadFromXml(false);
}

bool GameServers::loadFromXml(bool result)
{
	xmlDocPtr doc = xmlParseFile(getFilePath(FILE_TYPE_XML,"servers.xml").c_str());
	if(!doc)
	{
		std::clog << "[Warning - GameServers::loadFromXml] Cannot load servers file." << std::endl;
		std::clog << getLastXMLError() << std::endl;
		return false;
	}

	xmlNodePtr root = xmlDocGetRootElement(doc);
	if(xmlStrcmp(root->name,(const xmlChar*)"servers"))
	{
		std::clog << "[Error - GameServers::loadFromXml] Malformed servers file." << std::endl;
		xmlFreeDoc(doc);
		return false;
	}

	std::string strValue;
	int32_t intValue;
	for(xmlNodePtr p = root->children; p; p = p->next)
	{
		if(xmlStrcmp(p->name, (const xmlChar*)"server"))
			continue;

		std::string name, address;
		uint32_t id, versionMin, versionMax;

		IntegerVec ports;
		if(readXMLInteger(p, "id", intValue))
			id = intValue;
		else
		{
			std::clog << "[Error - GameServers::loadFromXml] Missing id, skipping" << std::endl;
			continue;
		}

		if(serverList.find(id) != serverList.end())
		{
			std::clog << "[Error - GameServers::loadFromXml] Duplicate server id " << id << ", skipping" << std::endl;
			continue;
		}

		if(readXMLString(p, "name", strValue))
			name = strValue;
		else
		{
			name = "Server #" + id;
			std::clog << "[Warning - GameServers::loadFromXml] Missing name for server " << id << ", using default" << std::endl;
		}

		if(readXMLInteger(p, "versionMin", intValue))
			versionMin = intValue;
		else
		{
			versionMin = CLIENT_VERSION_MIN;
			std::clog << "[Warning - GameServers::loadFromXml] Missing versionMin for server " << id << ", using default" << std::endl;
		}

		if(readXMLInteger(p, "versionMax", intValue))
			versionMax = intValue;
		else
		{
			versionMax = CLIENT_VERSION_MAX;
			std::clog << "[Warning - GameServers::loadFromXml] Missing versionMax for server " << id << ", using default" << std::endl;
		}

		if(readXMLString(p, "address", strValue) || readXMLString(p, "ip", strValue))
			address = strValue;
		else
		{
			address = "localhost";
			std::clog << "[Warning - GameServers::loadFromXml] Missing address for server " << id << ", using default" << std::endl;
		}

		if(readXMLString(p, "port", strValue))
			ports = vectorAtoi(explodeString(strValue, ","));
		else
		{
			ports.push_back(7181);
			std::clog << "[Warning - GameServers::loadFromXml] Missing port for server " << id << ", using default" << std::endl;
		}

		if(GameServer* server = new GameServer(name, versionMin, versionMax, inet_addr(address.c_str()), ports))
			serverList[id] = server;
		else
			std::clog << "[Error - GameServers::loadFromXml] Couldn't add server " << name << std::endl;
	}

	if(result)
	{
		std::clog << "> Servers loaded:" << std::endl;
		for(GameServersMap::iterator it = serverList.begin(); it != serverList.end(); ++it)
		{
			IntegerVec games = it->second->getPorts();
			for(IntegerVec::const_iterator tit = games.begin(); tit != games.end(); ++tit)
				std::clog << it->second->getName() << " (" << it->second->getAddress() << ":" << *tit << ")" << std::endl;
		}
	}

	xmlFreeDoc(doc);
	return true;
}

GameServer* GameServers::getServerById(uint32_t id) const
{
	GameServersMap::const_iterator it = serverList.find(id);
	if(it != serverList.end())
		return it->second;

	return NULL;
}
