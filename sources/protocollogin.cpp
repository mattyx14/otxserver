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
#include <iomanip>

#include "protocollogin.h"
#include "tools.h"
#include "const.h"

#include "iologindata.h"
#include "ioban.h"

#include "outputmessage.h"
#include "connection.h"
#ifdef __LOGIN_SERVER__
#include "gameservers.h"
#endif

#include "configmanager.h"
#include "game.h"


extern ConfigManager g_config;
extern Game g_game;

extern std::list<std::pair<uint32_t, uint32_t> > serverIps;

#ifdef __ENABLE_SERVER_DIAGNOSTIC__
uint32_t ProtocolLogin::protocolLoginCount = 0;

#endif
#ifdef __DEBUG_NET_DETAIL__
void ProtocolLogin::deleteProtocolTask()
{
	std::clog << "Deleting ProtocolLogin" << std::endl;
	Protocol::deleteProtocolTask();
}

#endif
void ProtocolLogin::disconnectClient(uint8_t error, const char* message)
{
	OutputMessage_ptr output = OutputMessagePool::getOutputMessage();
	output->addByte(error);
	output->addString(message);
	send(output);
	disconnect();
}

void ProtocolLogin::onRecvFirstMessage(NetworkMessage& msg)
{
	if(g_game.getGameState() == GAMESTATE_SHUTDOWN)
	{
		getConnection()->close();
		return;
	}

	uint32_t clientIp = getConnection()->getIP();
	msg.skipBytes(2); // client platform
	uint16_t version = msg.get<uint16_t>();

#ifdef CLIENT_VERSION_DATA
	uint32_t datSignature = msg.get<uint32_t>();
	uint32_t sprSignature = msg.get<uint32_t>();

	uint32_t picSignature = msg.get<uint32_t>();
#else
	msg.skipBytes(12);
#endif
	if(!RSA_decrypt(msg))
	{
		getConnection()->close();
		return;
	}

	uint32_t key[4] = {msg.get<uint32_t>(), msg.get<uint32_t>(), msg.get<uint32_t>(), msg.get<uint32_t>()};
	enableXTEAEncryption();
	setXTEAKey(key);

	std::string name = msg.getString(), password = msg.getString();
	if(name.empty())
	{
		name = "10";
	}

	if(!g_config.getBool(ConfigManager::MANUAL_ADVANCED_CONFIG))
	{
		if(version < g_config.getNumber(ConfigManager::VERSION_MIN) || version > g_config.getNumber(ConfigManager::VERSION_MAX))
		{
			disconnectClient(0x14, g_config.getString(ConfigManager::VERSION_MSG).c_str());
			return;
		}
	}
	else
	{
		if(version < CLIENT_VERSION_MIN || version > CLIENT_VERSION_MAX)
		{
			disconnectClient(0x14, "Only clients with protocol " CLIENT_VERSION_STRING " allowed!");
			return;
		}
	}

#ifdef CLIENT_VERSION_DATA
	if(sprSignature != CLIENT_VERSION_SPR)
	{
		disconnectClient(0x0A, CLIENT_VERSION_DATA);
		return;
	}

	if(datSignature != CLIENT_VERSION_DAT)
	{
		disconnectClient(0x0A, CLIENT_VERSION_DATA);
		return;
	}

	if(picSignature != CLIENT_VERSION_PIC)
	{
		disconnectClient(0x0A, CLIENT_VERSION_DATA);
		return;
	}
#endif

	if(g_game.getGameState() < GAMESTATE_NORMAL)
	{
		disconnectClient(0x0A, "Server is just starting up, please wait.");
		return;
	}

	if(g_game.getGameState() == GAMESTATE_MAINTAIN)
	{
		disconnectClient(0x0A, "Server is under maintenance, please re-connect in a while.");
		return;
	}

	if(IOBan::getInstance()->isIpBanished(clientIp))
	{
		disconnectClient(0x0A, "Your IP is banished!");
		return;
	}

	Account account;
	if(!IOLoginData::getInstance()->loadAccount(account, name) || (account.name != "10" && !encryptTest(account.salt + password, account.password)))
	{
		disconnectClient(0x0A, "Invalid account name or password.");
		return;
	}

	Ban ban;
	ban.value = account.number;

	ban.type = BAN_ACCOUNT;
	if(IOBan::getInstance()->getData(ban) && !IOLoginData::getInstance()->hasFlag(account.number, PlayerFlag_CannotBeBanned))
	{
		bool deletion = ban.expires < 0;
		std::string name_ = "Automatic ";
		if(!ban.adminId)
			name_ += (deletion ? "deletion" : "banishment");
		else
			IOLoginData::getInstance()->getNameByGuid(ban.adminId, name_, true);

		std::ostringstream ss;
		ss << "Your account has been " << (deletion ? "deleted" : "banished") << " at:\n" << formatDateEx(ban.added, "%d %b %Y").c_str()
			<< " by: " << name_.c_str() << ".\nThe comment given was:\n" << ban.comment.c_str() << ".\nYour " << (deletion ?
			"account won't be undeleted" : "banishment will be lifted at:\n") << (deletion ? "" : formatDateEx(ban.expires).c_str()) << ".";

		disconnectClient(0x0A, ss.str().c_str());
		return;
	}

	// remove premium days
	#ifndef __LOGIN_SERVER__
	IOLoginData::getInstance()->removePremium(account);
	if(account.name != "10" && !g_config.getBool(ConfigManager::ACCOUNT_MANAGER) && !account.charList.size())
	{
		disconnectClient(0x0A, std::string("This account does not contain any character yet.\nCreate a new character on the "
			+ g_config.getString(ConfigManager::SERVER_NAME) + " website at " + g_config.getString(ConfigManager::URL) + ".").c_str());
		return;
	}
	#else
	Characters charList;
	for(Characters::iterator it = account.charList.begin(); it != account.charList.end(); ++it)
	{
		if(version >= it->second.server->getVersionMin() && version <= it->second.server->getVersionMax())
			charList[it->first] = it->second;
	}

	IOLoginData::getInstance()->removePremium(account);
	if(account.name != "10" && !g_config.getBool(ConfigManager::ACCOUNT_MANAGER) && !charList.size())
	{
		disconnectClient(0x0A, std::string("This account does not contain any character on this client yet.\nCreate a new character on the "
			+ g_config.getString(ConfigManager::SERVER_NAME) + " website at " + g_config.getString(ConfigManager::URL) + ".").c_str());
		return;
	}
	#endif

	OutputMessage_ptr output = OutputMessagePool::getOutputMessage();

	output->addByte(0x14);
	uint32_t serverIp = serverIps.front().first;
	for(std::list<std::pair<uint32_t, uint32_t> >::iterator it = serverIps.begin(); it != serverIps.end(); ++it)
	{
		if((it->first & it->second) != (clientIp & it->second))
			continue;

		serverIp = it->first;
		break;
	}

	char motd[1300];
	sprintf(motd, "%d\n%s", g_game.getMotdId(), g_config.getString(ConfigManager::MOTD).c_str());
	output->addString(motd);

	//Add char list
	output->addByte(0x64);
	if (account.name == "10" && account.name != "0")
	{
		PlayerVector players;
		for (AutoList<Player>::iterator it = Player::autoList.begin(); it != Player::autoList.end(); ++it)
		{
			if (!it->second->isRemoved() && it->second->client->isBroadcasting())
				players.push_back(it->second);
		}
		if (!players.size())
			disconnectClient(0x0A, "There are no livestreams online right now.");
		else
		{
			std::sort(players.begin(), players.end(), Player::sort);
			output->addByte(players.size());
			for (PlayerVector::iterator it = players.begin(); it != players.end(); ++it)
			{
				std::ostringstream s;
				s << (*it)->getLevel();
				if (!(*it)->client->check(password))
					s << "*";

				output->addString((*it)->getName());
				output->addString(s.str());
				output->add<uint32_t>(serverIp);

				IntegerVec games = vectorAtoi(explodeString(g_config.getString(ConfigManager::GAME_PORT), ","));
				output->add<uint16_t>(games[random_range(0, games.size() - 1)]);
			}
		}
	}
	else
	{
		if(g_config.getBool(ConfigManager::ACCOUNT_MANAGER) && account.number != 1)
		{
			output->addByte(account.charList.size() + 1);
			output->addString("Account Manager");

			output->addString(g_config.getString(ConfigManager::SERVER_NAME));
			output->add<uint32_t>(serverIp);

			IntegerVec games = vectorAtoi(explodeString(g_config.getString(ConfigManager::GAME_PORT), ","));
			output->add<uint16_t>(games[random_range(0, games.size() - 1)]);
		}
		else
			output->addByte((uint8_t)account.charList.size());

		#ifndef __LOGIN_SERVER__
		for(Characters::iterator it = account.charList.begin(); it != account.charList.end(); ++it)
		{
			output->addString((*it));
			if(g_config.getBool(ConfigManager::ON_OR_OFF_CHARLIST))
			{
				if(g_game.getPlayerByName((*it)))
					output->addString("Online");
				else
					output->addString("Offline");
			}
			else
				output->addString(g_config.getString(ConfigManager::SERVER_NAME));

			output->add<uint32_t>(serverIp);
			IntegerVec games = vectorAtoi(explodeString(g_config.getString(ConfigManager::GAME_PORT), ","));
			output->add<uint16_t>(games[random_range(0, games.size() - 1)]);
		}
		#else
		for(Characters::iterator it = charList.begin(); it != charList.end(); ++it)
		{
			output->addString(it->second.name);
			if(!g_config.getBool(ConfigManager::ON_OR_OFF_CHARLIST) || it->second.status < 0)
				output->addString(it->second.server->getName());
			else if(it->second.status)
				output->addString("Online");
			else
				output->addString("Offline");

			output->add<uint32_t>(it->second.server->getAddress());
			IntegerVec games = it->second.server->getPorts();
			output->add<uint16_t>(games[random_range(0, games.size() - 1)]);
		}
		#endif
	}

	//Add premium days
	if(g_config.getBool(ConfigManager::FREE_PREMIUM))
		output->add<uint16_t>(GRATIS_PREMIUM);
	else
		output->add<uint16_t>(account.premiumDays);

	send(output);
	disconnect();
}
