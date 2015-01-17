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

#ifndef __DATABASE_PGSQL__
#define __DATABASE_PGSQL__

#ifdef __USE_PGSQL__
#ifndef __DATABASE__
#error "database.h should be included first."
#endif
#include "libpq-fe.h"

class DatabasePgSQL : public _Database
{
	public:
		DatabasePgSQL();
		DATABASE_VIRTUAL ~DatabasePgSQL() {PQfinish(m_handle);}

		DATABASE_VIRTUAL bool multiLine() const {return true;}

		DATABASE_VIRTUAL bool beginTransaction() {return query("BEGIN");}
		DATABASE_VIRTUAL bool rollback() {return query("ROLLBACK");}
		DATABASE_VIRTUAL bool commit() {return query("COMMIT");}

		DATABASE_VIRTUAL bool query(std::string query);
		DATABASE_VIRTUAL DBResult* storeQuery(std::string query);

		DATABASE_VIRTUAL std::string escapeString(std::string s);
		DATABASE_VIRTUAL std::string escapeBlob(const char *s, uint32_t length);

		DATABASE_VIRTUAL uint64_t getLastInsertId();
		DATABASE_VIRTUAL DatabaseEngine_t getDatabaseEngine() {return DATABASE_ENGINE_POSTGRESQL;}

	protected:
		std::string _parse(const std::string& s);
		PGconn* m_handle;
};

class PgSQLResult : public _DBResult
{
	friend class DatabasePgSQL;

	public:
		DATABASE_VIRTUAL int32_t getDataInt(const std::string& s) {return atoi(
			PQgetvalue(m_handle, m_cursor, PQfnumber(m_handle, s.c_str())));}
		DATABASE_VIRTUAL int64_t getDataLong(const std::string& s) {return atoll(
			PQgetvalue(m_handle, m_cursor, PQfnumber(m_handle, s.c_str())));}
		DATABASE_VIRTUAL std::string getDataString(const std::string& s) {return std::string(
			PQgetvalue(m_handle, m_cursor, PQfnumber(m_handle, s.c_str())));}
		DATABASE_VIRTUAL const char* getDataStream(const std::string& s, uint64_t& size);

		DATABASE_VIRTUAL void free();
		DATABASE_VIRTUAL bool next();

	protected:
		PgSQLResult(PGresult* results);
		DATABASE_VIRTUAL ~PgSQLResult();

		PGresult* m_handle;
		int32_t m_rows, m_cursor;
};
#endif
#endif 