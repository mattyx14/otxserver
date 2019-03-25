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
#if defined WINDOWS
#include <winerror.h>
#endif

#include "protocol.h"
#include "tools.h"

#include "scheduler.h"
#include "connection.h"
#include "outputmessage.h"

#include <openssl/rsa.h>
extern RSA* g_RSA;

void Protocol::onSendMessage(OutputMessage_ptr msg)
{
	#ifdef __DEBUG_NET_DETAIL__
	std::clog << "Protocol::onSendMessage" << std::endl;
	#endif
	if(!m_rawMessages)
	{
		msg->writeMessageLength();
		if(m_encryptionEnabled)
		{
			#ifdef __DEBUG_NET_DETAIL__
			std::clog << "Protocol::onSendMessage - encrypt" << std::endl;
			#endif
			XTEA_encrypt(*msg);
		}

		if(m_checksumEnabled)
		{
			#ifdef __DEBUG_NET_DETAIL__
			std::clog << "Protocol::onSendMessage - crypto header" << std::endl;
			#endif
			msg->addCryptoHeader(m_checksumEnabled);
		}
	}

	if(msg == m_outputBuffer)
		m_outputBuffer.reset();
}

void Protocol::onRecvMessage(NetworkMessage& msg)
{
	#ifdef __DEBUG_NET_DETAIL__
	std::clog << "Protocol::onRecvMessage" << std::endl;
	#endif
	if(m_encryptionEnabled)
	{
		#ifdef __DEBUG_NET_DETAIL__
		std::clog << "Protocol::onRecvMessage - decrypt" << std::endl;
		#endif
		if(!XTEA_decrypt(msg))
			return;
	}

	parsePacket(msg);
}

OutputMessage_ptr Protocol::getOutputBuffer()
{
	if(m_outputBuffer)
		return m_outputBuffer;

	if(!m_connection)
		return OutputMessage_ptr();

	m_outputBuffer = OutputMessagePool::getInstance()->getOutputMessage(this);
	return m_outputBuffer;
}

void Protocol::releaseProtocol()
{
	if(m_refCount > 0)
		Scheduler::getInstance().addEvent(createSchedulerTask(SCHEDULER_MINTICKS, boost::bind(&Protocol::releaseProtocol, this)));
	else
		deleteProtocolTask();
}

void Protocol::deleteProtocolTask()
{
	//dispather thread
	assert(!m_refCount);
	setConnection(Connection_ptr());
	delete this;
}

void Protocol::XTEA_encrypt(OutputMessage& msg)
{
	//add bytes until reach 8 multiple
	uint16_t messageLength = msg.size();
	if(messageLength % 8)
	{
		uint16_t n = 8 - (messageLength % 8);
		msg.putPadding(n);
		messageLength += n;
	}

	int32_t readPos = -1;
	uint32_t *buffer = (uint32_t*)msg.getOutputBuffer(), delta = 0x61C88647;
	while(++readPos < messageLength >> 2)
	{
		uint32_t v0 = buffer[readPos], v1 = buffer[readPos + 1], sum = 0;
		for(int32_t i = 0; i < 32; ++i)
		{
			v0 += ((v1 << 4 ^ v1 >> 5) + v1) ^ (sum + m_key[sum & 3]);
			sum -= delta;
			v1 += ((v0 << 4 ^ v0 >> 5) + v0) ^ (sum + m_key[sum >> 11 & 3]);
		}

		buffer[readPos] = v0;
		buffer[++readPos] = v1;
	}
}

bool Protocol::XTEA_decrypt(NetworkMessage& msg)
{
	if((msg.size() - 6) % 8)
	{
		std::clog << "[Failure - Protocol::XTEA_decrypt] Not valid encrypted message size";
		int32_t ip = getIP();
		if(ip)
			std::clog << " (IP: " << convertIPAddress(ip) << ")";

		std::clog << std::endl;
		return false;
	}

	int32_t messageLength = msg.size() - 6, readPos = -1;
	uint32_t *buffer = (uint32_t*)(msg.buffer() + msg.position()), delta = 0x61C88647;
	while(++readPos < messageLength >> 2)
	{
		uint32_t v0 = buffer[readPos], v1 = buffer[readPos + 1], sum = 0xC6EF3720;
		for(int32_t i = 0; i < 32; ++i)
		{
			v1 -= ((v0 << 4 ^ v0 >> 5) + v0) ^ (sum + m_key[sum >> 11 & 3]);
			sum += delta;
			v0 -= ((v1 << 4 ^ v1 >> 5) + v1) ^ (sum + m_key[sum & 3]);
		}

		buffer[readPos] = v0;
		buffer[++readPos] = v1;
	}

	int32_t tmp = msg.get<uint16_t>();
	if(tmp > msg.size() - 8)
	{
		std::clog << "[Failure - Protocol::XTEA_decrypt] Not valid unencrypted message size";
		uint32_t ip = getIP();
		if(ip)
			std::clog << " (IP: " << convertIPAddress(ip) << ")";

		std::clog << std::endl;
		return false;
	}

	msg.setSize(tmp);
	return true;
}

bool Protocol::RSA_decrypt(NetworkMessage& msg)
{
	if(msg.size() - msg.position() != 128)
	{
		std::clog << "[Warning - Protocol::RSA_decrypt] Not valid packet size";
		int32_t ip = getIP();
		if(ip)
			std::clog << " (IP: " << convertIPAddress(ip) << ")";

		std::clog << std::endl;
		return false;
	}

	uint16_t size = msg.size();
	RSA_private_decrypt(128, (uint8_t*)(msg.buffer() + msg.position()), (uint8_t*)msg.buffer(), g_RSA, RSA_NO_PADDING);
	msg.setSize(size);

	msg.setPosition(0);
	if(!msg.get<char>())
		return true;

	std::clog << "[Warning - Protocol::RSA_decrypt] First byte != 0";
	int32_t ip = getIP();
	if(ip)
		std::clog << " (IP: " << convertIPAddress(ip) << ")";

	std::clog << std::endl;
	return false;
}

uint32_t Protocol::getIP() const
{
	if(Connection_ptr connection = getConnection())
		return connection->getIP();

	return 0;
}
