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

#ifndef __DATABASE_MANAGER__
#define __DATABASE_MANAGER__
#include "database.h"

class DatabaseManager
{
	public:
		DatabaseManager() {}
		virtual ~DatabaseManager() {}

		static DatabaseManager* getInstance()
		{
			static DatabaseManager instance;
			return &instance;
		}

		bool tableExists(std::string table);
		bool triggerExists(std::string trigger);

		int32_t getDatabaseVersion();
		bool isDatabaseSetup();

		bool optimizeTables();
		uint32_t updateDatabase();

		bool getDatabaseConfig(std::string config, int32_t &value);
		void registerDatabaseConfig(std::string config, int32_t value);

		bool getDatabaseConfig(std::string config, std::string &value);
		void registerDatabaseConfig(std::string config, std::string value);

		void checkEncryption();
		void checkTriggers();
};
#endif

