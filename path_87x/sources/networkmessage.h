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
// along with this program. If not, see <http://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////

#ifndef __NETWORKMESSAGE__
#define __NETWORKMESSAGE__
#include "otsystem.h"
#include "const.h"

enum SocketCode_t
{
	SOCKET_CODE_OK,
	SOCKET_CODE_TIMEOUT,
	SOCKET_CODE_ERROR,
};

class Item;
class Position;

class NetworkMessage
{
	public:
		NetworkMessage() {reset();}
		virtual ~NetworkMessage() {}

		// resets the internal buffer to an empty message
		void reset(uint16_t size = NETWORK_CRYPTOHEADER_SIZE)
		{
			m_size = 0;
			m_position = size;
		}

		// socket functions
		SocketCode_t read(SOCKET socket, bool ignoreLength, int32_t timeout = NETWORK_RETRY_TIMEOUT);
		SocketCode_t write(SOCKET socket, int32_t timeout = NETWORK_RETRY_TIMEOUT);

		// simple read functions for incoming message
		template<typename T>
		T get(bool peek = false)
		{
			T value = *(T*)(m_buffer + m_position);
			if(peek)
				return value;

			m_position += sizeof(T);
			return value;
		}

		std::string getString(bool peek = false, uint16_t size = 0);
		std::string getRaw(bool peek = false) {return getString(peek, m_size - m_position);}

		// read for complex types
		Position getPosition();

		// skips count unknown/unused bytes in an incoming message
		void skip(int32_t count) {m_position += count;}

		// simple write functions for outgoing message
		template<typename T>
		void put(T value)
		{
			if(!hasSpace(sizeof(T)))
				return;

			*(T*)(m_buffer + m_position) = value;
			m_position += sizeof(T);
			m_size += sizeof(T);
		}

		void putString(const std::string& value, bool addSize = true) {putString(value.c_str(), value.length(), addSize);}
		void putString(const char* value, int length, bool addSize = true);

		void putPadding(uint32_t amount);

		// write for complex types
		void putPosition(const Position& pos);
		void putItem(uint16_t id, uint8_t count);
		void putItem(const Item* item);
		void putItemId(const Item* item);
		void putItemId(uint16_t itemId);

		int32_t decodeHeader();

		// message propeties functions
	  	uint16_t size() const {return m_size;}
		void setSize(uint16_t size) {m_size = size;}

		uint16_t position() const {return m_position;}
		void setPosition(uint16_t position) {m_position = position;}

		char* buffer() {return (char*)&m_buffer[0];}
		char* bodyBuffer()
		{
			m_position = NETWORK_HEADER_SIZE;
			return (char*)&m_buffer[NETWORK_HEADER_SIZE];
		}

#ifdef __TRACK_NETWORK__
		virtual void track(std::string file, int32_t line, std::string func) {}
		virtual void clearTrack() {}

#endif
	protected:
		// used to check available space while writing
		inline bool hasSpace(int32_t size) const {return (size + m_position < NETWORK_MAX_SIZE - 16);}

		// message propeties
		uint16_t m_size;
		uint16_t m_position;

		// message data
		uint8_t m_buffer[NETWORK_MAX_SIZE];
};

typedef boost::shared_ptr<NetworkMessage> NetworkMessage_ptr;
#endif
