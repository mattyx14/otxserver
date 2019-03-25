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
#include "resources.h"

#if defined(WINDOWS) && !defined(_CONSOLE)
#include "gui.h"
#endif

extern Game g_game;

#ifdef __ENABLE_SERVER_DIAGNOSTIC__
uint32_t ProtocolOld::protocolOldCount = 0;

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
	if(OutputMessage_ptr output = OutputMessagePool::getInstance()->getOutputMessage(this, false))
	{
		TRACK_MESSAGE(output);
		output->put<char>(error);
		output->putString(message);
		OutputMessagePool::getInstance()->send(output);
	}

	getConnection()->close();
}

void ProtocolOld::onRecvFirstMessage(NetworkMessage& msg)
{
	if(
#if defined(WINDOWS) && !defined(_CONSOLE)
		!GUI::getInstance()->m_connections ||
#endif
		g_game.getGameState() == GAMESTATE_SHUTDOWN)
	{
		getConnection()->close();
		return;
	}

	msg.skip(2);
	uint16_t version = msg.get<uint16_t>();

	msg.skip(12);
	if(version <= 760)
		disconnectClient(0x0A, "Only clients with protocol " CLIENT_VERSION_STRING " allowed!");

	if(!RSA_decrypt(msg))
	{
		getConnection()->close();
		return;
	}

	uint32_t key[4] = {msg.get<uint32_t>(), msg.get<uint32_t>(), msg.get<uint32_t>(), msg.get<uint32_t>()};
	enableXTEAEncryption();
	setXTEAKey(key);

	if(version <= 822)
		disableChecksum();

	disconnectClient(0x0A, "Only clients with protocol " CLIENT_VERSION_STRING " allowed!");
}