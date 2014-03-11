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

#ifndef __STATUS__
#define __STATUS__

#include "otsystem.h"
#include "protocol.h"

enum RequestedInfo_t
{
	REQUEST_BASIC_SERVER_INFO 	= 0x01,
	REQUEST_SERVER_OWNER_INFO	= 0x02,
	REQUEST_MISC_SERVER_INFO	= 0x04,
	REQUEST_PLAYERS_INFO		= 0x08,
	REQUEST_SERVER_MAP_INFO		= 0x10,
	REQUEST_EXT_PLAYERS_INFO	= 0x20,
	REQUEST_PLAYER_STATUS_INFO	= 0x40,
	REQUEST_SERVER_SOFTWARE_INFO	= 0x80
};

typedef std::map<uint32_t, int64_t> IpConnectMap;
class ProtocolStatus : public Protocol
{
	public:
#ifdef __ENABLE_SERVER_DIAGNOSTIC__
		static uint32_t protocolStatusCount;
#endif
		virtual void onRecvFirstMessage(NetworkMessage& msg);

		ProtocolStatus(Connection_ptr connection): Protocol(connection)
		{
#ifdef __ENABLE_SERVER_DIAGNOSTIC__
			protocolStatusCount++;
#endif
		}
		virtual ~ProtocolStatus()
		{
#ifdef __ENABLE_SERVER_DIAGNOSTIC__
			protocolStatusCount--;
#endif
		}

		enum {protocolId = 0xFF};
		enum {isSingleSocket = false};
		enum {hasChecksum = false};

		static const char* protocolName() {return "status protocol";}

	protected:
		static IpConnectMap ipConnectMap;
		#ifdef __DEBUG_NET_DETAIL__
		virtual void deleteProtocolTask();
		#endif
};

class Status
{
	public:
		virtual ~Status() {}
		static Status* getInstance()
		{
			static Status status;
			return &status;
		}

		std::string getStatusString(bool sendPlayers) const;
		void getInfo(uint32_t requestedInfo, OutputMessage_ptr output, NetworkMessage& msg) const;

		uint32_t getUptime() const {return (OTSYS_TIME() - m_start) / 1000;}
		int64_t getStart() const {return m_start;}

	protected:
		Status()
		{
			m_start = OTSYS_TIME();
		}

	private:
		int64_t m_start;
};
#endif
