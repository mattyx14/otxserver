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

#ifndef __DATABASEMYSQLPP__
#define __DATABASEMYSQLPP__

#ifdef __USE_MYSQLPP__
#ifndef __DATABASE__
#error "database.h should be included first."
#endif

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include <cppconn/statement.h>

#include <sstream>
#include <map>

class DatabaseMySQLpp : public _Database
{
	public:
		DatabaseMySQLpp();
		DATABASE_VIRTUAL ~DatabaseMySQLpp();

		DATABASE_VIRTUAL bool connect(bool _reconnect);
		DATABASE_VIRTUAL bool multiLine() const {return true;}

		DATABASE_VIRTUAL bool beginTransaction();
		DATABASE_VIRTUAL bool rollback();
		DATABASE_VIRTUAL bool commit();

		DATABASE_VIRTUAL bool query(std::string query);
		DATABASE_VIRTUAL DBResult* storeQuery(std::string query);

		DATABASE_VIRTUAL std::string escapeString(std::string s) {return escapeBlob(s.c_str(), s.length());}
		DATABASE_VIRTUAL std::string escapeBlob(const char* s, uint32_t length);

		DATABASE_VIRTUAL uint64_t getLastInsertId();
		DATABASE_VIRTUAL DatabaseEngine_t getDatabaseEngine() {return DATABASE_ENGINE_MYSQL;}

	protected:
		sql::mysql::MySQL_Driver* m_driver;
		sql::mysql::MySQL_Connection* m_connection;
};

class MySQLppResult : public _DBResult
{
	friend class DatabaseMySQLpp;
	public:
		DATABASE_VIRTUAL int32_t getDataInt(const std::string& s);
		DATABASE_VIRTUAL int64_t getDataLong(const std::string& s);
		DATABASE_VIRTUAL std::string getDataString(const std::string& s);
		DATABASE_VIRTUAL const char* getDataStream(const std::string& s, uint64_t& size);

		DATABASE_VIRTUAL void free();
		DATABASE_VIRTUAL bool next();

	protected:
		MySQLppResult(sql::ResultSet* result);
		DATABASE_VIRTUAL ~MySQLppResult();

		sql::ResultSet* m_result;
};
#endif
#endif
