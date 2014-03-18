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
#include "server.h"

#include "connection.h"
#include "outputmessage.h"
#include "textlogger.h"
#include "scheduler.h"

#include "configmanager.h"
#include "tools.h"

extern ConfigManager g_config;

bool ServicePort::m_logError = true;

void ServicePort::services(boost::weak_ptr<ServicePort> weakService, IPAddressList ips, uint16_t port)
{
	if(weakService.expired())
		return;

	if(ServicePort_ptr service = weakService.lock())
		service->open(ips, port);
}

void ServicePort::service(boost::weak_ptr<ServicePort> weakService, IPAddress ip, uint16_t port)
{
	if(weakService.expired())
		return;

	ServicePort_ptr service = weakService.lock();
	if(!service)
		return;

	IPAddressList ips;
	ips.push_back(ip);
	service->open(ips, port);
}

bool ServicePort::add(Service_ptr newService)
{
	for(ServiceVec::const_iterator it = m_services.begin(); it != m_services.end(); ++it)
	{
		if((*it)->isSingleSocket())
			return false;
	}

	m_services.push_back(newService);
	return true;
}

void ServicePort::open(IPAddressList ips, uint16_t port)
{
	m_pendingStart = false;
	m_serverPort = port;

	bool error = false;
	IPAddressList pendingIps;
	for(IPAddressList::iterator it = ips.begin(); it != ips.end(); ++it)
	{
		try
		{
			Acceptor_ptr tmp(new boost::asio::ip::tcp::acceptor(m_io_service,
				boost::asio::ip::tcp::endpoint(*it, m_serverPort)));
			tmp->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
			tmp->set_option(boost::asio::ip::tcp::no_delay(true));

			accept(tmp);
			m_acceptors[tmp] = *it;
		}
		catch(std::exception& e)
		{
			if(m_logError)
			{
				LOG_MESSAGE(LOGTYPE_ERROR, e.what(), "NETWORK")
				pendingIps.push_back(*it);
				if(!error)
					error = true;
			}
		}
	}

	if(error)
	{
		m_logError = false;
		m_pendingStart = true;
		Scheduler::getInstance().addEvent(createSchedulerTask(5000, boost::bind(
			&ServicePort::services, boost::weak_ptr<ServicePort>(shared_from_this()), pendingIps, m_serverPort)));
	}
}

void ServicePort::close()
{
	if(!m_acceptors.size())
		return;

	for(AcceptorVec::iterator it = m_acceptors.begin(); it != m_acceptors.end(); ++it)
	{
		if(it->first->is_open())
			continue;

		boost::system::error_code error;
		it->first->close(error);
		if(error)
		{
			PRINT_ASIO_ERROR("Closing listen socket");
		}
	}

	m_acceptors.clear();
}

void ServicePort::accept(Acceptor_ptr acceptor)
{
	try
	{
		boost::asio::ip::tcp::socket* socket = new boost::asio::ip::tcp::socket(m_io_service);
		acceptor->async_accept(*socket, boost::bind(
			&ServicePort::handle, this, acceptor, socket, boost::asio::placeholders::error));
	}
	catch(std::exception& e)
	{
		if(m_logError)
		{
			LOG_MESSAGE(LOGTYPE_ERROR, e.what(), "NETWORK")
			m_logError = false;
		}
	}
}

void ServicePort::handle(Acceptor_ptr acceptor, boost::asio::ip::tcp::socket* socket, const boost::system::error_code& error)
{
	if(!error)
	{
		if(m_services.empty())
		{
#ifdef __DEBUG_NET__
			std::clog << "[Error - ServerPort::handle] No services running!" << std::endl;
#endif
			return;
		}

		boost::system::error_code error;
		const boost::asio::ip::tcp::endpoint ip = socket->remote_endpoint(error);

		uint32_t remoteIp = 0;
		if(!error)
			remoteIp = htonl(ip.address().to_v4().to_ulong());

		Connection_ptr connection;
		if(remoteIp && ConnectionManager::getInstance()->acceptConnection(remoteIp) &&
			(connection = ConnectionManager::getInstance()->createConnection(
			socket, m_io_service, shared_from_this())))
		{
			if(m_services.front()->isSingleSocket())
				connection->handle(m_services.front()->makeProtocol(connection));
			else
				connection->accept();
		}
		else if(socket->is_open())
		{
			boost::system::error_code error;
			socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);

			socket->close(error);
			delete socket;
		}

#ifdef __DEBUG_NET_DETAIL__
		std::clog << "handle - OK" << std::endl;
#endif
		accept(acceptor);
	}
	else if(error != boost::asio::error::operation_aborted)
	{
		PRINT_ASIO_ERROR("Handling");
		close();
		if(!m_pendingStart)
		{
			m_pendingStart = true;
			Scheduler::getInstance().addEvent(createSchedulerTask(5000, boost::bind(
				&ServicePort::service, boost::weak_ptr<ServicePort>(shared_from_this()),
				m_acceptors[acceptor], m_serverPort)));
		}
	}
#ifdef __DEBUG_NET__
	else
		std::clog << "[Error - ServerPort::handle] Operation aborted." << std::endl;
#endif
}

std::string ServicePort::getProtocolNames() const
{
	if(m_services.empty())
		return std::string();

	std::string str = m_services.front()->getProtocolName();
	for(int32_t i = 1, j = m_services.size(); i < j; ++i)
	{
		str += ", ";
		str += m_services[i]->getProtocolName();
	}

	return str;
}

Protocol* ServicePort::makeProtocol(bool checksum, NetworkMessage& msg) const
{
	uint8_t protocolId = msg.get<char>();
	for(ServiceVec::const_iterator it = m_services.begin(); it != m_services.end(); ++it)
	{
		if((*it)->getProtocolId() == protocolId && ((checksum && (*it)->hasChecksum()) || !(*it)->hasChecksum()))
			return (*it)->makeProtocol(Connection_ptr());
	}

	return NULL;
}

void ServiceManager::run()
{
	assert(!running);
	try
	{
		std::vector<boost::shared_ptr<boost::thread> > threads;
		for(uint32_t i = 0; i < g_config.getNumber(ConfigManager::SERVICE_THREADS); ++i)
		{
			boost::shared_ptr<boost::thread> thread(new boost::thread(
				boost::bind(&boost::asio::io_service::run, &m_io_service)));
			threads.push_back(thread);
		}

		running = true;
		for(std::vector<boost::shared_ptr<boost::thread> >::const_iterator it = threads.begin(); it != threads.end(); ++it)
			(*it)->join();
	}
	catch(std::exception& e)
	{
		LOG_MESSAGE(LOGTYPE_ERROR, e.what(), "NETWORK")
	}
}

void ServiceManager::stop()
{
	if(!running)
		return;

	running = false;
	for(AcceptorsMap::iterator it = m_acceptors.begin(); it != m_acceptors.end(); ++it)
	{
		try
		{
			m_io_service.post(boost::bind(&ServicePort::close, it->second));
		}
		catch(std::exception& e)
		{
			LOG_MESSAGE(LOGTYPE_ERROR, e.what(), "NETWORK")
		}
	}

	m_acceptors.clear();
	OutputMessagePool::getInstance()->stop();

	deathTimer.expires_from_now(boost::posix_time::seconds(3));
	deathTimer.async_wait(boost::bind(&ServiceManager::die, this));
}

std::list<uint16_t> ServiceManager::getPorts() const
{
	std::list<uint16_t> ports;
	for(AcceptorsMap::const_iterator it = m_acceptors.begin(); it != m_acceptors.end(); ++it)
		ports.push_back(it->first);

	ports.unique();
	return ports;
}
