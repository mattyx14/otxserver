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
#ifdef __USE_SQLITE__
#include <iostream>
#include <boost/regex.hpp>

#include "database.h"
#include "databasesqlite.h"

#include "tools.h"
#include "configmanager.h"

extern ConfigManager g_config;

#if SQLITE_VERSION_NUMBER < 3003009
#define sqlite3_prepare_v2 sqlite3_prepare
#endif

DatabaseSQLite::DatabaseSQLite() :
	m_handle(NULL)
{
	// test for existence of database file;
	// sqlite3_open will create a new one if it isn't there (what we don't want)
	if(!fileExists(g_config.getString(ConfigManager::SQL_FILE).c_str()))
		return;

	// Initialize sqlite
	if(sqlite3_open(g_config.getString(ConfigManager::SQL_FILE).c_str(), &m_handle) != SQLITE_OK)
	{
		std::clog << "Failed to initialize SQLite connection: " << sqlite3_errmsg(m_handle) << " (" << sqlite3_errcode(m_handle) << ")" << std::endl;
		sqlite3_close(m_handle);
	}
	else
		m_connected = true;
}

std::string DatabaseSQLite::_parse(const std::string& s)
{
	std::string query = "";
	query.reserve(s.size());

	bool inString = false;
	for(uint32_t i = 0; i < s.length(); ++i)
	{
		uint8_t ch = s[i];
		if(ch == '\'')
		{
			if(inString && s[i + 1] != '\'')
				inString = false;
			else
				inString = true;
		}

		if(ch == '`' && !inString)
			ch = '"';

		query += ch;
	}

	return query;
}

bool DatabaseSQLite::query(std::string query)
{
	boost::recursive_mutex::scoped_lock lockClass(sqliteLock);
	if(!m_connected)
		return false;

	std::string buf = _parse(query);
#ifdef __SQL_QUERY_DEBUG__
	std::clog << "SQLLITE DEBUG, query: " << buf << std::endl;
#endif
	sqlite3_stmt* stmt;
	// prepares statement
	if(sqlite3_prepare_v2(m_handle, buf.c_str(), buf.length(), &stmt, NULL) != SQLITE_OK)
	{
		sqlite3_finalize(stmt);
		std::clog << "sqlite3_prepare_v2(): SQLITE ERROR: " << sqlite3_errmsg(m_handle)  << " (" << buf << ")" << std::endl;
		return false;
	}

	// executes it once
	int32_t ret = sqlite3_step(stmt);
	if(ret != SQLITE_OK && ret != SQLITE_DONE && ret != SQLITE_ROW)
	{
		sqlite3_finalize(stmt);
		std::clog << "sqlite3_step(): SQLITE ERROR: " << sqlite3_errmsg(m_handle) << std::endl;
		return false;
	}

	// closes statement
	// at all not sure if it should be debugged - query was executed correctly...
	sqlite3_finalize(stmt);
	return true;
}

DBResult* DatabaseSQLite::storeQuery(std::string query)
{
	boost::recursive_mutex::scoped_lock lockClass(sqliteLock);
	if(!m_connected)
		return NULL;

	std::string buf = _parse(query);
#ifdef __SQL_QUERY_DEBUG__
	std::clog << "SQLLITE DEBUG, storeQuery: " << buf << std::endl;
#endif
	sqlite3_stmt* stmt;
	// prepares statement
	if(sqlite3_prepare_v2(m_handle, buf.c_str(), buf.length(), &stmt, NULL) != SQLITE_OK)
	{
		sqlite3_finalize(stmt);
		std::clog << "sqlite3_prepare_v2(): SQLITE ERROR: " << sqlite3_errmsg(m_handle)  << " (" << buf << ")" << std::endl;
		return NULL;
	}

	DBResult* result = new SQLiteResult(stmt);
	return verifyResult(result);
}

std::string DatabaseSQLite::escapeString(std::string s)
{
	// remember about quoiting even an empty string!
	if(!s.size())
		return std::string("''");

	// the worst case is 2n + 3
	char* output = new char[(s.length() << 1) + 3];
	// quotes escaped string and frees temporary buffer
	sqlite3_snprintf((s.length() << 1) + 1, output, "%Q", s.c_str());

	std::string r(output);
	delete[] output;

	//escape % and _ because we are using LIKE operator.
	r = boost::regex_replace(r, boost::regex("%"), "\\%");
	r = boost::regex_replace(r, boost::regex("_"), "\\_");
	if(r[r.length() - 1] != '\'')
		r += "'";

	return r;
}

std::string DatabaseSQLite::escapeBlob(const char* s, uint32_t length)
{
	std::string buf = "x'";
	char* hex = new char[2 + 1]; //need one extra byte for null-character
	for(uint32_t i = 0; i < length; ++i)
	{
		sprintf(hex, "%02x", ((uint8_t)s[i]));
		buf += hex;
	}

	delete[] hex;
	buf += "'";
	return buf;
}

int32_t SQLiteResult::getDataInt(const std::string& s)
{
	listNames_t::iterator it = m_listNames.find(s);
	if(it != m_listNames.end())
		return sqlite3_column_int(m_handle, it->second);

	std::clog << "Error during getDataInt(" << s << ")." << std::endl;
	return 0; // Failed
}

int64_t SQLiteResult::getDataLong(const std::string& s)
{
	listNames_t::iterator it = m_listNames.find(s);
	if(it != m_listNames.end())
		return sqlite3_column_int64(m_handle, it->second);

	std::clog << "Error during getDataLong(" << s << ")." << std::endl;
	return 0; // Failed
}

std::string SQLiteResult::getDataString(const std::string& s)
{
	listNames_t::iterator it = m_listNames.find(s);
	if(it != m_listNames.end() )
	{
		std::string value = (const char*)sqlite3_column_text(m_handle, it->second);
		return value;
	}

	std::clog << "Error during getDataString(" << s << ")." << std::endl;
	return std::string(""); // Failed
}

const char* SQLiteResult::getDataStream(const std::string& s, uint64_t& size)
{
	listNames_t::iterator it = m_listNames.find(s);
	if(it != m_listNames.end())
	{
		const char* value = (const char*)sqlite3_column_blob(m_handle, it->second);
		size = sqlite3_column_bytes(m_handle, it->second);
		return value;
	}

	std::clog << "Error during getDataStream(" << s << ")." << std::endl;
	return NULL; // Failed
}

void SQLiteResult::free()
{
	if(!m_handle)
	{
		std::clog << "[Critical - SQLiteResult::free] Trying to free already freed result!!!" << std::endl;
		return;
	}

	sqlite3_finalize(m_handle);
	m_handle = NULL;

	m_listNames.clear();
	delete this;
}

SQLiteResult::~SQLiteResult()
{
	if(!m_handle)
		return;

	sqlite3_finalize(m_handle);
	m_listNames.clear();
}

SQLiteResult::SQLiteResult(sqlite3_stmt* stmt)
{
	if(!stmt)
		return;

	m_handle = stmt;
	int32_t fields = sqlite3_column_count(m_handle);
	for(int32_t i = 0; i < fields; ++i)
		m_listNames[sqlite3_column_name(m_handle, i)] = i;
}
#endif
