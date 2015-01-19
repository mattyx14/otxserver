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

#ifndef __CONNECTION_H__
#define __CONNECTION_H__
#include "otsystem.h"

#include "networkmessage.h"
#include <boost/utility.hpp>
#include <boost/enable_shared_from_this.hpp>

class OutputMessage;
typedef boost::shared_ptr<OutputMessage> OutputMessage_ptr;

class Connection;
typedef boost::shared_ptr<Connection> Connection_ptr;

class ServiceBase;
typedef boost::shared_ptr<ServiceBase> Service_ptr;

class ServicePort;
typedef boost::shared_ptr<ServicePort> ServicePort_ptr;

#ifdef __DEBUG_NET__
#define PRINT_ASIO_ERROR(description) \
	std::clog << "[Error - " << __FUNCTION__ << "] " << description << " - " << error.message() << " (" << error.message() << ")" << std::endl;
#else
#define PRINT_ASIO_ERROR(x)
#endif

struct LoginBlock
{
	int32_t lastLogin, lastProtocol, loginsAmount;
};

struct ConnectBlock
{
	uint32_t count;
	uint64_t startTime, blockTime;
};

class Protocol;
class ConnectionManager
{
	public:
		virtual ~ConnectionManager() {}
		static ConnectionManager* getInstance()
		{
			static ConnectionManager instance;
			return &instance;
		}

		Connection_ptr createConnection(boost::asio::ip::tcp::socket* socket,
			boost::asio::io_service& io_service, ServicePort_ptr servicers);
		void releaseConnection(Connection_ptr connection);

		bool isDisabled(uint32_t clientIp, int32_t protocolId);
		void addAttempt(uint32_t clientIp, int32_t protocolId, bool success);

		bool acceptConnection(uint32_t clientIp);
		void shutdown();

	protected:
		ConnectionManager() {}

		typedef std::map<uint32_t, LoginBlock> IpLoginMap;
		IpLoginMap ipLoginMap;

		typedef std::map<uint32_t, ConnectBlock> IpConnectMap;
		IpConnectMap ipConnectMap;

		std::list<Connection_ptr> m_connections;
		boost::recursive_mutex m_connectionManagerLock;
};

class Connection : public boost::enable_shared_from_this<Connection>, boost::noncopyable
{
	public:
		enum {writeTimeout = 30};
		enum {readTimeout = 30};

		enum ConnectionState_t
		{
			CONNECTION_STATE_OPEN = 0,
			CONNECTION_STATE_REQUEST_CLOSE,
			CONNECTION_STATE_CLOSING,
			CONNECTION_STATE_CLOSED
		};

#ifdef __ENABLE_SERVER_DIAGNOSTIC__
		static uint32_t connectionCount;
#endif
	private:
		Connection(boost::asio::ip::tcp::socket* socket, boost::asio::io_service& io_service, ServicePort_ptr servicePort):
			m_socket(socket), m_readTimer(io_service), m_writeTimer(io_service), m_service(io_service), m_servicePort(servicePort)
		{
			m_refCount = m_pendingWrite = m_pendingRead = 0;
			m_connectionState = CONNECTION_STATE_OPEN;
			m_receivedFirst = m_writeError = m_readError = false;
			m_protocol = NULL;

#ifdef __ENABLE_SERVER_DIAGNOSTIC__
			connectionCount++;
#endif
		}

		friend class ConnectionManager;

	public:
		virtual ~Connection()
		{
#ifdef __ENABLE_SERVER_DIAGNOSTIC__
			connectionCount--;
#endif
		}

		boost::asio::ip::tcp::socket& getHandle() {return *m_socket;}
		uint32_t getIP() const;
		uint32_t getEndpoint() const;

		void handle(Protocol* protocol);
		void accept();

		bool send(OutputMessage_ptr msg);
		void close();

		int32_t addRef() {return ++m_refCount;}
		int32_t unRef() {return --m_refCount;}

	private:
		void parseHeader(const boost::system::error_code& error);
		void parsePacket(const boost::system::error_code& error);

		void onWrite(OutputMessage_ptr msg, const boost::system::error_code& error);
		void onStop();

		void handleReadError(const boost::system::error_code& error);
		void handleWriteError(const boost::system::error_code& error);

		static void handleReadTimeout(boost::weak_ptr<Connection> weak, const boost::system::error_code& error);
		static void handleWriteTimeout(boost::weak_ptr<Connection> weak, const boost::system::error_code& error);

		void closeConnection();
		void deleteConnection();
		void releaseConnection();

		void onReadTimeout();
		void onWriteTimeout();

		void internalSend(OutputMessage_ptr msg);
		void closeSocket();

		NetworkMessage m_msg;
		Protocol* m_protocol;

		boost::asio::ip::tcp::socket* m_socket;
		boost::asio::deadline_timer m_readTimer, m_writeTimer;

		boost::asio::io_service& m_service;
		ServicePort_ptr m_servicePort;
		bool m_receivedFirst, m_writeError, m_readError;

		int32_t m_pendingWrite, m_pendingRead;
		ConnectionState_t m_connectionState;
		uint32_t m_refCount;

		static bool m_logError;
		boost::recursive_mutex m_connectionLock;
};

#endif
