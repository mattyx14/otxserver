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
		// static protocol information
		enum {server_sends_first = false};
		enum {protocol_identifier = 0x01};
		enum {use_checksum = false};
		static const char* protocol_name() {
			return "old login protocol";
		}

		explicit ProtocolOld(Connection_ptr connection): Protocol(connection) {}

		void onRecvFirstMessage(NetworkMessage& msg) final;

	protected:
		void disconnectClient(uint8_t error, const char* message);
};

#endif