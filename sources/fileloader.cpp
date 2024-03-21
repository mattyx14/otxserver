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
#include "fileloader.h"

FileLoader::FileLoader()
{
	m_file = NULL;
	m_root = NULL;
	m_buffer = new uint8_t[1024];
	m_buffer_size = 1024;
	m_lastError = ERROR_NONE;

	//cache
	m_use_cache = false;
	m_cache_size = 0;
	m_cache_index = m_cache_offset = NO_VALID_CACHE;
	memset(m_cached_data, 0, sizeof(m_cached_data));
}

FileLoader::~FileLoader()
{
	if(m_file)
	{
		fclose(m_file);
		m_file = NULL;
	}

	NodeStruct::clearNet(m_root);
	delete[] m_buffer;
	for(int32_t i = 0; i < CACHE_BLOCKS; ++i)
		delete[] m_cached_data[i].data;
}

bool FileLoader::openFile(const char* name, const char* accept_identifier, bool write, bool caching /*= false*/)
{
	if(write)
	{
		m_file = fopen(name, "wb");
		if(!m_file)
		{
			m_lastError = ERROR_CAN_NOT_CREATE;
			return false;
		}

		uint32_t version = 0;
		writeData(&version, sizeof(version), false);
		return true;
	}

	m_file = fopen(name, "rb");
	if(!m_file)
	{
		m_lastError = ERROR_CAN_NOT_OPEN;
		return false;
	}

	char identifier[4];
	if(fread(identifier, 1, 4, m_file) < 4)
	{
		fclose(m_file);
		m_file = NULL;
		m_lastError = ERROR_EOF;
		return false;
	}

	// The first four bytes must either match the accept identifier or be 0x00000000 (wildcard)
	if(memcmp(identifier, accept_identifier, 4) != 0 && memcmp(identifier, "\0\0\0\0", 4) != 0)
	{
		fclose(m_file);
		m_file = NULL;
		m_lastError = ERROR_INVALID_FILE_VERSION;
		return false;
	}

	if(caching)
	{
		m_use_cache = true;
		fseek(m_file, 0, SEEK_END);
		int32_t file_size = ftell(m_file);
		m_cache_size = std::min(32768, std::max(file_size/20, 8192)) & ~0x1FFF;
	}

	if(!safeSeek(4))
	{
		m_lastError = ERROR_INVALID_FORMAT;
		return false;
	}

	delete m_root;
	m_root = new NodeStruct();
	m_root->start = 4;

	int32_t byte = 0;
	if(safeSeek(4) && readByte(byte) && byte == NODE_START)
		return parseNode(m_root);

	return false;
}

bool FileLoader::parseNode(NODE node)
{
	int32_t byte, pos;
	NODE currentNode = node;
	while(true)
	{
		//read node type
		if(!readByte(byte))
			return false;

		currentNode->type = byte;
		bool setPropsSize = false;
		while(true)
		{
			if(!readByte(byte))
				return false;

			bool skipNode = false;
			switch(byte)
			{
				case NODE_START:
				{
					//child node start
					if(!safeTell(pos))
						return false;

					NODE childNode = new NodeStruct();
					childNode->start = pos;
					currentNode->propsSize = pos - currentNode->start - 2;
					currentNode->child = childNode;

					setPropsSize = true;
					if(!parseNode(childNode))
						return false;

					break;
				}

				case NODE_END:
				{
					//current node end
					if(!setPropsSize)
					{
						if(!safeTell(pos))
							return false;

						currentNode->propsSize = pos - currentNode->start - 2;
					}

					if(!readByte(byte))
						return true;

					switch(byte)
					{
						case NODE_START:
						{
							//starts next node
							if(!safeTell(pos))
								return false;

							skipNode = true;
							NODE nextNode = new NodeStruct();
							nextNode->start = pos;
							currentNode->next = nextNode;
							currentNode = nextNode;

							break;
						}

						case NODE_END:
							return safeTell(pos) && safeSeek(pos);

						default:
							m_lastError = ERROR_INVALID_FORMAT;
							return false;
					}

					break;
				}

				case ESCAPE_CHAR:
				{
					if(!readByte(byte))
						return false;

					break;
				}

				default:
					break;
			}

			if(skipNode)
				break;
		}
	}

	return false;
}

const uint8_t* FileLoader::getProps(const NODE node, uint32_t &size)
{
	if(!node)
		return NULL;

	if(node->propsSize >= m_buffer_size)
	{
		delete[] m_buffer;
		while(node->propsSize >= m_buffer_size)
			m_buffer_size *= 2;

		m_buffer = new uint8_t[m_buffer_size];
	}

	//get buffer
	if(!readBytes(m_buffer, node->propsSize, node->start + 2))
		return NULL;

	//unscape buffer
	uint32_t j = 0;
	bool escaped = false;
	for(uint32_t i = 0; i < node->propsSize; ++i, ++j)
	{
		if(m_buffer[i] == ESCAPE_CHAR)
		{
			//escape char found, skip it and write next
			++i;
			m_buffer[j] = m_buffer[i];
			//is neede a displacement for next bytes
			escaped = true;
		}
		else if(escaped) //perform that displacement
			m_buffer[j] = m_buffer[i];
	}

	size = j;
	return m_buffer;
}

bool FileLoader::getProps(const NODE node, PropStream &props)
{
	uint32_t size;
	if(const uint8_t* a = getProps(node, size))
	{
		props.init((char*)a, size);
		return true;
	}

	props.init(NULL, 0);
	return false;
}

int32_t FileLoader::setProps(void* data, uint16_t size)
{
	//data
	if(!writeData(data, size, true))
		return getError();

	return ERROR_NONE;
}

void FileLoader::startNode(uint8_t type)
{
	uint8_t nodeBegin = NODE_START;
	writeData(&nodeBegin, sizeof(nodeBegin), false);
	writeData(&type, sizeof(type), true);
}

void FileLoader::endNode()
{
	uint8_t nodeEnd = NODE_END;
	writeData(&nodeEnd, sizeof(nodeEnd), false);
}

NODE FileLoader::getChildNode(const NODE& parent, uint32_t &type) const
{
	if(!parent)
	{
		type = m_root->type;
		return m_root;
	}

	NODE child = parent->child;
	if(child)
		type = child->type;

	return child;
}

NODE FileLoader::getNextNode(const NODE& prev, uint32_t &type) const
{
	if(!prev)
		return NO_NODE;

	NODE next = prev->next;
	if(next)
		type = next->type;

	return next;
}

inline bool FileLoader::readByte(int32_t &value)
{
	if(m_use_cache)
	{
		if(m_cache_index == NO_VALID_CACHE)
		{
			m_lastError = ERROR_CACHE_ERROR;
			return false;
		}

		if(m_cache_offset >= m_cached_data[m_cache_index].size)
		{
			int32_t pos = m_cache_offset + m_cached_data[m_cache_index].base, tmp = getCacheBlock(pos);
			if(tmp < 0)
				return false;

			m_cache_index = tmp;
			m_cache_offset = pos - m_cached_data[m_cache_index].base;
			if(m_cache_offset >= m_cached_data[m_cache_index].size)
				return false;
		}

		value = m_cached_data[m_cache_index].data[m_cache_offset];
		m_cache_offset++;
		return true;
	}

	value = fgetc(m_file);
	if(value != EOF)
		return true;

	m_lastError = ERROR_EOF;
	return false;
}

inline bool FileLoader::readBytes(uint8_t* buffer, int32_t size, int32_t pos)
{
	if(m_use_cache)
	{
		//seek at pos
		uint32_t reading, remain = size, bufferPos = 0;
		do
		{
			//prepare cache
			uint32_t i = getCacheBlock(pos);
			if(i == NO_VALID_CACHE)
				return false;

			m_cache_index = i;
			m_cache_offset = pos - m_cached_data[i].base;

			//get maximum read block size and calculate remaining bytes
			reading = std::min(remain, (uint32_t)m_cached_data[i].size - m_cache_offset);
			remain = remain - reading;

			//read it
			memcpy(buffer + bufferPos, m_cached_data[m_cache_index].data + m_cache_offset, reading);

			//update variables
			m_cache_offset = m_cache_offset + reading;
			bufferPos = bufferPos + reading;
			pos = pos + reading;
		}
		while(remain > 0);
		return true;
	}

	if(fseek(m_file, pos, SEEK_SET))
	{
		m_lastError = ERROR_SEEK_ERROR;
		return false;
	}

	if(fread(buffer, 1, size, m_file) == (uint32_t)size)
		return true;

	m_lastError = ERROR_EOF;
	return false;
}

inline bool FileLoader::checks(const NODE& node)
{
	if(!m_file)
	{
		m_lastError = ERROR_NOT_OPEN;
		return false;
	}

	if(!node)
	{
		m_lastError = ERROR_INVALID_NODE;
		return false;
	}

	return true;
}

inline bool FileLoader::safeSeek(uint32_t pos)
{
	if(m_use_cache)
	{
		uint32_t i = getCacheBlock(pos);
		if(i == NO_VALID_CACHE)
			return false;

		m_cache_index = i;
		m_cache_offset = pos - m_cached_data[i].base;
	}
	else if(fseek(m_file, pos, SEEK_SET))
	{
		m_lastError = ERROR_SEEK_ERROR;
		return false;
	}

	return true;
}

inline bool FileLoader::safeTell(int32_t &pos)
{
	if(m_use_cache)
	{
		if(m_cache_index == NO_VALID_CACHE)
		{
			m_lastError = ERROR_CACHE_ERROR;
			return false;
		}

		pos = m_cached_data[m_cache_index].base + m_cache_offset - 1;
		return true;
	}

	pos = ftell(m_file);
	if(pos == -1)
	{
		m_lastError = ERROR_TELL_ERROR;
		return false;
	}

	pos = pos - 1;
	return true;
}

inline uint32_t FileLoader::getCacheBlock(uint32_t pos)
{
	bool found = false;
	uint32_t i, base_pos = pos & ~(m_cache_size - 1);
	for(i = 0; i < CACHE_BLOCKS; ++i)
	{
		if(!m_cached_data[i].loaded || m_cached_data[i].base != base_pos)
			continue;

		found = true;
		break;
	}

	if(!found)
		i = loadCacheBlock(pos);

	return i;
}

int32_t FileLoader::loadCacheBlock(uint32_t pos)
{
	int32_t i, loading_cache = -1, base_pos = pos & ~(m_cache_size - 1);
	for(i = 0; i < CACHE_BLOCKS; ++i)
	{
		if(m_cached_data[i].loaded)
			continue;

		loading_cache = i;
		break;
	}

	if(loading_cache == -1)
	{
		for(i = 0; i < CACHE_BLOCKS; ++i)
		{
			if((long)(std::abs((long)m_cached_data[i].base - base_pos)) <= (long)(2 * m_cache_size))
				continue;

			loading_cache = i;
			break;
		}

		if(loading_cache == -1)
			loading_cache = 0;
	}

	if(m_cached_data[loading_cache].data == NULL)
		m_cached_data[loading_cache].data = new uint8_t[m_cache_size];

	m_cached_data[loading_cache].base = base_pos;
	if(fseek(m_file, m_cached_data[loading_cache].base, SEEK_SET))
	{
		m_lastError = ERROR_SEEK_ERROR;
		return -1;
	}

	uint32_t size = fread(m_cached_data[loading_cache].data, 1, m_cache_size, m_file);
	m_cached_data[loading_cache].size = size;
	if(size < (pos - m_cached_data[loading_cache].base))
	{
		m_lastError = ERROR_SEEK_ERROR;
		return -1;
	}

	m_cached_data[loading_cache].loaded = 1;
	return loading_cache;
}