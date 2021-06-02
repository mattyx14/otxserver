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
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "status.h"
#include "const.h"
#include "tools.h"

#include "connection.h"
#include "networkmessage.h"
#include "outputmessage.h"

#include "configmanager.h"
#include "game.h"


extern ConfigManager g_config;
extern Game g_game;

std::map<uint32_t, int64_t> ProtocolStatus::ipConnectMap;
const uint64_t ProtocolStatus::start = OTSYS_TIME();

enum RequestedInfo_t : uint16_t {
	REQUEST_BASIC_SERVER_INFO = 1 << 0,
	REQUEST_OWNER_SERVER_INFO = 1 << 1,
	REQUEST_MISC_SERVER_INFO = 1 << 2,
	REQUEST_PLAYERS_INFO = 1 << 3,
	REQUEST_MAP_INFO = 1 << 4,
	REQUEST_EXT_PLAYERS_INFO = 1 << 5,
	REQUEST_PLAYER_STATUS_INFO = 1 << 6,
	REQUEST_SERVER_SOFTWARE_INFO = 1 << 7,
};

void ProtocolStatus::onRecvFirstMessage(NetworkMessage& msg)
{
	uint32_t ip = getIP();
	if (ip != 0x0100007F) {
		std::map<uint32_t, int64_t>::const_iterator it = ipConnectMap.find(ip);
		if (it != ipConnectMap.end() && (OTSYS_TIME() < (it->second + g_config.getNumber(ConfigManager::STATUSQUERY_TIMEOUT)))) {
			disconnect();
			return;
		}
	}

	ipConnectMap[ip] = OTSYS_TIME();

	uint8_t type = msg.get<char>();
	switch(type)
	{
		case 0xFF:
		{
			if(msg.getString(4) == "info")
			{
				Dispatcher::getInstance().addTask(createTask(std::bind(&ProtocolStatus::sendStatusString,
                                      std::static_pointer_cast<ProtocolStatus>(shared_from_this()))));
				return;
			}

			break;
		}

		case 0x01:
		{
			uint32_t requestedInfo = msg.get<uint16_t>(); // only a byte is necessary, though we could add new infos here
			std::string characterName;
			if (requestedInfo & REQUEST_PLAYER_STATUS_INFO)
				characterName = msg.getString();

			Dispatcher::getInstance().addTask(createTask(std::bind(&ProtocolStatus::sendInfo, std::dynamic_pointer_cast<ProtocolStatus>(shared_from_this()), 
				requestedInfo, characterName)));
			return;
		}

		default:
			break;
	}

	disconnect();
}

void ProtocolStatus::sendStatusString()
{
	auto output = OutputMessagePool::getOutputMessage();

	setRawMessages(true);

	xmlDocPtr doc = xmlNewDoc((const xmlChar*)"1.0");
	doc->children = xmlNewDocNode(doc, NULL, (const xmlChar*)"tsqp", NULL);
	xmlNodePtr root = doc->children;

	char buffer[90];
	xmlSetProp(root, (const xmlChar*)"version", (const xmlChar*)"1.0");

	xmlNodePtr p = xmlNewNode(NULL,(const xmlChar*)"serverinfo");
	sprintf(buffer, "%u", (uint32_t)((OTSYS_TIME() - ProtocolStatus::start) / 1000));
	xmlSetProp(p, (const xmlChar*)"uptime", (const xmlChar*)buffer);
	xmlSetProp(p, (const xmlChar*)"ip", (const xmlChar*)g_config.getString(ConfigManager::IP).c_str());
	xmlSetProp(p, (const xmlChar*)"servername", (const xmlChar*)g_config.getString(ConfigManager::SERVER_NAME).c_str());
	sprintf(buffer, "%d", (int32_t)g_config.getNumber(ConfigManager::LOGIN_PORT));
	xmlSetProp(p, (const xmlChar*)"port", (const xmlChar*)buffer);
	xmlSetProp(p, (const xmlChar*)"location", (const xmlChar*)g_config.getString(ConfigManager::LOCATION).c_str());
	xmlSetProp(p, (const xmlChar*)"url", (const xmlChar*)g_config.getString(ConfigManager::URL).c_str());
	xmlSetProp(p, (const xmlChar*)"server", (const xmlChar*)SOFTWARE_NAME);
	xmlSetProp(p, (const xmlChar*)"version", (const xmlChar*)SOFTWARE_VERSION);
	xmlSetProp(p, (const xmlChar*)"client", (const xmlChar*)CLIENT_VERSION_STRING);
	xmlAddChild(root, p);

	p = xmlNewNode(NULL,(const xmlChar*)"owner");
	xmlSetProp(p, (const xmlChar*)"name", (const xmlChar*)g_config.getString(ConfigManager::OWNER_NAME).c_str());
	xmlSetProp(p, (const xmlChar*)"email", (const xmlChar*)g_config.getString(ConfigManager::OWNER_EMAIL).c_str());
	xmlAddChild(root, p);

	p = xmlNewNode(NULL,(const xmlChar*)"players");
	sprintf(buffer, "%d", g_game.getPlayersWithMcLimit());
	xmlSetProp(p, (const xmlChar*)"online", (const xmlChar*)buffer);

	sprintf(buffer, "%d", (int32_t)g_config.getNumber(ConfigManager::MAX_PLAYERS));
	xmlSetProp(p, (const xmlChar*)"max", (const xmlChar*)buffer);

	sprintf(buffer, "%d", g_game.getPlayersRecord());
	xmlSetProp(p, (const xmlChar*)"peak", (const xmlChar*)buffer);

	sprintf(buffer, "%d", g_game.getUniquePlayersOnline());
	xmlSetProp(p, (const xmlChar*)"unique_players", (const xmlChar*)buffer);

	xmlAddChild(root, p);

	p = xmlNewNode(NULL,(const xmlChar*)"monsters");
	sprintf(buffer, "%d", g_game.getMonstersOnline());
	xmlSetProp(p, (const xmlChar*)"total", (const xmlChar*)buffer);
	xmlAddChild(root, p);

	p = xmlNewNode(NULL,(const xmlChar*)"npcs");
	sprintf(buffer, "%d", g_game.getNpcsOnline());
	xmlSetProp(p, (const xmlChar*)"total", (const xmlChar*)buffer);
	xmlAddChild(root, p);

	p = xmlNewNode(NULL,(const xmlChar*)"map");
	xmlSetProp(p, (const xmlChar*)"name", (const xmlChar*)g_config.getString(ConfigManager::MAP_NAME).c_str());
	xmlSetProp(p, (const xmlChar*)"author", (const xmlChar*)g_config.getString(ConfigManager::MAP_AUTHOR).c_str());

	uint32_t mapWidth, mapHeight;
	g_game.getMapDimensions(mapWidth, mapHeight);
	sprintf(buffer, "%u", mapWidth);
	xmlSetProp(p, (const xmlChar*)"width", (const xmlChar*)buffer);
	sprintf(buffer, "%u", mapHeight);

	xmlSetProp(p, (const xmlChar*)"height", (const xmlChar*)buffer);
	xmlAddChild(root, p);

	xmlNewTextChild(root, NULL, (const xmlChar*)"motd", (const xmlChar*)g_config.getString(ConfigManager::MOTD).c_str());

	xmlChar* s = NULL;
	int32_t len = 0;
	xmlDocDumpMemory(doc, (xmlChar**)&s, &len);

	std::string xml;
	if(s)
		xml = std::string((char*)s, len);

	xmlFree(s);
	xmlFreeDoc(doc);

	output->addBytes(xml.c_str(), xml.size());
	send(output);
	disconnect();
}

void ProtocolStatus::sendInfo(uint16_t requestedInfo, const std::string& characterName)
{
	auto output = OutputMessagePool::getOutputMessage();

	if(requestedInfo & REQUEST_BASIC_SERVER_INFO)
	{
		output->addByte(0x10);
		output->addString(g_config.getString(ConfigManager::SERVER_NAME).c_str());
		output->addString(g_config.getString(ConfigManager::IP).c_str());

		char buffer[10];
		sprintf(buffer, "%d", (int32_t)g_config.getNumber(ConfigManager::LOGIN_PORT));
		output->addString(buffer);
	}

	if (requestedInfo & REQUEST_OWNER_SERVER_INFO)
	{
		output->addByte(0x11);
		output->addString(g_config.getString(ConfigManager::OWNER_NAME).c_str());
		output->addString(g_config.getString(ConfigManager::OWNER_EMAIL).c_str());
	}

	if(requestedInfo & REQUEST_MISC_SERVER_INFO)
	{
		output->addByte(0x12);
		output->addString(g_config.getString(ConfigManager::MOTD).c_str());
		output->addString(g_config.getString(ConfigManager::LOCATION).c_str());
		output->addString(g_config.getString(ConfigManager::URL).c_str());

		output->add<uint64_t>((OTSYS_TIME() - ProtocolStatus::start) / 1000);
	}

	if(requestedInfo & REQUEST_PLAYERS_INFO)
	{
		output->addByte(0x20);
		output->add<uint32_t>(g_game.getPlayersOnline());
		output->add<uint32_t>((uint32_t)g_config.getNumber(ConfigManager::MAX_PLAYERS));
		output->add<uint32_t>(g_game.getPlayersRecord());
	}

	if(requestedInfo & REQUEST_MAP_INFO)
	{
		output->addByte(0x30);
		output->addString(g_config.getString(ConfigManager::MAP_NAME).c_str());
		output->addString(g_config.getString(ConfigManager::MAP_AUTHOR).c_str());

		uint32_t mapWidth, mapHeight;
		g_game.getMapDimensions(mapWidth, mapHeight);
		output->add<uint16_t>(mapWidth);
		output->add<uint16_t>(mapHeight);
	}

	if(requestedInfo & REQUEST_EXT_PLAYERS_INFO)
	{
		output->addByte(0x21);
		std::list<std::pair<std::string, uint32_t> > players;
		for(AutoList<Player>::iterator it = Player::autoList.begin(); it != Player::autoList.end(); ++it)
		{
			if(!it->second->isRemoved() && !it->second->isGhost())
				players.push_back(std::make_pair(it->second->getName(), it->second->getLevel()));
		}

		output->add<uint32_t>(players.size());
		for(std::list<std::pair<std::string, uint32_t> >::iterator it = players.begin(); it != players.end(); ++it)
		{
			output->addString(it->first);
			output->add<uint32_t>(it->second);
		}
	}

	if(requestedInfo & REQUEST_PLAYER_STATUS_INFO)
	{
		output->addByte(0x22);

		Player* p = NULL;
		if(g_game.getPlayerByNameWildcard(characterName, p) == RET_NOERROR && !p->isGhost())
			output->addByte(0x01);
		else
			output->addByte(0x00);
	}

	if(requestedInfo & REQUEST_SERVER_SOFTWARE_INFO)
	{
		output->addByte(0x23);
		output->addString(SOFTWARE_NAME);
		output->addString(SOFTWARE_VERSION);
		output->addString(CLIENT_VERSION_STRING);
	}

	send(output);
	disconnect();
}
