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
#include "otpch.h"
#include <iostream>

#include "networkmessage.h"
#include "position.h"
#include "item.h"
#include "player.h"

SocketCode_t NetworkMessage::read(SOCKET socket, bool ignoreLength, int32_t timeout/* = NETWORK_RETRY_TIME*/)
{
	int32_t waiting = 0, data = NETWORK_DEFAULT_SIZE;
	if(!ignoreLength)
	{
		do
		{
			// just read the size to avoid reading 2 messages at once
			int32_t ret = recv(socket, (char*)m_buffer, NETWORK_HEADER_SIZE, 0);
			if(ret <= 0)
			{
				if(errno == EWOULDBLOCK)
				{
					ret = 0;
					OTSYS_SLEEP(10);

					waiting += 10;
					if(waiting > timeout)
					{
						reset(NETWORK_HEADER_SIZE);
						return SOCKET_CODE_TIMEOUT;
					}
				}
				else
				{
					reset(NETWORK_HEADER_SIZE);
					return SOCKET_CODE_ERROR;
				}
			}

			m_size += ret;
		}
		while(m_size < NETWORK_HEADER_SIZE);

		// for now we expect 2 bytes at once, it should not be splitted
		data = (int32_t)(m_buffer[0] | m_buffer[1] << 8);
		if(m_size != NETWORK_HEADER_SIZE || data > NETWORK_MAX_SIZE - NETWORK_HEADER_SIZE)
		{
			reset(NETWORK_HEADER_SIZE);
			return SOCKET_CODE_ERROR;
		}
	}

	int32_t recvd = 0;
	do
	{
		// read the real data
		int32_t ret = recv(socket, (char*)m_buffer + recvd + NETWORK_HEADER_SIZE, data - recvd, 0);
		if(ret <= 0)
		{
			if(errno == EWOULDBLOCK)
			{
				ret = 0;
				OTSYS_SLEEP(100);

				waiting += 100;
				if(waiting > timeout)
				{
					reset(NETWORK_HEADER_SIZE);
					return SOCKET_CODE_TIMEOUT;
				}
			}
			else if(data == NETWORK_DEFAULT_SIZE)
				break;
			else
			{
				reset(NETWORK_HEADER_SIZE);
				return SOCKET_CODE_ERROR;
			}
		}

		recvd += ret;
	}
	while(recvd < data);
	m_size += recvd;

	// we got something unexpected/incomplete
	if(m_size <= NETWORK_HEADER_SIZE || (!ignoreLength && m_size - NETWORK_HEADER_SIZE != data))
	{
		reset(NETWORK_HEADER_SIZE);
		return SOCKET_CODE_ERROR;
	}

	m_position = NETWORK_HEADER_SIZE;
	return SOCKET_CODE_OK;
}

SocketCode_t NetworkMessage::write(SOCKET socket, int32_t timeout/* = NETWORK_RETRY_TIME*/)
{
	if(!m_size)
		return SOCKET_CODE_OK;

	m_buffer[2] = (uint8_t)(m_size);
	m_buffer[3] = (uint8_t)(m_size >> 8);

	int32_t sent = 0, waiting = 0;
	do
	{
		int32_t ret = send(socket, (char*)m_buffer + sent + NETWORK_HEADER_SIZE,
			std::min(m_size - sent + NETWORK_HEADER_SIZE, 1000), 0);
		if(ret <= 0)
		{
			if(errno == EWOULDBLOCK)
			{
				ret = 0;
				OTSYS_SLEEP(100);

				waiting += 100;
				if(waiting > timeout)
					return SOCKET_CODE_TIMEOUT;
			}
			else
				return SOCKET_CODE_ERROR;
		}

	    	sent += ret;
	}
	while(sent < m_size + NETWORK_HEADER_SIZE);
	return SOCKET_CODE_OK;
}

std::string NetworkMessage::getString(bool peek/* = false*/, uint16_t size/* = 0*/)
{
	if(!size)
		size = get<uint16_t>(peek);

	uint16_t position = m_position;
	if(peek)
		position += 2;

	if(size >= (16384 - position))
		return std :: string();

	char* v = (char*)(m_buffer + position);
	if(peek)
		return std::string(v, size);

	m_position += size;
	return std::string(v, size);
}

Position NetworkMessage::getPosition()
{
	Position pos;
	pos.x = get<uint16_t>();
	pos.y = get<uint16_t>();
	pos.z = get<char>();
	return pos;
}

void NetworkMessage::putString(const char* value, int length, bool addSize/* = true*/)
{
	uint32_t size = (uint32_t)length;
	if(!hasSpace(size + (addSize ? 2 : 0)) || size > 8192)
		return;

	if(addSize)
		put<uint16_t>(size);

	memcpy((char*)(m_buffer + m_position), value, length);
	m_position += size;
	m_size += size;
}

void NetworkMessage::putPadding(uint32_t amount)
{
	if(!hasSpace(amount))
		return;

	memset((void*)&m_buffer[m_position], 0x33, amount);
	m_size += amount;
}

void NetworkMessage::putPosition(const Position& pos)
{
	put<uint16_t>(pos.x);
	put<uint16_t>(pos.y);
	put<char>(pos.z);
}

void NetworkMessage::putItem(uint16_t id, uint8_t count)
{
	const ItemType &it = Item::items[id];
	put<uint16_t>(it.clientId);
	if(it.stackable)
		put<char>(count);
	else if(it.isSplash() || it.isFluidContainer())
		put<char>(fluidMap[count % 8]);
}

void NetworkMessage::putItem(const Item* item)
{
	const ItemType& it = Item::items[item->getID()];
	put<uint16_t>(it.clientId);
	if(it.stackable)
		put<char>(item->getSubType());
	else if(it.isSplash() || it.isFluidContainer())
		put<char>(fluidMap[item->getSubType() % 8]);
}

void NetworkMessage::putItemId(const Item* item)
{
	const ItemType& it = Item::items[item->getID()];
	put<uint16_t>(it.clientId);
}

void NetworkMessage::putItemId(uint16_t itemId)
{
	const ItemType& it = Item::items[itemId];
	put<uint16_t>(it.clientId);
}

int32_t NetworkMessage::decodeHeader()
{
	int32_t size = (int32_t)(m_buffer[0] | m_buffer[1] << 8);
	m_size = size;
	return size;
}
