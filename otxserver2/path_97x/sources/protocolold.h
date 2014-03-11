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

#ifndef __PROTOCOL_OLD__
#define __PROTOCOL_OLD__
#include "protocol.h"

class NetworkMessage;
class ProtocolOld : public Protocol
{
	public:
#ifdef __ENABLE_SERVER_DIAGNOSTIC__
		static uint32_t protocolOldCount;
#endif
		virtual void onRecvFirstMessage(NetworkMessage& msg);

		ProtocolOld(Connection_ptr connection): Protocol(connection)
		{
#ifdef __ENABLE_SERVER_DIAGNOSTIC__
			protocolOldCount++;
#endif
		}
		virtual ~ProtocolOld()
		{
#ifdef __ENABLE_SERVER_DIAGNOSTIC__
			protocolOldCount--;
#endif
		}

		enum {isSingleSocket = false};
		enum {hasChecksum = false};

	protected:
		#ifdef __DEBUG_NET_DETAIL__
		virtual void deleteProtocolTask();
		#endif
		void disconnectClient(uint8_t error, const char* message);
};

class ProtocolOldLogin : public ProtocolOld
{
	public:
		ProtocolOldLogin(Connection_ptr connection) : ProtocolOld(connection) {}

		enum {protocolId = 0x01};
		static const char* protocolName() {return "old login protocol";}
};

class ProtocolOldGame : public ProtocolOld
{
	public:
		ProtocolOldGame(Connection_ptr connection) : ProtocolOld(connection) {}

		enum {protocolId = 0x0A};
		static const char* protocolName() {return "old game protocol";}
};
#endif
