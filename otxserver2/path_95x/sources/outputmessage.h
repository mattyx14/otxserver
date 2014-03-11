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

#ifndef __OUTPUT_MESSAGE__
#define __OUTPUT_MESSAGE__
#include "otsystem.h"
#include "tools.h"

#include "connection.h"
#include "networkmessage.h"

#ifdef __TRACK_NETWORK__
#include <iostream>
#include <sstream>
#endif

class Protocol;
#define OUTPUT_POOL_SIZE 100

class OutputMessage : public NetworkMessage, boost::noncopyable
{
	private:
		OutputMessage() {freeMessage();}

	public:
		virtual ~OutputMessage() {}

		Protocol* getProtocol() const {return m_protocol;}
		Connection_ptr getConnection() const {return m_connection;}

		char* getOutputBuffer() {return (char*)&m_buffer[m_outputBufferStart];}
		uint64_t getFrame() const {return m_frame;}

		void writeMessageLength() {addHeader((uint16_t)(m_size));}
		void addCryptoHeader(bool addChecksum)
		{
			if(addChecksum)
				addHeader((adlerChecksum((uint8_t*)(m_buffer + m_outputBufferStart), m_size)));

			addHeader((uint16_t)(m_size));
		}

#ifdef __TRACK_NETWORK__
		virtual void track(std::string file, int32_t line, std::string func)
		{
			if(lastUses.size() >= 25)
				lastUses.pop_front();

			std::ostringstream os;
			os << /*file << ":" */"line " << line << " " << func;
			lastUses.push_back(os.str());
		}

		virtual void clearTrack()
		{
			lastUses.clear();
		}

		void printTrace()
		{
			uint32_t n = 1;
			for(std::list<std::string>::const_reverse_iterator it = lastUses.rbegin(); it != lastUses.rend(); ++it, ++n)
				std::clog << "\t" << n << ".\t" << (*it) << std::endl;
		}
#endif

		enum OutputMessageState
		{
			STATE_FREE,
			STATE_ALLOCATED,
			STATE_ALLOCATED_NO_AUTOSEND,
			STATE_WAITING
		};

	protected:
		template <typename T>
		inline void addHeader(T value)
		{
			if((int32_t)m_outputBufferStart - (int32_t)sizeof(T) < 0)
			{
				std::clog << "[Error - OutputMessage::addHeader] m_outputBufferStart(" << m_outputBufferStart << ") < " << sizeof(T) << std::endl;
				return;
			}

			m_outputBufferStart -= sizeof(T);
			*(T*)(m_buffer + m_outputBufferStart) = value;
			m_size += sizeof(T);
		}

		void freeMessage()
		{
			setConnection(Connection_ptr());
			setProtocol(NULL);
			m_frame = 0;

			//allocate enough size for headers:
			// 2 bytes for unencrypted message
			// 4 bytes for checksum
			// 2 bytes for encrypted message
			m_outputBufferStart = 8;
			setState(OutputMessage::STATE_FREE);
		}

		friend class OutputMessagePool;

		void setProtocol(Protocol* protocol) {m_protocol = protocol;}
		void setConnection(Connection_ptr connection) {m_connection = connection;}

		void setState(OutputMessageState state) {m_state = state;}
		OutputMessageState getState() const {return m_state;}

		void setFrame(uint64_t frame) {m_frame = frame;}

		Protocol* m_protocol;
		Connection_ptr m_connection;
#ifdef __TRACK_NETWORK__
		std::list<std::string> lastUses;
#endif

		OutputMessageState m_state;
		uint64_t m_frame;
		uint32_t m_outputBufferStart;
};

class OutputMessagePool
{
	private:
		OutputMessagePool();

	public:
#ifdef __ENABLE_SERVER_DIAGNOSTIC__
		static uint32_t outputMessagePoolCount;
#endif
		virtual ~OutputMessagePool();
		static OutputMessagePool* getInstance()
		{
			static OutputMessagePool instance;
			return &instance;
		}

		OutputMessage_ptr getOutputMessage(Protocol* protocol, bool autoSend = true);

		void send(OutputMessage_ptr msg);
		void stop() {m_shutdown = true;}
		void sendAll();

		void startExecutionFrame();
		void autoSend(OutputMessage_ptr msg);

#ifdef __ENABLE_SERVER_DIAGNOSTIC__
		size_t getTotalMessageCount() const {return (size_t)outputMessagePoolCount;}
#else
		size_t getTotalMessageCount() const {return m_allMessages.size();}
#endif
		size_t getAvailableMessageCount() const {return m_outputMessages.size();}
		size_t getAutoMessageCount() const {return m_autoSend.size();}
		size_t getQueuedMessageCount() const {return m_addQueue.size();}

	protected:
		void configureOutputMessage(OutputMessage_ptr msg, Protocol* protocol, bool autoSend);

		void releaseMessage(OutputMessage* msg);
		void internalReleaseMessage(OutputMessage* msg);

		typedef std::list<OutputMessage_ptr> OutputMessageList;
		OutputMessageList m_autoSend;
		OutputMessageList m_addQueue;

		typedef std::list<OutputMessage*> InternalList;
		InternalList m_outputMessages;
		InternalList m_allMessages;

		boost::recursive_mutex m_outputPoolLock;
		uint64_t m_frameTime;
		bool m_shutdown;
};

#ifdef __TRACK_NETWORK__
	#define TRACK_MESSAGE(omsg) (omsg)->track(__FILE__, __LINE__, __FUNCTION__)
#else
	#define TRACK_MESSAGE(omsg)
#endif
#endif
