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

#include "manager.h"
#include "player.h"
#include "tools.h"

#include "configmanager.h"
#include "game.h"
#include "chat.h"

#include "connection.h"
#include "outputmessage.h"
#include "networkmessage.h"

extern ConfigManager g_config;
extern Game g_game;
extern Chat g_chat;

#ifdef __ENABLE_SERVER_DIAGNOSTIC__
uint32_t ProtocolManager::protocolManagerCount = 0;
#endif

void ProtocolManager::onRecvFirstMessage(NetworkMessage&)
{
	m_state = NO_CONNECTED;
	if(g_config.getString(ConfigManager::MANAGER_PASSWORD).empty())
	{
		addLogLine(LOGTYPE_WARNING, "Connection attempt on disabled protocol");
		getConnection()->close();
		return;
	}

	if(!Manager::getInstance()->allow(getIP()))
	{
		addLogLine(LOGTYPE_WARNING, "IP not allowed");
		getConnection()->close();
		return;
	}

	if(!Manager::getInstance()->addConnection(this))
	{
		addLogLine(LOGTYPE_WARNING, "Cannot add more connections");
		getConnection()->close();
		return;
	}

	if(OutputMessage_ptr msg = getOutputBuffer())
	{
		TRACK_MESSAGE(msg);
		msg->addByte(MP_MSG_HELLO);

		msg->add<uint32_t>(1); //version
		msg->addString("TFMANAGER");
	}

	m_lastCommand = time(NULL);
	m_state = NO_LOGGED_IN;
}

void ProtocolManager::parsePacket(NetworkMessage& msg)
{
	if(g_game.getGameState() == GAMESTATE_SHUTDOWN)
	{
		getConnection()->close();
		return;
	}

	uint8_t recvbyte = msg.get<char>();
	OutputMessage_ptr output = getOutputBuffer();
	if(!output)
		return;

	TRACK_MESSAGE(output);
	switch(m_state)
	{
		case NO_LOGGED_IN:
		{
			if((time(NULL) - m_startTime) > 30000)
			{
				//login timeout
				getConnection()->close();
				addLogLine(LOGTYPE_WARNING, "Login timeout");
				return;
			}

			if(m_loginTries > 3)
			{
				output->addByte(MP_MSG_ERROR);
				output->addString("Too many login attempts");

				getConnection()->close();
				addLogLine(LOGTYPE_WARNING, "Too many login attempts");
				return;
			}

			if(recvbyte != MP_MSG_LOGIN)
			{
				output->addByte(MP_MSG_ERROR);
				output->addString("You are not logged in");

				getConnection()->close();
				addLogLine(LOGTYPE_WARNING, "Wrong command while not logged in");
				return;
			}
			break;
		}

		case LOGGED_IN:
			break;

		default:
		{
			getConnection()->close();
			addLogLine(LOGTYPE_ERROR, "No valid connection state");
			return;
		}
	}

	m_lastCommand = time(NULL);
	switch(recvbyte)
	{
		case MP_MSG_LOGIN:
		{
			if(m_state == NO_LOGGED_IN)
			{
				std::string pass = msg.getString(), word = g_config.getString(ConfigManager::MANAGER_PASSWORD);
				_encrypt(word, false);
				if(pass == word)
				{
					if(!Manager::getInstance()->loginConnection(this))
					{
						output->addByte(MP_MSG_FAILURE);
						output->addString("Unknown connection");

						getConnection()->close();
						addLogLine(LOGTYPE_ERROR, "Login failed due to unknown connection");
						return;
					}
					else
					{
						output->addByte(MP_MSG_USERS);
						addLogLine(LOGTYPE_EVENT, "Logged in, sending users");

						std::map<uint32_t, std::string> users;
						for(AutoList<Player>::iterator it = Player::autoList.begin(); it != Player::autoList.end(); ++it)
						{
							if(!it->second->isRemoved())
								users[it->first] = it->second->getName();
						}

						output->add<uint16_t>(users.size());
						for(std::map<uint32_t, std::string>::iterator it = users.begin(); it != users.end(); ++it)
						{
							output->add<uint32_t>(it->first);
							output->addString(it->second);
						}

						send(output);
						m_state = LOGGED_IN;
					}
				}
				else
				{
					output->addByte(MP_MSG_ERROR);
					output->addString("Wrong password");

					m_loginTries++;
					addLogLine(LOGTYPE_EVENT, "Login failed due to wrong password (" + pass + ")");
				}
			}

			break;
		}

		case MP_MSG_LOGOUT:
		{
			output->addByte(MP_MSG_BYE);
			output->addString("Bye, bye!");

			getConnection()->close();
			addLogLine(LOGTYPE_EVENT, "Logout");
			return;
		}

		case MP_MSG_KEEP_ALIVE:
			break;

		case MP_MSG_PING:
			Dispatcher::getInstance().addTask(createTask(boost::bind(&ProtocolManager::pong, this)));
			break;

		case MP_MSG_LUA:
			Dispatcher::getInstance().addTask(createTask(boost::bind(&ProtocolManager::execute, this, msg.getString())));
			break;

		case MP_MSG_USER_INFO:
			Dispatcher::getInstance().addTask(createTask(boost::bind(&ProtocolManager::user, this, msg.get<uint32_t>())));
			break;

		case MP_MSG_CHAT_REQUEST:
			Dispatcher::getInstance().addTask(createTask(boost::bind(&ProtocolManager::channels, this)));
			break;

		case MP_MSG_CHAT_OPEN:
			Dispatcher::getInstance().addTask(createTask(boost::bind(&ProtocolManager::channel, this, msg.get<uint16_t>(), true)));
			break;

		case MP_MSG_CHAT_CLOSE:
			Dispatcher::getInstance().addTask(createTask(boost::bind(&ProtocolManager::channel, this, msg.get<uint16_t>(), false)));
			break;

		case MP_MSG_CHAT_TALK:
			Dispatcher::getInstance().addTask(createTask(boost::bind(&ProtocolManager::chat, this, msg.getString(), msg.get<uint16_t>(), (MessageClasses)msg.get<char>(), msg.getString())));
			break;

		default:
			break;
	}
}

void ProtocolManager::release()
{
	addLogLine(LOGTYPE_EVENT, "Closing protocol");
	Manager::getInstance()->removeConnection(this);
	Protocol::release();
}

void ProtocolManager::pong()
{
	if(m_state != LOGGED_IN)
		return;

	OutputMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	msg->addByte(MP_MSG_PONG);
}

void ProtocolManager::execute(std::string lua)
{
	if(m_state != LOGGED_IN)
		return;

	OutputMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	switch(Manager::getInstance()->execute(lua))
	{
		case LUA_TRUE:
		{
			msg->addByte(MP_MSG_SUCCESS);
			addLogLine(LOGTYPE_EVENT, "Executed Lua script");
			break;
		}
		case LUA_RESERVE:
		{
			msg->addByte(MP_MSG_FAILURE);
			msg->addString("Unable to reserve enviroment for Lua script");
			addLogLine(LOGTYPE_ERROR, "Unable to reserve enviroment for Lua script");
			break;
		}
		default:
		{
			msg->addByte(MP_MSG_FAILURE);
			msg->addString("An error occured while executing Lua script, please check Server Log");
			addLogLine(LOGTYPE_ERROR, "An error occured while executing Lua script");
			break;
		}
	}
}

void ProtocolManager::user(uint32_t playerId)
{
	if(m_state != LOGGED_IN)
		return;

	OutputMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	if(Player* player = g_game.getPlayerByID(playerId))
	{
		msg->addByte(MP_MSG_USER_DATA);
		msg->add<uint32_t>(playerId);

		msg->add<int32_t>(player->getGroupId());
		msg->add<uint32_t>(player->getVocationId());

		msg->add<uint32_t>(player->getLevel());
		msg->add<uint32_t>(player->getMagicLevel());
		// TODO: continue...
	}
	else
	{
		msg->addByte(MP_MSG_ERROR);
		msg->addString("Player not found");
	}
}

void ProtocolManager::channels()
{
	if(m_state != LOGGED_IN)
		return;

	OutputMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	msg->addByte(MP_MSG_CHAT_LIST);
	ChannelList list = g_chat.getPublicChannels();

	msg->add<uint16_t>(list.size());
	for(ChannelList::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		msg->add<uint16_t>((*it)->getId());
		msg->addString((*it)->getName());

		msg->add<uint16_t>((*it)->getFlags());
		msg->add<uint16_t>((*it)->getUsers().size());
	}
}

void ProtocolManager::chat(std::string name, uint16_t channelId, MessageClasses type, std::string message)
{
	if(m_state != LOGGED_IN)
		return;

	OutputMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	ChatChannel* channel = NULL;
	if((channel = g_chat.getChannelById(channelId)) && g_chat.isPublicChannel(channelId))
	{
		if(!channel->talk(name, type, message))
		{
			msg->addByte(MP_MSG_FAILURE);
			msg->addString("Could not talk to channel");
		}
		else
			msg->addByte(MP_MSG_SUCCESS);
	}
	else
	{
		msg->addByte(MP_MSG_ERROR);
		msg->addString("Invalid channel");
	}
}

void ProtocolManager::channel(uint16_t channelId, bool opening)
{
	if(m_state != LOGGED_IN)
		return;

	OutputMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	if(opening)
	{
		ChatChannel* channel = NULL;
		if((channel = g_chat.getChannelById(channelId)) && g_chat.isPublicChannel(channelId))
		{
			m_channels |= (1 << (uint32_t)channelId);
			msg->addByte(MP_MSG_CHAT_USERS);
			msg->add<uint16_t>(channelId);

			UsersMap users = channel->getUsers();
			msg->add<uint16_t>(users.size());
			for(UsersMap::const_iterator it = users.begin(); it != users.end(); ++it)
				msg->add<uint32_t>(it->first);
		}
		else
		{
			msg->addByte(MP_MSG_ERROR);
			msg->addString("Invalid channel");
		}
	}
	else if(g_chat.getChannelById(channelId) && g_chat.isPublicChannel(channelId))
	{
		m_channels &= ~(1 << (uint32_t)channelId);
		msg->addByte(MP_MSG_SUCCESS);
	}
	else
	{
		msg->addByte(MP_MSG_ERROR);
		msg->addString("Invalid channel");
	}
}

void ProtocolManager::output(const std::string& message)
{
	if(m_state != LOGGED_IN)
		return;

	OutputMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->addByte(MP_MSG_OUTPUT);
	msg->addString(message);
}

void ProtocolManager::addUser(Player* player)
{
	if(m_state != LOGGED_IN)
		return;

	OutputMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->addByte(MP_MSG_USER_ADD);

	msg->add<uint32_t>(player->getID());
	msg->addString(player->getName());
}

void ProtocolManager::removeUser(uint32_t playerId)
{
	if(m_state != LOGGED_IN)
		return;

	OutputMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->addByte(MP_MSG_USER_REMOVE);
	msg->add<uint32_t>(playerId);
}

void ProtocolManager::talk(uint32_t playerId, uint16_t channelId, MessageClasses type, const std::string& message)
{
	if(m_state != LOGGED_IN)
		return;

	OutputMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	if(!(m_channels & (1 << (uint32_t)channelId)))
		return;

	TRACK_MESSAGE(msg);
	msg->addByte(MP_MSG_CHAT_MESSAGE);
	msg->add<uint32_t>(playerId);

	msg->add<uint16_t>(channelId);
	msg->addByte(type);
	msg->addString(message);
}

void ProtocolManager::addUser(uint32_t playerId, uint16_t channelId)
{
	if(m_state != LOGGED_IN)
		return;

	OutputMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->addByte(MP_MSG_CHAT_USER_ADD);

	msg->add<uint32_t>(playerId);
	msg->add<uint16_t>(channelId);
}

void ProtocolManager::removeUser(uint32_t playerId, uint16_t channelId)
{
	if(m_state != LOGGED_IN)
		return;

	OutputMessage_ptr msg = getOutputBuffer();
	if(!msg)
		return;

	TRACK_MESSAGE(msg);
	msg->addByte(MP_MSG_CHAT_USER_REMOVE);

	msg->add<uint32_t>(playerId);
	msg->add<uint16_t>(channelId);
}

bool Manager::addConnection(ProtocolManager* client)
{
	if(g_config.getNumber(ConfigManager::MANAGER_CONNECTIONS_LIMIT) > 0 && m_clients.size()
		>= (size_t)g_config.getNumber(ConfigManager::MANAGER_CONNECTIONS_LIMIT))
		return false;

	m_clients[client] = false;
	return true;
}

bool Manager::loginConnection(ProtocolManager* client)
{
	ClientMap::iterator it = m_clients.find(client);
	if(it == m_clients.end())
		return false;

	it->second = true;
	return true;
}

void Manager::removeConnection(ProtocolManager* client)
{
	ClientMap::iterator it = m_clients.find(client);
	if(it != m_clients.end())
		m_clients.erase(it);
}

bool Manager::allow(uint32_t ip) const
{
	if(!g_config.getBool(ConfigManager::MANAGER_LOCALHOST_ONLY))
		return true;

	if(ip == 0x0100007F) //127.0.0.1
		return true;

	if(g_config.getBool(ConfigManager::MANAGER_LOGS))
		LOG_MESSAGE(LOGTYPE_EVENT, "forbidden connection try", "MANAGER " + convertIPAddress(ip));

	return false;
}

LuaReturn_t Manager::execute(const std::string& script)
{
	if(!m_interface)
	{
		// create upon first execution to save some memory and
		// avoid any startup crashes related to creation on load
		m_interface = new LuaInterface("Manager Interface");
		m_interface->initState();
	}

	if(!m_interface->reserveEnv())
		return LUA_RESERVE;

	LuaReturn_t tmp = (LuaReturn_t)m_interface->loadBuffer(script);
	m_interface->releaseEnv();
	return tmp;
}

void Manager::output(const std::string& message)
{
	for(ClientMap::const_iterator it = m_clients.begin(); it != m_clients.end(); ++it)
	{
		if(it->second)
			it->first->output(message);
	}
}

void Manager::addUser(Player* player)
{
	for(ClientMap::const_iterator it = m_clients.begin(); it != m_clients.end(); ++it)
	{
		if(it->second)
			it->first->addUser(player);
	}
}

void Manager::removeUser(uint32_t playerId)
{
	for(ClientMap::const_iterator it = m_clients.begin(); it != m_clients.end(); ++it)
	{
		if(it->second)
			it->first->removeUser(playerId);
	}
}

void Manager::talk(uint32_t playerId, uint16_t channelId, MessageClasses type, const std::string& message)
{
	for(ClientMap::const_iterator it = m_clients.begin(); it != m_clients.end(); ++it)
	{
		if(it->second && it->first->checkChannel(channelId))
			it->first->talk(playerId, channelId, type, message);
	}
}

void Manager::addUser(uint32_t playerId, uint16_t channelId)
{
	for(ClientMap::const_iterator it = m_clients.begin(); it != m_clients.end(); ++it)
	{
		if(it->second && it->first->checkChannel(channelId))
			it->first->addUser(playerId, channelId);
	}
}

void Manager::removeUser(uint32_t playerId, uint16_t channelId)
{
	for(ClientMap::const_iterator it = m_clients.begin(); it != m_clients.end(); ++it)
	{
		if(it->second && it->first->checkChannel(channelId))
			it->first->removeUser(playerId, channelId);
	}
}

void ProtocolManager::addLogLine(LogType_t type, std::string message)
{
	if(!g_config.getBool(ConfigManager::MANAGER_LOGS))
		return;

	std::string tmp = "MANAGER";
	if(getIP())
		tmp += " " + convertIPAddress(getIP());

	LOG_MESSAGE(type, message, tmp)
}