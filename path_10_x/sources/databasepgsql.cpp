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
#ifdef __USE_PGSQL__
#include <iostream>

#include "database.h"
#include "databasepgsql.h"

#include "configmanager.h"
extern ConfigManager g_config;

DatabasePgSQL::DatabasePgSQL() :
	m_handle(NULL)
{
	std::stringstream dns;
	dns << "host='" << g_config.getString(ConfigManager::SQL_HOST) << "' dbname='" << g_config.getString(ConfigManager::SQL_DB) << "' user='" << g_config.getString(ConfigManager::SQL_USER) << "' password='" << g_config.getString(ConfigManager::SQL_PASS) << "' port='" << g_config.getNumber(ConfigManager::SQL_PORT) << "'";

	m_handle = PQconnectdb(dns.str().c_str());
	if(PQstatus(m_handle) != CONNECTION_OK)
	{
		std::clog << "Failed to estabilish PostgreSQL database connection: " << PQerrorMessage(m_handle) << std::endl;
		PQfinish(m_handle);
	}
	else
		m_connected = true;
}

std::string DatabasePgSQL::_parse(const std::string& s)
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

bool DatabasePgSQL::query(std::string query)
{
	if(!m_connected)
		return false;

	// executes query
	PGresult* res = PQexec(m_handle, _parse(query).c_str());
	ExecStatusType stat = PQresultStatus(res);
	if(stat != PGRES_COMMAND_OK && stat != PGRES_TUPLES_OK)
	{
		std::clog << "PQexec(): " << query << ": " << PQresultErrorMessage(res) << std::endl;
		PQclear(res);
		return false;
	}

	// everything went fine
	PQclear(res);
	return true;
}

DBResult* DatabasePgSQL::storeQuery(std::string query)
{
	if(!m_connected)
		return NULL;

	// executes query
	PGresult* res = PQexec(m_handle, _parse(query).c_str());
	ExecStatusType stat = PQresultStatus(res);
	if(stat != PGRES_COMMAND_OK && stat != PGRES_TUPLES_OK)
	{
		std::clog << "PQexec(): " << query << ": " << PQresultErrorMessage(res) << std::endl;
		PQclear(res);
		return false;
	}

	// everything went fine
	DBResult* result = new PgSQLResult(res);
	return verifyResult(result);
}

std::string DatabasePgSQL::escapeString(std::string s)
{
	// remember to quote even empty string!
	if(!m_connected || !s.size())
		return std::string("''");

	// the worst case is 2n + 1
	int32_t error;
	char* output = new char[(s.length() * 2) + 1];
	// quotes escaped string and frees temporary buffer
	PQescapeStringConn(m_handle, output, s.c_str(), s.length(), reinterpret_cast<int32_t*>(&error));

	std::string r = std::string("'");
	r += output;
	r += "'";

	delete[] output;
	return r;
}

std::string DatabasePgSQL::escapeBlob(const char *s, uint32_t length)
{
	// remember to quote even empty stream!
	if(!m_connected || !s)
		return std::string("''");

	// quotes escaped string and frees temporary buffer
	size_t len;
	char* output = (char*)PQescapeByteaConn(m_handle, (uint8_t*)s, length, &len);

	std::string r = std::string("E'");
	r += output;
	r += "'";

	PQfreemem(output);
	return r;
}

uint64_t DatabasePgSQL::getLastInsertId()
{
	if(!m_connected)
		return 0;

	PGresult* res = PQexec(m_handle, "SELECT LASTVAL() as last;");
	ExecStatusType stat = PQresultStatus(res);
	if(stat != PGRES_COMMAND_OK && stat != PGRES_TUPLES_OK)
	{
		std::clog << "PQexec(): \"SELECT LASTVAL() as last\": " << PQresultErrorMessage(res) << std::endl;
		PQclear(res);
		return 0;
	}

	const uint64_t id = atoll(PQgetvalue(res, 0, PQfnumber(res, "last")));
	PQclear(res);
	return id;
}

const char* PgSQLResult::getDataStream(const std::string& s, uint64_t& size)
{
	std::string buf = PQgetvalue(m_handle, m_cursor, PQfnumber(m_handle, s.c_str()));
	uint8_t* temp = PQunescapeBytea( (const uint8_t*)buf.c_str(), (size_t*)&size);

	char* value = new char[buf.size()];
	strcpy(value, (char*)temp);

	PQfreemem(temp);
	return value;
}

void PgSQLResult::free()
{
	if(!m_handle)
	{
		std::clog << "[Critical - PgSQLResult::free] Trying to free already freed result!!!" << std::endl;
		return;
	}

	PQclear(m_handle);
	m_handle = NULL;
	delete this;
}

bool PgSQLResult::next()
{
	if(m_cursor >= m_rows)
		return false;

	m_cursor++;
	return true;
}

PgSQLResult::~PgSQLResult()
{
	if(m_handle)
		PQclear(m_handle);
}

PgSQLResult::PgSQLResult(PGresult* result)
{
	if(!result)
		return;

	m_handle = result;
	m_rows = PQntuples(m_handle) - 1;
	m_cursor = -1;
}
#endif