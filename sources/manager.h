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
#ifndef __MANAGER__
#define __MANAGER__
#include "otsystem.h"
#include "const.h"

#include "textlogger.h"
#include "protocol.h"
#include "luascript.h"

enum
{
	MP_MSG_LOGIN = 1,
	MP_MSG_LOGOUT = 2,
	MP_MSG_KEEP_ALIVE = 3,
	MP_MSG_PING = 4,
	MP_MSG_LUA = 5,
	MP_MSG_USER_INFO = 6,
	MP_MSG_CHAT_REQUEST = 7,
	MP_MSG_CHAT_OPEN = 8,
	MP_MSG_CHAT_CLOSE = 9,
	MP_MSG_CHAT_TALK = 10,

	MP_MSG_ERROR = 1,
	MP_MSG_SUCCESS = 2,
	MP_MSG_FAILURE = 3,
	MP_MSG_HELLO = 4,
	MP_MSG_BYE = 5,
	MP_MSG_PONG = 6,
	MP_MSG_OUTPUT = 7,
	MP_MSG_USERS = 8,
	MP_MSG_USER_ADD = 9,
	MP_MSG_USER_REMOVE = 10,
	MP_MSG_USER_DATA = 11,
	MP_MSG_CHAT_LIST = 12,
	MP_MSG_CHAT_USERS = 13,
	MP_MSG_CHAT_USER_ADD = 14,
	MP_MSG_CHAT_USER_REMOVE = 15,
	MP_MSG_CHAT_MESSAGE = 16
};

enum LuaReturn_t
{
	LUA_RESERVE = -1,
	LUA_FALSE = 0,
	LUA_TRUE = 1
};

class ProtocolManager; // TODO
class NetworkMessage;
class Player;

class Manager
{
	public:
		virtual ~Manager() {m_clients.clear();}
		static Manager* getInstance()
		{
			static Manager instance;
			return &instance;
		}

		bool addConnection(ProtocolManager* client);
		bool loginConnection(ProtocolManager* client);
		void removeConnection(ProtocolManager* client);

		bool allow(uint32_t ip) const;
		LuaReturn_t execute(const std::string& script);

		void output(const std::string& message);
		void addUser(Player* player);
		void removeUser(uint32_t playerId);

		void talk(uint32_t playerId, uint16_t channelId, MessageClasses type, const std::string& message);
		void addUser(uint32_t playerId, uint16_t channelId);
		void removeUser(uint32_t playerId, uint16_t channelId);

	protected:
		Manager() {}
		LuaInterface* m_interface;

		typedef std::map<ProtocolManager*, bool> ClientMap;
		ClientMap m_clients;
};


class ProtocolManager : public Protocol
{
	public:
#ifdef __ENABLE_SERVER_DIAGNOSTIC__
		static uint32_t protocolManagerCount;
#endif
		virtual void onRecvFirstMessage(NetworkMessage& msg);

		ProtocolManager(Connection_ptr connection): Protocol(connection)
		{
			m_state = NO_CONNECTED;
			m_loginTries = m_lastCommand = m_channels = 0;
			m_startTime = time(NULL);
#ifdef __ENABLE_SERVER_DIAGNOSTIC__
			protocolManagerCount++;
#endif
		}
		virtual ~ProtocolManager()
		{
#ifdef __ENABLE_SERVER_DIAGNOSTIC__
			protocolManagerCount--;
#endif
		}

                enum {server_sends_first = false};
                enum {protocol_identifier = 0xFD};
                enum {use_checksum = false};
                static const char* protocol_name() {
                        return "manager protocol";
                }

		bool checkChannel(uint16_t channelId) const {return ((m_channels & (uint32_t)channelId) == (uint32_t)channelId);}

		void output(const std::string& message);
		void addUser(Player* player);
		void removeUser(uint32_t playerId);

		void talk(uint32_t playerId, uint16_t channelId, MessageClasses type, const std::string& message);
		void addUser(uint32_t playerId, uint16_t channelId);
		void removeUser(uint32_t playerId, uint16_t channelId);

	protected:
		enum ProtocolState_t
		{
			NO_CONNECTED,
			NO_LOGGED_IN,
			LOGGED_IN,
		};

		virtual void parsePacket(NetworkMessage& msg);
		virtual void release();

		void pong();
		void execute(std::string lua);
		void user(uint32_t playerId);

		void channels();
		void chat(std::string name, uint16_t channelId, MessageClasses type, std::string message);
		void channel(uint16_t channelId, bool opening);

	private:
		void addLogLine(LogType_t type, std::string message);

		int32_t m_loginTries;
		ProtocolState_t m_state;
		uint32_t m_lastCommand, m_startTime, m_channels;
};
#endif