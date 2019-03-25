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
#include <string>

#include "database.h"
#ifdef __USE_MYSQL__
#include "databasemysql.h"
#endif
#ifdef __USE_SQLITE__
#include "databasesqlite.h"
#endif
#ifdef __USE_PGSQL__
#include "databasepgsql.h"
#endif

#if defined MULTI_SQL_DRIVERS
#include "configmanager.h"
extern ConfigManager g_config;
#endif

boost::recursive_mutex DBQuery::databaseLock;
Database* _Database::m_instance = NULL;

Database* _Database::getInstance()
{
	if(!m_instance)
	{
#if defined MULTI_SQL_DRIVERS
#ifdef __USE_MYSQL__
		if(g_config.getString(ConfigManager::SQL_TYPE) == "mysql")
			m_instance = new DatabaseMySQL;
#endif
#ifdef __USE_MYSQLPP__
		else if(g_config.getString(ConfigManager::SQL_TYPE) == "mysql++")
			m_instance = new DatabaseMySQLpp;
#endif
#ifdef __USE_SQLITE__
		else if(g_config.getString(ConfigManager::SQL_TYPE) == "sqlite")
			m_instance = new DatabaseSQLite;
#endif
#ifdef __USE_PGSQL__
		else if(g_config.getString(ConfigManager::SQL_TYPE) == "pgsql")
			m_instance = new DatabasePgSQL;
#endif
#else
		m_instance = new Database;
#endif
	}

	m_instance->use();
	return m_instance;
}

DBResult* _Database::verifyResult(DBResult* result)
{
	if(result->next())
		return result;

	result->free();
	return NULL;
}

void DBInsert::setQuery(std::string query)
{
	m_query = query;
	m_buf = "";
	m_rows = 0;
}

bool DBInsert::addRow(std::string row)
{
	if(!m_db->multiLine())
		return m_db->query(m_query + "(" + row + ")");

	++m_rows;
	if(m_buf.empty())
		m_buf = "(" + row + ")";
	else if(m_buf.length() > 8192)
	{
		if(!execute())
			return false;

		m_buf = "(" + row + ")";
	}
	else
		m_buf += ",(" + row + ")";

	return true;
}

bool DBInsert::addRow(std::stringstream& row)
{
	bool ret = addRow(row.str());
	row.str("");
	return ret;
}

bool DBInsert::execute()
{
	if(!m_db->multiLine() || m_buf.empty() || !m_rows)
		return true;

	m_rows = 0;
	bool ret = m_db->query(m_query + m_buf);
	m_buf = "";
	return ret;
}
