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
#ifdef __USE_MYSQL__
#include <iostream>

#include "database.h"
#include "databasemysql.h"

#include "scheduler.h"
#include "configmanager.h"
#include "tools.h"

#ifdef _MSC_VER
#include <errmsg.h>
#else
#include <mysql/errmsg.h>
#endif

extern ConfigManager g_config;

DatabaseMySQL::DatabaseMySQL() :
	m_handle(new MYSQL), m_attempts(0), m_timeoutTask(0)
{
	assert(connect(false));
	int32_t timeout = g_config.getNumber(ConfigManager::SQL_KEEPALIVE) * 1000;
	if(timeout)
		m_timeoutTask = Scheduler::getInstance().addEvent(createSchedulerTask(timeout,
			boost::bind(&DatabaseMySQL::keepAlive, this)));

	if(asLowerCaseString(g_config.getString(ConfigManager::HOUSE_STORAGE)) == "relational")
		return;

	//we cannot lock mutex here :)
	DBResult* result = storeQuery("SHOW variables LIKE 'max_allowed_packet';");
	assert(result);
	if(result->getDataLong("Value") < 16776192)
		std::clog << std::endl << "> WARNING: max_allowed_packet might be set too low for binary map storage." << std::endl
			<< "Use the following query to raise max_allow_packet: SET GLOBAL max_allowed_packet = 16776192;" << std::endl;

	result->free();
}

DatabaseMySQL::~DatabaseMySQL()
{
	mysql_close(m_handle);
	delete m_handle;
	if(m_timeoutTask != 0)
		Scheduler::getInstance().stopEvent(m_timeoutTask);
}

bool DatabaseMySQL::connect(bool _reconnect)
{
	m_connected = false;
	if(_reconnect)
	{
		uint32_t attempts = g_config.getNumber(ConfigManager::MYSQL_RECONNECTION_ATTEMPTS);
		if(attempts != 0 && m_attempts > attempts)
			return false;

		std::clog << "> WARNING: MYSQL Lost connection, attempting to reconnect...";
		if(attempts != 0 && ++m_attempts > attempts)
		{
			std::clog << std::endl << "Failed connection to database - maximum reconnect attempts passed." << std::endl;
			return false;
		}
	}

	if(!mysql_init(m_handle))
	{
		std::clog << std::endl << "Failed to initialize MySQL connection handler." << std::endl;
		return false;
	}

	int32_t timeout = g_config.getNumber(ConfigManager::MYSQL_READ_TIMEOUT);
	if(timeout)
		mysql_options(m_handle, MYSQL_OPT_READ_TIMEOUT, (const char*)&timeout);

	timeout = g_config.getNumber(ConfigManager::MYSQL_WRITE_TIMEOUT);
	if(timeout)
		mysql_options(m_handle, MYSQL_OPT_WRITE_TIMEOUT, (const char*)&timeout);

	if(!mysql_real_connect(m_handle,
			g_config.getString(ConfigManager::SQL_HOST).c_str(),
			g_config.getString(ConfigManager::SQL_USER).c_str(),
			g_config.getString(ConfigManager::SQL_PASS).c_str(),
			g_config.getString(ConfigManager::SQL_DB).c_str(),
			g_config.getNumber(ConfigManager::SQL_PORT),
		NULL, 0))
	{
		std::clog << std::endl << "Failed connecting to database - MYSQL ERROR: " << mysql_error(m_handle) << " (" << mysql_errno(m_handle) << ")" << std::endl;
		return false;
	}

	m_connected = true;
	m_attempts = 0;
	return true;
}

bool DatabaseMySQL::rollback()
{
	if(!m_connected)
		return false;

	if(mysql_rollback(m_handle))
	{
		std::clog << "mysql_rollback() - MYSQL ERROR: " << mysql_error(m_handle) << " (" << mysql_errno(m_handle) << ")" << std::endl;
		return false;
	}

	return true;
}

bool DatabaseMySQL::commit()
{
	if(!m_connected)
		return false;

	if(mysql_commit(m_handle))
	{
		std::clog << "mysql_commit() - MYSQL ERROR: " << mysql_error(m_handle) << " (" << mysql_errno(m_handle) << ")" << std::endl;
		return false;
	}

	return true;
}

bool DatabaseMySQL::query(std::string query)
{
	if(!m_connected && !connect(true))
		return false;

	bool result = true;
#ifdef __SQL_QUERY_DEBUG__
	std::clog << "MYSQL DEBUG, query: " << query.c_str() << std::endl;
#endif
	if(mysql_real_query(m_handle, query.c_str(), query.length()))
	{
		int32_t error = mysql_errno(m_handle);
		if(error == CR_SERVER_LOST || error == CR_SERVER_GONE_ERROR)
			m_connected = false;

		std::clog << "mysql_real_query(): " << query << " - MYSQL ERROR: " << mysql_error(m_handle) << " (" << error << ")" << std::endl;
		result = false;
	}

	if(MYSQL_RES* tmp = mysql_store_result(m_handle))
		mysql_free_result(tmp);

	return result;
}

DBResult* DatabaseMySQL::storeQuery(std::string query)
{
	if(!m_connected && !connect(true))
		return NULL;

	int32_t error = 0;
#ifdef __SQL_QUERY_DEBUG__
	std::clog << "MYSQL DEBUG, storeQuery: " << query.c_str() << std::endl;
#endif
	if(mysql_real_query(m_handle, query.c_str(), query.length()))
	{
		error = mysql_errno(m_handle);
		if(error == CR_SERVER_LOST || error == CR_SERVER_GONE_ERROR)
			m_connected = false;

		std::clog << "mysql_real_query(): " << query << " - MYSQL ERROR: " << mysql_error(m_handle) << " (" << error << ")" << std::endl;
		return NULL;
	}

	if(MYSQL_RES* _result = mysql_store_result(m_handle))
	{
		DBResult* result = (DBResult*)new MySQLResult(_result);
		return verifyResult(result);
	}

	error = mysql_errno(m_handle);
	if(error == CR_UNKNOWN_ERROR || error == CR_SERVER_LOST)
		m_connected = false;

	std::clog << "mysql_store_result(): " << query << " - MYSQL ERROR: " << mysql_error(m_handle) << " (" << error << ")" << std::endl;
	return NULL;
}

std::string DatabaseMySQL::escapeBlob(const char* s, uint32_t length)
{
	if(!m_connected || *s == '\0')
		return "''";

	char* output = new char[(length << 1) + 1];
	mysql_real_escape_string(m_handle, output, s, length);

	std::string res = "'";
	res += output;
	res += "'";

	delete[] output;
	return res;
}

void DatabaseMySQL::keepAlive()
{
	int32_t timeout = g_config.getNumber(ConfigManager::SQL_KEEPALIVE) * 1000;
	if(timeout != 0 && OTSYS_TIME() > m_use + timeout && mysql_ping(m_handle))
		connect(true);

	Scheduler::getInstance().addEvent(createSchedulerTask(timeout,
		boost::bind(&DatabaseMySQL::keepAlive, this)));
}

int32_t MySQLResult::getDataInt(const std::string& s)
{
	listNames_t::iterator it = m_listNames.find(s);
	if(it != m_listNames.end())
		return m_row[it->second] ? atoi(m_row[it->second]) : 0;

	std::clog << "Error during getDataInt(" << s << ")." << std::endl;
	return 0; // Failed
}

int64_t MySQLResult::getDataLong(const std::string& s)
{
	listNames_t::iterator it = m_listNames.find(s);
	if(it != m_listNames.end())
		return m_row[it->second] ? atoll(m_row[it->second]) : 0;

	std::clog << "Error during getDataLong(" << s << ")." << std::endl;
	return 0; // Failed
}

std::string MySQLResult::getDataString(const std::string& s)
{
	listNames_t::iterator it = m_listNames.find(s);
	if(it != m_listNames.end())
		return m_row[it->second] ? std::string(m_row[it->second]) : std::string();

	std::clog << "Error during getDataString(" << s << ")." << std::endl;
	return std::string(); // Failed
}

const char* MySQLResult::getDataStream(const std::string& s, uint64_t& size)
{
	size = 0;
	listNames_t::iterator it = m_listNames.find(s);
	if(it == m_listNames.end())
	{
		std::clog << "Error during getDataStream(" << s << ")." << std::endl;
		return NULL; // Failed
	}

	if(!m_row[it->second])
		return NULL;

	size = mysql_fetch_lengths(m_handle)[it->second];
	return m_row[it->second];
}

void MySQLResult::free()
{
	if(!m_handle)
	{
		std::clog << "[Critical - MySQLResult::free] Trying to free already freed result!!!" << std::endl;
		return;
	}

	mysql_free_result(m_handle);
	m_handle = NULL;
	delete this;
}

bool MySQLResult::next()
{
	m_row = mysql_fetch_row(m_handle);
	return (m_row != NULL);
}

MySQLResult::~MySQLResult()
{
	if(m_handle)
		mysql_free_result(m_handle);
}

MySQLResult::MySQLResult(MYSQL_RES* result)
{
	if(!result)
		return;

	m_handle = result;
	int32_t i = 0;

	MYSQL_FIELD* field = NULL;
	while((field = mysql_fetch_field(m_handle)))
		m_listNames[field->name] = i++;
}
#endif
