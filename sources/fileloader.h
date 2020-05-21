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

#ifndef __FILELOADER__
#define __FILELOADER__
#include "otsystem.h"

struct NodeStruct;
typedef NodeStruct* NODE;

struct NodeStruct
{
	NodeStruct()
	{
		start = propsSize = type = 0;
		next = child = 0;
	}
	virtual ~NodeStruct() {}

	uint32_t start, propsSize, type;
	NodeStruct* next;
	NodeStruct* child;

	static void clearNet(NodeStruct* root) {if(root) clearChild(root);}
	private:
		static void clearNext(NodeStruct* node)
		{
			NodeStruct* deleteNode = node;
			NodeStruct* nextNode;
			while(deleteNode)
			{
				if(deleteNode->child)
					clearChild(deleteNode->child);

				nextNode = deleteNode->next;
				delete deleteNode;
				deleteNode = nextNode;
			}
		}

		static void clearChild(NodeStruct* node)
		{
			if(node->child)
				clearChild(node->child);

			if(node->next)
				clearNext(node->next);

			delete node;
		}
};

#define NO_NODE 0
enum FILELOADER_ERRORS
{
	ERROR_NONE,
	ERROR_INVALID_FILE_VERSION,
	ERROR_CAN_NOT_OPEN,
	ERROR_CAN_NOT_CREATE,
	ERROR_EOF,
	ERROR_SEEK_ERROR,
	ERROR_NOT_OPEN,
	ERROR_INVALID_NODE,
	ERROR_INVALID_FORMAT,
	ERROR_TELL_ERROR,
	ERROR_COULDNOTWRITE,
	ERROR_CACHE_ERROR
};

class PropStream;
class FileLoader
{
	public:
		FileLoader();
		virtual ~FileLoader();

		bool openFile(const char* name, const char* identifier, bool write, bool caching = false);
		const uint8_t* getProps(const NODE, uint32_t &size);
		bool getProps(const NODE, PropStream& props);
		NODE getChildNode(const NODE& parent, uint32_t &type) const;
		NODE getNextNode(const NODE& prev, uint32_t &type) const;

		void startNode(uint8_t type);
		void endNode();
		int32_t setProps(void* data, uint16_t size);

		int32_t getError() const {return m_lastError;}
		void clearError() {m_lastError = ERROR_NONE;}

	protected:
		enum SPECIAL_BYTES
		{
			NODE_START = 0xFE,
			NODE_END = 0xFF,
			ESCAPE_CHAR = 0xFD,
		};
		bool parseNode(NODE node);

		inline bool readByte(int32_t &value);
		inline bool readBytes(unsigned char* buffer, int32_t size, int32_t pos);
		inline bool checks(const NODE& node);
		inline bool safeSeek(uint32_t pos);
		inline bool safeTell(int32_t &pos);

	public:
		inline bool writeData(const void* data, int32_t size, bool unescape)
		{
			for(int32_t i = 0; i < size; ++i)
			{
				uint8_t c = *(((uint8_t*)data) + i);
				if(unescape && (c == NODE_START || c == NODE_END || c == ESCAPE_CHAR))
				{
					uint8_t tmp = ESCAPE_CHAR;

					size_t value = fwrite(&tmp, 1, 1, m_file);
					if(value != 1)
					{
						m_lastError = ERROR_COULDNOTWRITE;
						return false;
					}
				}

				size_t value = fwrite(&c, 1, 1, m_file);
				if(value != 1)
				{
					m_lastError = ERROR_COULDNOTWRITE;
					return false;
				}
			}

			return true;
		}

	protected:
		FILELOADER_ERRORS m_lastError;

		FILE* m_file;

		NODE m_root;
		uint32_t m_buffer_size;
		uint8_t* m_buffer;

		bool m_use_cache;
		struct _cache
		{
			uint32_t loaded, base;
			uint8_t* data;
			size_t size;
		};

		#define CACHE_BLOCKS 3
		uint32_t m_cache_size;
		_cache m_cached_data[CACHE_BLOCKS];

		#define NO_VALID_CACHE 0xFFFFFFFF
		uint32_t m_cache_index, m_cache_offset;

		inline uint32_t getCacheBlock(uint32_t pos);
		int32_t loadCacheBlock(uint32_t pos);
};

class PropStream
{
	public:
		PropStream() {end = NULL; p = NULL;}
		virtual ~PropStream() {}

		void init(const char* a, uint32_t size)
		{
			p = a;
			end = a + size;
		}
		int32_t size() const {return end - p;}

		template <typename T>
		inline bool getType(T& ret)
		{
			if(size() < (int32_t)sizeof(T))
				return false;

			ret = *((T*)p);
			p += sizeof(T);
			return true;
		}

		template <typename T>
		inline bool getStruct(T* &ret)
		{
			if(size() < (int32_t)sizeof(T))
			{
				ret = NULL;
				return false;
			}

			ret = (T*)p;
			p += sizeof(T);
			return true;
		}

		inline bool getByte(uint8_t& ret) {return getType(ret);}
		inline bool getShort(uint16_t& ret) {return getType(ret);}
		inline bool getTime(time_t& ret) {return getType(ret);}
		inline bool getLong(uint32_t& ret) {return getType(ret);}

		inline bool getFloat(float& ret)
		{
			// ugly hack, but it makes reading not depending on arch
			if(size() < (int32_t)sizeof(uint32_t))
				return false;

			float f;
			memcpy(&f, (uint32_t*)p, sizeof(uint32_t));

			ret = f;
			p += sizeof(uint32_t);
			return true;
		}

		inline bool getString(std::string& ret)
		{
			uint16_t strLen;
			return getShort(strLen) && getString(ret, strLen);
		}

		inline bool getString(std::string& ret, uint16_t strLen)
		{
			if(size() < (int32_t)strLen)
				return false;

			char* str = new char[strLen + 1];
			memcpy(str, p, strLen);
			str[strLen] = 0;

			ret.assign(str, strLen);
			delete[] str;
			p = p + strLen;
			return true;
		}

		inline bool getLongString(std::string& ret)
		{
			uint32_t strLen;
			if(!getLong(strLen))
				return false;

			if(size() < (int32_t)strLen)
				return false;

			char* str = new char[strLen + 1];
			memcpy(str, p, strLen);
			str[strLen] = 0;

			ret.assign(str, strLen);
			delete[] str;
			p = p + strLen;
			return true;
		}

		inline bool skip(int16_t n)
		{
			if(size() < n)
				return false;

			p += n;
			return true;
		}

	protected:
		const char* p;
		const char* end;
};

class PropWriteStream
{
	public:
		PropWriteStream()
		{
			bufferSize = 32;
			buffer = reinterpret_cast<char*>(malloc(bufferSize));
			if (!buffer) {
				throw std::bad_alloc();
			}
			size = 0;
		}
		~PropWriteStream() {free(buffer);}

		inline void clear() {
			size = 0;
		}

		const char* getStream(uint32_t& _size) const
		{
			_size = size;
			return buffer;
		}

		template <typename T>
		inline void addType(T add)
		{
			reserve(sizeof(T));
			memcpy(buffer + size, &add, sizeof(T));
			size += sizeof(T);
		}

		inline void addByte(uint8_t ret) {addType(ret);}
		inline void addShort(uint16_t ret) {addType(ret);}
		inline void addTime(time_t ret) {addType(ret);}
		inline void addLong(uint32_t ret) {addType(ret);}

		inline void addString(const std::string& add)
		{
			uint16_t strLen = add.size();
			addShort(strLen);
			reserve(strLen);
			memcpy(buffer + size, add.c_str(), strLen);
			size += strLen;
		}

		inline void addLongString(const std::string& add)
		{
			uint32_t strLen = add.size();
			addLong(strLen);
			reserve(strLen);
			memcpy(buffer + size, add.c_str(), strLen);
			size += strLen;
		}


	protected:
		void reserve(size_t length) {
			if ((bufferSize - size) >= length) {
				return;
			}

			do {
				bufferSize <<= 1;
			} while ((bufferSize - size) < length);

			void* newBuffer = realloc(buffer, bufferSize);
			if (!newBuffer) {
				throw std::bad_alloc();
			}

			buffer = reinterpret_cast<char*>(newBuffer);
		}

		char* buffer;
		uint32_t bufferSize, size;
};
#endif
