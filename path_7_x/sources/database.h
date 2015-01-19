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

#ifndef __DATABASE__
#define __DATABASE__
#include "otsystem.h"

#include "enums.h"
#include <sstream>

#ifdef MULTI_SQL_DRIVERS
#define DATABASE_VIRTUAL virtual
#define DATABASE_CLASS _Database
#define RESULT_CLASS _DBResult
class _Database;
class _DBResult;
#else
#define DATABASE_VIRTUAL

#if defined(__USE_MYSQL__)
#define DATABASE_CLASS DatabaseMySQL
#define RESULT_CLASS MySQLResult
class DatabaseMySQL;
class MySQLResult;

#elif defined(__USE_MYSQLPP__)
#define DATABASE_CLASS DatabaseMySQLpp
#define RESULT_CLASS MySQLppResult
class DatabaseMySQLpp;
class MySQLppResult;

#elif defined(__USE_SQLITE__)
#define DATABASE_CLASS DatabaseSQLite
#define RESULT_CLASS SQLiteResult
class DatabaseSQLite;
class SQLiteResult;

#elif defined(__USE_PGSQL__)
#define DATABASE_CLASS DatabasePgSQL
#define RESULT_CLASS PgSQLResult
class DatabasePgSQL;
class PgSQLResult;

#endif
#endif

#ifndef DATABASE_CLASS
#error "You have to compile with at least one database driver!"
#define DBResult void
#define DBInsert void*
#define Database void
#else
typedef DATABASE_CLASS Database;
typedef RESULT_CLASS DBResult;

enum DBParam_t
{
	DBPARAM_MULTIINSERT = 1
};

class _Database
{
	public:
		friend class DBTransaction;

		/**
		* Singleton implementation.
		*
		* Retruns instance of database handler. Don't create database (or drivers) instances in your code - instead of it use Database::getInstance()-> This method stores static instance of connection class internaly to make sure exacly one instance of connection is created for entire system.
		*
		* @return database connection handler singletor
		*/
		static Database* getInstance();

		/**
		* Database information.
		*
		* Returns currently used database attribute.
		*
		* @param DBParam_t parameter to get
		* @return suitable for given parameter
		*/
		DATABASE_VIRTUAL bool multiLine() const {return false;}

		/**
		* Database connected.
		*
		* Returns whether or not the database is connected.
		*
		* @return whether or not the database is connected.
		*/
		DATABASE_VIRTUAL bool isConnected() const {return m_connected;}

		/**
		* Database ...
		*/
		DATABASE_VIRTUAL void use() {m_use = OTSYS_TIME();}

		/**
		* Transaction related methods.
		*
		* Methods for starting, commiting and rolling back transaction. Each of the returns boolean value.
		*
		* @return true on success, false on error
		* @note
		*	If your database system doesn't support transactions you should return true - it's not feature test, code should work without transaction, just will lack integrity.
		*/

		DATABASE_VIRTUAL bool beginTransaction() {return false;}
		DATABASE_VIRTUAL bool rollback() {return false;}
		DATABASE_VIRTUAL bool commit() {return false;}

		/**
		* Executes command.
		*
		* Executes query which doesn't generates results (eg. INSERT, UPDATE, DELETE...).
		*
		* @param std::string query command
		* @return true on success, false on error
		*/
		DATABASE_VIRTUAL bool query(std::string) {return false;}

		/**
		* Queries database.
		*
		* Executes query which generates results (mostly SELECT).
		*
		* @param std::string query
		* @return results object (null on error)
		*/
		DATABASE_VIRTUAL DBResult* storeQuery(std::string) {return NULL;}

		/**
		* Escapes string for query.
		*
		* Prepares string to fit SQL queries including quoting it.
		*
		* @param std::string string to be escaped
		* @return quoted string
		*/
		DATABASE_VIRTUAL std::string escapeString(std::string) {return "''";}

		/**
		* Escapes binary stream for query.
		*
		* Prepares binary stream to fit SQL queries.
		*
		* @param char* binary stream
		* @param long stream length
		* @return quoted string
		*/
		DATABASE_VIRTUAL std::string escapeBlob(const char*, uint32_t) {return "''";}

		/**
		 * Retrieve id of last inserted row
		 *
		 * @return id on success, 0 if last query did not result on any rows with auto_increment keys
		 */
		DATABASE_VIRTUAL uint64_t getLastInsertId() {return 0;}

		/**
		* Get case insensitive string comparison operator
		*
		* @return the case insensitive operator
		*/
		DATABASE_VIRTUAL std::string getStringComparer() {return "= ";}
		DATABASE_VIRTUAL std::string getUpdateLimiter() {return " LIMIT 1;";}

		/**
		* Get database engine
		*
		* @return the database engine type
		*/
		DATABASE_VIRTUAL DatabaseEngine_t getDatabaseEngine() {return DATABASE_ENGINE_NONE;}

	protected:
		DBResult* verifyResult(DBResult* result);

		_Database(): m_connected(false) {}
		DATABASE_VIRTUAL ~_Database() {m_connected = false;}

		bool m_connected;
		int64_t m_use;

	private:
		static Database* m_instance;
};

class _DBResult
{
	public:
		/** Get the Integer value of a field in database
		*\returns The Integer value of the selected field and row
		*\param s The name of the field
		*/
		DATABASE_VIRTUAL int32_t getDataInt(const std::string&) {return 0;}

		/** Get the Long value of a field in database
		*\returns The Long value of the selected field and row
		*\param s The name of the field
		*/
		DATABASE_VIRTUAL int64_t getDataLong(const std::string&) {return 0;}

		/** Get the String of a field in database
		*\returns The String of the selected field and row
		*\param s The name of the field
		*/
		DATABASE_VIRTUAL std::string getDataString(const std::string&) {return "";}

		/** Get the blob of a field in database
		*\returns a PropStream that is initiated with the blob data field, if not exist it returns NULL.
		*\param s The name of the field
		*/
		DATABASE_VIRTUAL const char* getDataStream(const std::string&, uint64_t&) {return "";}

		/** Result freeing
		*/
		DATABASE_VIRTUAL void free() {}

		/** Moves to next result in set
		*\returns true if moved, false if there are no more results.
		*/
		DATABASE_VIRTUAL bool next() {return false;}

	protected:
		_DBResult() {}
		DATABASE_VIRTUAL ~_DBResult() {}
};

/**
 * Thread locking hack.
 *
 * By using this class for your queries you lock and unlock database for threads.
*/
class DBQuery : public std::stringstream
{
	friend class _Database;
	public:
		DBQuery() {databaseLock.lock();}
		~DBQuery() {databaseLock.unlock();}

	protected:
		static boost::recursive_mutex databaseLock;
};

/**
 * INSERT statement.
 *
 * Gives possibility to optimize multiple INSERTs on databases that support multiline INSERTs.
 */
class DBInsert
{
	public:
		/**
		* Associates with given database handler.
		*
		* @param Database* database wrapper
		*/
		DBInsert(Database* db): m_db(db), m_rows(0) {}
		~DBInsert() {}

		/**
		* Sets query prototype.
		*
		* @param std::string& INSERT query
		*/
		void setQuery(std::string query);

		/**
		* Adds new row to INSERT statement.
		*
		* On databases that doesn't support multiline INSERTs it simply execute INSERT for each row.
		*
		* @param std::string& row data
		*/
		bool addRow(std::string row);
		/**
		* Allows to use addRow() with stringstream as parameter.
		*/
		bool addRow(std::stringstream& row);

		/**
		* Executes current buffer.
		*/
		bool execute();

	protected:
		Database* m_db;

		uint32_t m_rows;
		std::string m_query, m_buf;
};


#ifndef MULTI_SQL_DRIVERS
#if defined(__USE_MYSQL__)
#include "databasemysql.h"
#elif defined(__USE_MYSQLPP__)
#include "databasemysqlpp.h"
#elif defined(__USE_SQLITE__)
#include "databasesqlite.h"
#elif defined(__USE_PGSQL__)
#include "databasepgsql.h"
#endif
#endif

class DBTransaction
{
	public:
		DBTransaction(Database* database)
		{
			m_db = database;
			m_state = STATE_FRESH;
		}

		~DBTransaction()
		{
			if(m_state == STATE_READY)
				m_db->rollback();
		}

		bool begin()
		{
			m_state = STATE_READY;
			return m_db->beginTransaction();
		}

		bool commit()
		{
			if(m_state != STATE_READY)
				return false;

			m_state = STATE_DONE;
			return m_db->commit();
		}

	private:
		Database* m_db;
		enum TransactionStates_t
		{
			STATE_FRESH,
			STATE_READY,
			STATE_DONE
		} m_state;
};
#endif
#endif
