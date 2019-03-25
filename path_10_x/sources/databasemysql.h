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

#ifndef __DATABASEMYSQL__
#define __DATABASEMYSQL__

#ifdef __USE_MYSQL__
#ifndef __DATABASE__
#error "database.h should be included first."
#endif

#ifdef _MSC_VER
#include <mysql.h>
#else
#include <mysql/mysql.h>
#endif

#if defined WINDOWS
#include <winsock2.h>
#endif
#include <sstream>
#include <map>

class DatabaseMySQL : public _Database
{
	public:
		DatabaseMySQL():
			m_handle(new MYSQL), m_timeoutTask(0) {}
		DATABASE_VIRTUAL ~DatabaseMySQL();

		DATABASE_VIRTUAL bool connect();
		DATABASE_VIRTUAL bool isMultiLine() {return true;}

		DATABASE_VIRTUAL bool beginTransaction() {return query("BEGIN");}
		DATABASE_VIRTUAL bool rollback();
		DATABASE_VIRTUAL bool commit();

		DATABASE_VIRTUAL bool query(std::string query);
		DATABASE_VIRTUAL DBResult* storeQuery(std::string query);

		DATABASE_VIRTUAL std::string escapeString(std::string s) {return escapeBlob(s.c_str(), s.length());}
		DATABASE_VIRTUAL std::string escapeBlob(const char* s, uint32_t length);

		DATABASE_VIRTUAL uint64_t getLastInsertId() {return (uint64_t)mysql_insert_id(m_handle);}
		DATABASE_VIRTUAL DatabaseEngine_t getDatabaseEngine() {return DATABASE_ENGINE_MYSQL;}

	protected:
		DATABASE_VIRTUAL void keepAlive();

		MYSQL* m_handle;
		uint32_t m_timeoutTask;
};

class MySQLResult : public _DBResult
{
	friend class DatabaseMySQL;
	public:
		DATABASE_VIRTUAL int32_t getDataInt(const std::string& s);
		DATABASE_VIRTUAL int64_t getDataLong(const std::string& s);
		DATABASE_VIRTUAL std::string getDataString(const std::string& s);
		DATABASE_VIRTUAL const char* getDataStream(const std::string& s, uint64_t& size);

		DATABASE_VIRTUAL void free();
		DATABASE_VIRTUAL bool next();

	protected:
		MySQLResult(MYSQL_RES* result);
		DATABASE_VIRTUAL ~MySQLResult();

		typedef std::map<const std::string, uint32_t> listNames_t;
		listNames_t m_listNames;

		MYSQL_RES* m_handle;
		MYSQL_ROW m_row;
};
#endif
#endif
