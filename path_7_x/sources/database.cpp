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

DBInsert::DBInsert(Database* db)
{
	m_db = db;
	m_rows = 0;
	// checks if current database engine supports multiline INSERTs
	m_multiLine = m_db->isMultiLine();
}

void DBInsert::setQuery(std::string query)
{
	m_query = query;
	m_buf = "";
	m_rows = 0;
}

bool DBInsert::addRow(std::string row)
{
	if(!m_multiLine) // executes INSERT for current row
		return m_db->query(m_query + "(" + row + ")");

	m_rows++;
	int32_t size = m_buf.length();
	// adds new row to buffer
	if(!size)
		m_buf = "(" + row + ")";
	else if(size > 8192)
	{
		if(!execute())
			return false;

		m_buf = "(" + row + ")";
	}
	else
		m_buf += ",(" + row + ")";

	return true;
}

bool DBInsert::addRow(std::ostringstream& row)
{
	bool ret = addRow(row.str());
	row.str("");
	return ret;
}

bool DBInsert::execute()
{
	if(!m_multiLine || m_buf.length() < 1 || !m_rows) // INSERTs were executed on-fly or there's no rows to execute
		return true;

	// executes buffer
	bool ret = m_db->query(m_query + m_buf);
	m_rows = 0;
	m_buf = "";
	return ret;
}
