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

#ifndef __ADMIN__
#define __ADMIN__
#include "otsystem.h"
#ifdef __OTADMIN__

#include "protocol.h"
#include "textlogger.h"

// -> server
// command(1 byte) | size(2 bytes) | parameters(size bytes)
// commands:
//	login
//		password(string)
//  encryption
//		encryption type(1 byte)
//			RSA1024+XTEA
//				:128 bytes encrypted using 1024 bytes public key
//				16 bytes XTEA key
//  key-exchange
//		public_key_type(1 byte)
//			RSA1024+XTEA
//  command
//		command + paramters(string)
//	no_operation/ping
//		nothing
//
// <- server
// ret-code(1 byte)| size(2 bytes) | parameters(size bytes)
// ret-codes:
//	hello
//		server_version(4 bytes)
//		server_string(string)
//		security_policy(2 bytes flags)
//			required_login
//			required_encryption
//		accepted_encryptions(4 bytes flags)
//			RSA1024+XTEA
//	key-exchange-ok
//		public_key_type(1 byte)
//			RSA1024+XTEA
//				:128 bytes public key modulus
//	key-exchange-failed
//		reason(string)
//  login-ok
//		nothing
//  login-failed
//		reason(string)
//  command-ok
//		command result(string)
//  command-failed
//		reason(string)
//  encryption-ok
//		nothing
//  encryption-failed
//		reason(string)
//	no_operation-ok
//		nothing
//	message
//		message(string)
//  error
//		message(string)
//

enum
{
	AP_MSG_LOGIN = 1,
	AP_MSG_ENCRYPTION = 2,
	AP_MSG_KEY_EXCHANGE = 3,
	AP_MSG_COMMAND = 4,
	AP_MSG_PING = 5,
	AP_MSG_KEEP_ALIVE = 6,

	AP_MSG_HELLO = 1,
	AP_MSG_KEY_EXCHANGE_OK = 2,
	AP_MSG_KEY_EXCHANGE_FAILED = 3,
	AP_MSG_LOGIN_OK = 4,
	AP_MSG_LOGIN_FAILED = 5,
	AP_MSG_COMMAND_OK = 6,
	AP_MSG_COMMAND_FAILED = 7,
	AP_MSG_ENCRYPTION_OK = 8,
	AP_MSG_ENCRYPTION_FAILED = 9,
	AP_MSG_PING_OK = 10,
	AP_MSG_MESSAGE = 11,
	AP_MSG_ERROR = 12,
};

enum
{
	CMD_BROADCAST = 1,
	CMD_CLOSE_SERVER = 2,
	CMD_PAY_HOUSES = 3,
	CMD_OPEN_SERVER = 4,
	CMD_SHUTDOWN_SERVER = 5,
	CMD_RELOAD_SCRIPTS = 6,
	CMD_KICK = 9,
	CMD_SAVE_SERVER = 13,
	CMD_SEND_MAIL = 14,
	CMD_SHALLOW_SAVE_SERVER = 15
};

enum
{
	REQUIRE_LOGIN = 1,
	REQUIRE_ENCRYPTION = 2
};

enum
{
	ENCRYPTION_RSA1024XTEA = 1
};

class NetworkMessage;
class Player;
class RSA;

class Admin
{
	public:
		virtual ~Admin();
		static Admin* getInstance()
		{
			static Admin instance;
			return &instance;
		}

		bool addConnection();
		void removeConnection();

		uint16_t getPolicy() const;
		uint32_t getOptions() const;

		static Item* createMail(const std::string xmlData, std::string& name, uint32_t& depotId);
		bool allow(uint32_t ip) const;

		bool isEncypted() const {return m_encrypted;}
		RSA* getRSAKey(uint8_t type);

	protected:
		Admin();

		int32_t m_currentConnections;
		bool m_encrypted;

		RSA* m_key_RSA1024XTEA;
};

class ProtocolAdmin : public Protocol
{
	public:
#ifdef __ENABLE_SERVER_DIAGNOSTIC__
		static uint32_t protocolAdminCount;
#endif
		virtual void onRecvFirstMessage(NetworkMessage& msg);

		ProtocolAdmin(Connection_ptr connection): Protocol(connection)
		{
			m_state = NO_CONNECTED;
			m_loginTries = m_lastCommand = 0;
			m_startTime = time(NULL);
#ifdef __ENABLE_SERVER_DIAGNOSTIC__
			protocolAdminCount++;
#endif
		}
		virtual ~ProtocolAdmin()
		{
#ifdef __ENABLE_SERVER_DIAGNOSTIC__
			protocolAdminCount--;
#endif
		}

		enum {protocolId = 0xFE};
		enum {isSingleSocket = false};
		enum {hasChecksum = false};
		static const char* protocolName() {return "admin protocol";}

	protected:
		enum ProtocolState_t
		{
			NO_CONNECTED,
			ENCRYPTION_NO_SET,
			ENCRYPTION_OK,
			NO_LOGGED_IN,
			LOGGED_IN
		};

		virtual void parsePacket(NetworkMessage& msg);
		virtual void releaseProtocol();
#ifdef __DEBUG_NET_DETAIL__
		virtual void deleteProtocolTask();
#endif

		// commands
		void adminCommandPayHouses();
		void adminCommandReload(int8_t reload);

		void adminCommandKickPlayer(const std::string& name);
		void adminCommandSendMail(const std::string& xmlData);

	private:
		void addLogLine(LogType_t type, std::string message);

		int32_t m_loginTries;
		ProtocolState_t m_state;
		uint32_t m_lastCommand, m_startTime;
};
#endif
#endif
