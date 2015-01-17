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

#ifndef __PROTOCOL_LOGIN__
#define __PROTOCOL_LOGIN__
#include "protocol.h"

class NetworkMessage;
class ProtocolLogin : public Protocol
{
	public:
#ifdef __ENABLE_SERVER_DIAGNOSTIC__
		static uint32_t protocolLoginCount;
#endif
		virtual void onRecvFirstMessage(NetworkMessage& msg);

		ProtocolLogin(Connection_ptr connection) : Protocol(connection)
		{
			enableChecksum();
#ifdef __ENABLE_SERVER_DIAGNOSTIC__
			protocolLoginCount++;
#endif
		}
		virtual ~ProtocolLogin()
		{
#ifdef __ENABLE_SERVER_DIAGNOSTIC__
			protocolLoginCount--;
#endif
		}

		enum {protocolId = 0x01};
		enum {isSingleSocket = false};
		enum {hasChecksum = true};

		static const char* protocolName() {return "login protocol";}

	protected:
		#ifdef __DEBUG_NET_DETAIL__
		virtual void deleteProtocolTask();
		#endif
		void disconnectClient(uint8_t error, const char* message);
};
#endif
