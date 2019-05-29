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
#include "protocolold.h"

#include "outputmessage.h"
#include "connection.h"
#include "game.h"

extern Game g_game;

#ifdef __ENABLE_SERVER_DIAGNOSTIC__
//uint32_t ProtocolOld::protocolOldCount = 0;

#endif
#ifdef __DEBUG_NET_DETAIL__
void ProtocolOld::deleteProtocolTask()
{
	std::clog << "Deleting ProtocolOld" << std::endl;
	Protocol::deleteProtocolTask();
}

#endif
void ProtocolOld::disconnectClient(uint8_t error, const char* message)
{
	OutputMessage_ptr output = OutputMessagePool::getOutputMessage();
	output->addByte(error);
	output->addString(message);
	send(output);
	disconnect();
}

void ProtocolOld::onRecvFirstMessage(NetworkMessage& msg)
{
	if(g_game.getGameState() == GAMESTATE_SHUTDOWN)
	{
		disconnect();
		return;
	}

	msg.skipBytes(2);
	uint16_t version = msg.get<uint16_t>();

	msg.skipBytes(12);
	if(version <= 760)
		disconnectClient(0x0A, "Only clients with protocol " CLIENT_VERSION_STRING " allowed!");

	if(!RSA_decrypt(msg))
	{
		disconnect();
		return;
	}

	uint32_t key[4] = {msg.get<uint32_t>(), msg.get<uint32_t>(), msg.get<uint32_t>(), msg.get<uint32_t>()};
	enableXTEAEncryption();
	setXTEAKey(key);

	if(version <= 822)
		disableChecksum();

	disconnectClient(0x0A, "Only clients with protocol " CLIENT_VERSION_STRING " allowed!");
}
