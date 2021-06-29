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
#include "enums.h"
#include <iostream>

#include "databasemanager.h"
#include "tools.h"

#include "configmanager.h"
extern ConfigManager g_config;

bool DatabaseManager::optimizeTables()
{
	Database* db = Database::getInstance();
	std::ostringstream query;
	switch(db->getDatabaseEngine())
	{
		case DATABASE_ENGINE_MYSQL:
		{
			query << "SELECT `TABLE_NAME` FROM `information_schema`.`tables` WHERE `TABLE_SCHEMA` = " << db->escapeString(g_config.getString(ConfigManager::SQL_DB)) << " AND `DATA_FREE` > 0;";
			DBResult* result;
			if(!(result = db->storeQuery(query.str())))
				break;

			query.str("");
			do
			{
				std::clog << "> Optimizing table: " << result->getDataString("TABLE_NAME") << "... ";
				query << "OPTIMIZE TABLE `" << result->getDataString("TABLE_NAME") << "`;";
				if(db->query(query.str()))
					std::clog << "[success]" << std::endl;
				else
					std::clog << "[failure]" << std::endl;

				query.str("");
			}
			while(result->next());

			result->free();
			return true;
		}

		case DATABASE_ENGINE_SQLITE:
		{
			if(!db->query("VACUUM;"))
				break;

			std::clog << ">>> Optimized database ... (done)." << std::endl;
			return true;
		}

		case DATABASE_ENGINE_POSTGRESQL:
		{
			if(!db->query("VACUUM FULL;"))
				break;

			std::clog << ">>> Optimized database ... (done)." << std::endl;
			return true;
		}

		default:
			break;
	}

	return false;
}

bool DatabaseManager::triggerExists(std::string trigger)
{
	Database* db = Database::getInstance();
	std::ostringstream query;
	switch(db->getDatabaseEngine())
	{
		case DATABASE_ENGINE_SQLITE:
			query << "SELECT `name` FROM `sqlite_master` WHERE `type` = 'trigger' AND `name` = " << db->escapeString(trigger) << ";";
			break;

		case DATABASE_ENGINE_MYSQL:
			query << "SELECT `TRIGGER_NAME` FROM `information_schema`.`triggers` WHERE `TRIGGER_SCHEMA` = " << db->escapeString(g_config.getString(ConfigManager::SQL_DB)) << " AND `TRIGGER_NAME` = " << db->escapeString(trigger) << ";";
			break;

		case DATABASE_ENGINE_POSTGRESQL:
			//TODO: PostgreSQL support
			return true;

		default:
			return false;
	}

	DBResult* result;
	if(!(result = db->storeQuery(query.str())))
		return false;

	result->free();
	return true;
}

bool DatabaseManager::tableExists(std::string table)
{
	Database* db = Database::getInstance();
	std::ostringstream query;
	switch(db->getDatabaseEngine())
	{
		case DATABASE_ENGINE_SQLITE:
			query << "SELECT `name` FROM `sqlite_master` WHERE `type` = 'table' AND `name` = " << db->escapeString(table) << ";";
			break;

		case DATABASE_ENGINE_MYSQL:
			query << "SELECT `TABLE_NAME` FROM `information_schema`.`tables` WHERE `TABLE_SCHEMA` = " << db->escapeString(g_config.getString(ConfigManager::SQL_DB)) << " AND `TABLE_NAME` = " << db->escapeString(table) << ";";
			break;

		case DATABASE_ENGINE_POSTGRESQL:
			//TODO: PostgreSQL support
			return true;

		default:
			return false;
	}

	DBResult* result;
	if(!(result = db->storeQuery(query.str())))
		return false;

	result->free();
	return true;
}

bool DatabaseManager::isDatabaseSetup()
{
	Database* db = Database::getInstance();
	switch(db->getDatabaseEngine())
	{
		case DATABASE_ENGINE_MYSQL:
		{
			std::ostringstream query;
			query << "SELECT `TABLE_NAME` FROM `information_schema`.`tables` WHERE `TABLE_SCHEMA` = " << db->escapeString(g_config.getString(ConfigManager::SQL_DB)) << ";";

			DBResult* result;
			if(!(result = db->storeQuery(query.str())))
				return false;

			result->free();
			return true;
		}

		case DATABASE_ENGINE_SQLITE:
			//a pre-setup sqlite database is already included
			return true;

		case DATABASE_ENGINE_POSTGRESQL:
			//TODO: PostgreSQL support
			return true;

		default:
			break;
	}

	return false;
}

int32_t DatabaseManager::getDatabaseVersion()
{
	if(!tableExists("server_config"))
		return 0;

	int32_t value = 0;
	if(getDatabaseConfig("db_version", value))
		return value;

	return -1;
}

uint32_t DatabaseManager::updateDatabase()
{
	Database* db = Database::getInstance();
	int32_t version = getDatabaseVersion();
	if(version < 0)
		return 0;

	if(version < 6 && db->getDatabaseEngine() == DATABASE_ENGINE_POSTGRESQL)
	{
		std::clog << "> WARNING: Couldn't update database - PostgreSQL support available since version 6, please use latest pgsql.sql schema." << std::endl;
		registerDatabaseConfig("db_version", 6);
		return 6;
	}

	std::ostringstream query;
	switch(version)
	{
		case 0:
		{
			std::clog << "> Updating database to version: 1..." << std::endl;
			if(db->getDatabaseEngine() == DATABASE_ENGINE_SQLITE)
				db->query("CREATE TABLE IF NOT EXISTS `server_config` (`config` VARCHAR(35) NOT NULL DEFAULT '', `value` INTEGER NOT NULL);");
			else
				db->query("CREATE TABLE IF NOT EXISTS `server_config` (`config` VARCHAR(35) NOT NULL DEFAULT '', `value` INT NOT NULL) ENGINE = InnoDB;");

			db->query("INSERT INTO `server_config` VALUES ('db_version', 1);");
			if(db->getDatabaseEngine() == DATABASE_ENGINE_SQLITE)
			{
				//TODO: 0.2 migration SQLite support
				std::cerr << "> SQLite migration from 0.2 support not available, please use latest database!" << std::endl;
				return 1;
			}

			if(!tableExists("server_motd"))
			{
				//update bans table
				if(db->query("CREATE TABLE IF NOT EXISTS `bans2` (`id` INT UNSIGNED NOT NULL auto_increment, `type` TINYINT(1) NOT NULL COMMENT 'this field defines if its ip, account, player, or any else ban', `value` INT UNSIGNED NOT NULL COMMENT 'ip, player guid, account number', `param` INT UNSIGNED NOT NULL DEFAULT 4294967295 COMMENT 'mask', `active` TINYINT(1) NOT NULL DEFAULT TRUE, `expires` INT UNSIGNED NOT NULL, `added` INT UNSIGNED NOT NULL, `admin_id` INT UNSIGNED NOT NULL DEFAULT 0, `comment` TEXT NOT NULL, `reason` INT UNSIGNED NOT NULL DEFAULT 0, `action` INT UNSIGNED NOT NULL DEFAULT 0, PRIMARY KEY (`id`), KEY `type` (`type`, `value`)) ENGINE = InnoDB;"))
				{
					query << "SELECT * FROM `bans`;";
					if(DBResult* result = db->storeQuery(query.str()))
					{
						do
						{
							switch(result->getDataInt("type"))
							{
								case 1:
									query << "INSERT INTO `bans2` (`type`, `value`, `param`, `active`, `expires`, `added`, `admin_id`, `comment`, `reason`, `action`) VALUES (1, " << result->getDataInt("ip") << ", " << result->getDataInt("mask") << ", " << (result->getDataInt("time") <= time(NULL) ? 0 : 1) << ", " << result->getDataInt("time") << ", 0, " << result->getDataInt("banned_by") << ", " << db->escapeString(result->getDataString("comment")) << ", " << result->getDataInt("reason_id") << ", " << result->getDataInt("action_id") << ");";
									break;

								case 2:
									query << "INSERT INTO `bans2` (`type`, `value`, `active`, `expires`, `added`, `admin_id`, `comment`, `reason`, `action`) VALUES (2, " << result->getDataInt("player") << ", " << (result->getDataInt("time") <= time(NULL) ? 0 : 1) << ", 0, " << result->getDataInt("time") << ", " << result->getDataInt("banned_by") << ", " << db->escapeString(result->getDataString("comment")) << ", " << result->getDataInt("reason_id") << ", " << result->getDataInt("action_id") << ");";
									break;

								case 3:
									query << "INSERT INTO `bans2` (`type`, `value`, `active`, `expires`, `added`, `admin_id`, `comment`, `reason`, `action`) VALUES (3, " << result->getDataInt("player") << ", " << (result->getDataInt("time") <= time(NULL) ? 0 : 1) << ", " << result->getDataInt("time") << ", 0, " << result->getDataInt("banned_by") << ", " << db->escapeString(result->getDataString("comment")) << ", " << result->getDataInt("reason_id") << ", " << result->getDataInt("action_id") << ");";
									break;

								case 4:
								case 5:
									query << "INSERT INTO `bans2` (`type`, `value`, `active`, `expires`, `added`, `admin_id`, `comment`, `reason`, `action`) VALUES (" << result->getDataInt("type") << ", " << result->getDataInt("player") << ", " << (result->getDataInt("time") <= time(NULL) ? 0 : 1) << ", 0, " << result->getDataInt("time") << ", " << result->getDataInt("banned_by") << ", " << db->escapeString(result->getDataString("comment")) << ", " << result->getDataInt("reason_id") << ", " << result->getDataInt("action_id") << ");";
									break;

								default:
									break;
							}

							if(query.str() != "")
							{
								db->query(query.str());
								query.str("");
							}
						}
						while(result->next());
						result->free();
					}

					db->query("DROP TABLE `bans`;");
					db->query("RENAME TABLE `bans2` TO `bans`;");
				}

				std::string queryList[] = {
					//create server_record table
					"CREATE TABLE IF NOT EXISTS `server_record` (`record` INT NOT NULL, `timestamp` BIGINT NOT NULL, PRIMARY KEY (`timestamp`)) ENGINE = InnoDB;",
					//create server_reports table
					"CREATE TABLE IF NOT EXISTS `server_reports` (`id` INT NOT NULL AUTO_INCREMENT, `player_id` INT UNSIGNED NOT NULL DEFAULT 0, `posx` INT NOT NULL DEFAULT 0, `posy` INT NOT NULL DEFAULT 0, `posz` INT NOT NULL DEFAULT 0, `timestamp` BIGINT NOT NULL DEFAULT 0, `report` TEXT NOT NULL, `reads` INT NOT NULL DEFAULT 0, PRIMARY KEY (`id`), KEY (`player_id`), KEY (`reads`)) ENGINE = InnoDB;",
					//create server_motd table
					"CREATE TABLE `server_motd` (`id` INT NOT NULL AUTO_INCREMENT, `text` TEXT NOT NULL, PRIMARY KEY (`id`)) ENGINE = InnoDB;",
					//create global_storage table
					"CREATE TABLE IF NOT EXISTS `global_storage` (`key` INT UNSIGNED NOT NULL, `value` INT NOT NULL, PRIMARY KEY (`key`)) ENGINE = InnoDB;",
					//insert data to server_record table
					"INSERT INTO `server_record` VALUES (0, 0);",
					//insert data to server_motd table
					"INSERT INTO `server_motd` VALUES (1, 'Welcome to The OTX Server!');",
					//update players table
					"ALTER TABLE `players` ADD `balance` BIGINT UNSIGNED NOT NULL DEFAULT 0 AFTER `blessings`;",
					"ALTER TABLE `players` ADD `stamina` BIGINT UNSIGNED NOT NULL DEFAULT 201660000 AFTER `balance`;",
					"ALTER TABLE `players` ADD `loss_items` INT NOT NULL DEFAULT 10 AFTER `loss_skills`;",
					"ALTER TABLE `players` ADD `marriage` INT UNSIGNED NOT NULL DEFAULT 0;",
					"UPDATE `players` SET `loss_experience` = 10, `loss_mana` = 10, `loss_skills` = 10, `loss_items` = 10;",
					//update accounts table
					"ALTER TABLE `accounts` DROP `type`;",
					//update player deaths table
					"ALTER TABLE `player_deaths` DROP `is_player`;",
					//update house table
					"ALTER TABLE `houses` CHANGE `warnings` `warnings` INT NOT NULL DEFAULT 0;",
					"ALTER TABLE `houses` ADD `lastwarning` INT UNSIGNED NOT NULL DEFAULT 0;",
					//update triggers
					"DROP TRIGGER IF EXISTS `ondelete_accounts`;",
					"DROP TRIGGER IF EXISTS `ondelete_players`;",
					"CREATE TRIGGER `ondelete_accounts` BEFORE DELETE ON `accounts` FOR EACH ROW BEGIN DELETE FROM `bans` WHERE `type` != 1 AND `type` != 2 AND `value` = OLD.`id`; END;",
					"CREATE TRIGGER `ondelete_players` BEFORE DELETE ON `players` FOR EACH ROW BEGIN DELETE FROM `bans` WHERE `type` = 2 AND `value` = OLD.`id`; UPDATE `houses` SET `owner` = 0 WHERE `owner` = OLD.`id`; END;"
				};
				for(uint32_t i = 0; i < sizeof(queryList) / sizeof(std::string); ++i)
					db->query(queryList[i]);
			}

			return 1;
		}

		case 1:
		{
			std::clog << "> Updating database to version: 2..." << std::endl;
			if(db->getDatabaseEngine() == DATABASE_ENGINE_SQLITE)
				db->query("ALTER TABLE `players` ADD `promotion` INTEGER NOT NULL DEFAULT 0;");
			else
				db->query("ALTER TABLE `players` ADD `promotion` INT NOT NULL DEFAULT 0;");

			DBResult* result;
			query << "SELECT `player_id`, `value` FROM `player_storage` WHERE `key` = 30018 AND `value` > 0";
			if((result = db->storeQuery(query.str())))
			{
				do
				{
					query << "UPDATE `players` SET `promotion` = " << result->getDataLong("value") << " WHERE `id` = " << result->getDataInt("player_id") << ";";
					db->query(query.str());
					query.str("");
				}
				while(result->next());
				result->free();
			}

			db->query("DELETE FROM `player_storage` WHERE `key` = 30018;");
			db->query("ALTER TABLE `accounts` ADD `name` VARCHAR(32) NOT NULL DEFAULT '';");

			query << "SELECT `id` FROM `accounts`;";
			if((result = db->storeQuery(query.str())))
			{
				do
				{
					query << "UPDATE `accounts` SET `name` = '" << result->getDataInt("id") << "' WHERE `id` = " << result->getDataInt("id") << ";";
					db->query(query.str());
					query.str("");
				}
				while(result->next());
				result->free();
			}

			if(db->getDatabaseEngine() == DATABASE_ENGINE_SQLITE)
				db->query("ALTER TABLE `players` ADD `deleted` INTEGER NOT NULL DEFAULT 0;");
			else
				db->query("ALTER TABLE `players` ADD `deleted` INT NOT NULL DEFAULT 0;");

			if(db->getDatabaseEngine() == DATABASE_ENGINE_MYSQL)
				db->query("ALTER TABLE `player_items` CHANGE `attributes` `attributes` BLOB NOT NULL;");

			registerDatabaseConfig("db_version", 2);
			return 2;
		}

		case 2:
		{
			std::clog << "> Updating database to version: 3..." << std::endl;
			db->query("UPDATE `players` SET `vocation` = `vocation` - 4 WHERE `vocation` >= 5 AND `vocation` <= 8;");

			DBResult* result;
			query << "SELECT COUNT(`id`) AS `count` FROM `players` WHERE `vocation` > 4;";
			if((result = db->storeQuery(query.str()))
				&& result->getDataInt("count"))
			{
				std::clog << "[Warning] There are still " << result->getDataInt("count") << " players with vocation above 4, please mind to update them manually." << std::endl;
				result->free();
			}

			registerDatabaseConfig("db_version", 3);
			return 3;
		}

		case 3:
		{
			std::clog << "> Updating database to version: 4..." << std::endl;
			db->query("ALTER TABLE `houses` ADD `name` VARCHAR(255) NOT NULL;");
			if(db->getDatabaseEngine() == DATABASE_ENGINE_SQLITE)
			{
				std::string queryList[] = {
					"ALTER TABLE `houses` ADD `size` INTEGER NOT NULL DEFAULT 0;",
					"ALTER TABLE `houses` ADD `town` INTEGER NOT NULL DEFAULT 0;",
					"ALTER TABLE `houses` ADD `price` INTEGER NOT NULL DEFAULT 0;",
					"ALTER TABLE `houses` ADD `rent` INTEGER NOT NULL DEFAULT 0;"
				};
				for(uint32_t i = 0; i < sizeof(queryList) / sizeof(std::string); ++i)
					db->query(queryList[i]);
			}
			else
			{
				std::string queryList[] = {
					"ALTER TABLE `houses` ADD `size` INT UNSIGNED NOT NULL DEFAULT 0;",
					"ALTER TABLE `houses` ADD `town` INT UNSIGNED NOT NULL DEFAULT 0;",
					"ALTER TABLE `houses` ADD `price` INT UNSIGNED NOT NULL DEFAULT 0;",
					"ALTER TABLE `houses` ADD `rent` INT UNSIGNED NOT NULL DEFAULT 0;"
				};
				for(uint32_t i = 0; i < sizeof(queryList) / sizeof(std::string); ++i)
					db->query(queryList[i]);
			}

			registerDatabaseConfig("db_version", 4);
			return 4;
		}

		case 4:
		{
			std::clog << "> Updating database to version: 5..." << std::endl;
			db->query("ALTER TABLE `player_deaths` ADD `altkilled_by` VARCHAR(255) NOT NULL;");
			registerDatabaseConfig("db_version", 5);
			return 5;
		}

		case 5:
		{
			std::clog << "> Updating database to version: 6..." << std::endl;
			switch(db->getDatabaseEngine())
			{
				case DATABASE_ENGINE_MYSQL:
				{
					std::string queryList[] = {
						"ALTER TABLE `global_storage` CHANGE `value` `value` VARCHAR(255) NOT NULL DEFAULT '0'",
						"ALTER TABLE `player_storage` CHANGE `value` `value` VARCHAR(255) NOT NULL DEFAULT '0'",
						"ALTER TABLE `tiles` CHANGE `x` `x` INT(5) UNSIGNED NOT NULL, CHANGE `y` `y` INT(5) UNSIGNED NOT NULL, CHANGE `z` `z` TINYINT(2) UNSIGNED NOT NULL;",
						"ALTER TABLE `tiles` ADD INDEX (`x`, `y`, `z`);",
						"ALTER TABLE `tile_items` ADD INDEX (`sid`);"
					};
					for(uint32_t i = 0; i < sizeof(queryList) / sizeof(std::string); ++i)
						db->query(queryList[i]);

					break;
				}

				case DATABASE_ENGINE_SQLITE:
				{
					std::string queryList[] = {
						"ALTER TABLE `global_storage` RENAME TO `global_storage2`;",
						"CREATE TABLE `global_storage` (`key` INTEGER NOT NULL, `value` VARCHAR(255) NOT NULL DEFAULT '0', UNIQUE (`key`));",
						"INSERT INTO `global_storage` SELECT * FROM `global_storage2`;",
						"ALTER TABLE `player_storage` RENAME TO `player_storage2`;",
						"CREATE TABLE `player_storage` (`player_id` INTEGER NOT NULL, `key` INTEGER NOT NULL, `value` VARCHAR(255) NOT NULL DEFAULT '0', UNIQUE (`player_id`, `key`), FOREIGN KEY (`player_id`) REFERENCES `players` (`id`));",
						"INSERT INTO `player_storage` SELECT * FROM `player_storage2`;"
					};
					for(uint32_t i = 0; i < sizeof(queryList) / sizeof(std::string); ++i)
						db->query(queryList[i]);

					break;
				}

				default:
					break;
			}

			registerDatabaseConfig("db_version", 6);
			return 6;
		}

		case 6:
		{
			std::clog << "> Updating database to version: 7..." << std::endl;
			if(g_config.getBool(ConfigManager::INGAME_GUILD_MANAGEMENT))
			{
				query << "SELECT `r`.`id`, `r`.`guild_id` FROM `guild_ranks` r LEFT JOIN `guilds` g ON `r`.`guild_id` = `g`.`id` WHERE `g`.`ownerid` = `g`.`id` AND `r`.`level` = 3;";
				if(DBResult* result = db->storeQuery(query.str()))
				{
					do
					{
						query << "UPDATE `guilds`, `players` SET `guilds`.`ownerid` = `players`.`id` WHERE `guilds`.`id` = " << result->getDataInt("guild_id") << " AND `players`.`rank_id` = " << result->getDataInt("id") << ";";
						db->query(query.str());
						query.str("");
					}
					while(result->next());
					result->free();
				}
			}

			registerDatabaseConfig("db_version", 7);
			return 7;
		}

		case 7:
		{
			std::clog << "> Updating database version to: 8..." << std::endl;
			switch(db->getDatabaseEngine())
			{
				case DATABASE_ENGINE_MYSQL:
				{
					std::string queryList[] = {
						"ALTER TABLE `server_motd` CHANGE `id` `id` INT UNSIGNED NOT NULL;",
						"ALTER TABLE `server_motd` DROP PRIMARY KEY;",
						"ALTER TABLE `server_motd` ADD `world_id` TINYINT(2) UNSIGNED NOT NULL DEFAULT 0;",
						"ALTER TABLE `server_motd` ADD UNIQUE (`id`, `world_id`);",
						"ALTER TABLE `server_record` DROP PRIMARY KEY;",
						"ALTER TABLE `server_record` ADD `world_id` TINYINT(2) UNSIGNED NOT NULL DEFAULT 0;",
						"ALTER TABLE `server_record` ADD UNIQUE (`timestamp`, `record`, `world_id`);",
						"ALTER TABLE `server_reports` ADD `world_id` TINYINT(2) UNSIGNED NOT NULL DEFAULT 0;",
						"ALTER TABLE `server_reports` ADD INDEX (`world_id`);",
						"ALTER TABLE `players` ADD `world_id` TINYINT(2) UNSIGNED NOT NULL DEFAULT 0;",
						"ALTER TABLE `global_storage` DROP PRIMARY KEY;",
						"ALTER TABLE `global_storage` ADD `world_id` TINYINT(2) UNSIGNED NOT NULL DEFAULT 0;",
						"ALTER TABLE `global_storage` ADD UNIQUE (`key`, `world_id`);",
						"ALTER TABLE `guilds` ADD `world_id` TINYINT(2) UNSIGNED NOT NULL DEFAULT 0;",
						"ALTER TABLE `guilds` ADD UNIQUE (`name`, `world_id`);",
						"ALTER TABLE `house_lists` ADD `world_id` TINYINT(2) UNSIGNED NOT NULL DEFAULT 0;",
						"ALTER TABLE `house_lists` ADD UNIQUE (`house_id`, `world_id`, `listid`);",
						"ALTER TABLE `houses` CHANGE `id` `id` INT NOT NULL;",
						"ALTER TABLE `houses` DROP PRIMARY KEY;",
						"ALTER TABLE `houses` ADD `world_id` TINYINT(2) UNSIGNED NOT NULL DEFAULT 0;",
						"ALTER TABLE `houses` ADD UNIQUE (`id`, `world_id`);",
						"ALTER TABLE `tiles` CHANGE `id` `id` INT NOT NULL;",
						"ALTER TABLE `tiles` ADD `world_id` TINYINT(2) UNSIGNED NOT NULL DEFAULT 0;",
						"ALTER TABLE `tiles` ADD UNIQUE (`id`, `world_id`);",
						"ALTER TABLE `tile_items` ADD `world_id` TINYINT(2) UNSIGNED NOT NULL DEFAULT 0;",
						"ALTER TABLE `tile_items` ADD UNIQUE (`tile_id`, `world_id`, `sid`);"
					};
					for(uint32_t i = 0; i < sizeof(queryList) / sizeof(std::string); ++i)
						db->query(queryList[i]);

					break;
				}

				case DATABASE_ENGINE_SQLITE:
				case DATABASE_ENGINE_POSTGRESQL:
				default:
				{
					//TODO
					break;
				}
			}

			registerDatabaseConfig("db_version", 8);
			return 8;
		}

		case 8:
		{
			std::clog << "> Updating database to version: 9..." << std::endl;
			db->query("ALTER TABLE `bans` ADD `statement` VARCHAR(255) NOT NULL;");
			registerDatabaseConfig("db_version", 9);
			return 9;
		}

		case 9:
		{
			std::clog << "> Updating database to version: 10..." << std::endl;
			registerDatabaseConfig("db_version", 10);
			return 10;
		}

		case 10:
		{
			std::clog << "> Updating database to version: 11..." << std::endl;
			db->query("ALTER TABLE `players` ADD `description` VARCHAR(255) NOT NULL DEFAULT '';");
			if(tableExists("map_storage"))
			{
				db->query("ALTER TABLE `map_storage` RENAME TO `house_data`;");
				db->query("ALTER TABLE `house_data` ADD `world_id` TINYINT(2) UNSIGNED NOT NULL DEFAULT 0 AFTER `house_id`;");
			}
			else if(!tableExists("house_storage"))
			{
				query << "CREATE TABLE `house_data` (`house_id` INT UNSIGNED NOT NULL, `world_id` TINYINT(2) UNSIGNED NOT NULL DEFAULT 0, `data` LONGBLOB NOT NULL, UNIQUE (`house_id`, `world_id`)";
				if(db->getDatabaseEngine() == DATABASE_ENGINE_SQLITE)
					query << ", FOREIGN KEY (`house_id`) REFERENCES `houses` (`id`) ON DELETE CASCADE";

				query << ")";
				if(db->getDatabaseEngine() == DATABASE_ENGINE_MYSQL)
					query << " ENGINE = InnoDB";

				query << ";";
				db->query(query.str());
				query.str("");
			}
			else
				db->query("ALTER TABLE `house_storage` RENAME TO `house_data`;");

			registerDatabaseConfig("db_version", 11);
			return 11;
		}

		case 11:
		{
			std::clog << "> Updating database to version: 12..." << std::endl;
			db->query("UPDATE `players` SET `stamina` = 151200000 WHERE `stamina` > 151200000;");
			db->query("UPDATE `players` SET `loss_experience` = `loss_experience` * 10, `loss_mana` = `loss_mana` * 10, `loss_skills` = `loss_skills` * 10, `loss_items` = `loss_items` * 10;");
			switch(db->getDatabaseEngine())
			{
				case DATABASE_ENGINE_MYSQL:
				{
					std::string queryList[] = {
						"ALTER TABLE `players` CHANGE `stamina` `stamina` INT NOT NULL DEFAULT 151200000;",
						"ALTER TABLE `players` CHANGE `loss_experience` `loss_experience` INT NOT NULL DEFAULT 100;",
						"ALTER TABLE `players` CHANGE `loss_mana` `loss_mana` INT NOT NULL DEFAULT 100;",
						"ALTER TABLE `players` CHANGE `loss_skills` `loss_skills` INT NOT NULL DEFAULT 100;",
						"ALTER TABLE `players` CHANGE `loss_items` `loss_items` INT NOT NULL DEFAULT 100;",
						"ALTER TABLE `players` ADD `loss_containers` INT NOT NULL DEFAULT 100 AFTER `loss_skills`;"
					};
					for(uint32_t i = 0; i < sizeof(queryList) / sizeof(std::string); ++i)
						db->query(queryList[i]);

					break;
				}

				case DATABASE_ENGINE_SQLITE:
				case DATABASE_ENGINE_POSTGRESQL:
				default:
				{
					//TODO
					break;
				}
			}

			registerDatabaseConfig("db_version", 12);
			return 12;
		}

		case 12:
		{
			std::clog << "> Updating database to version: 13..." << std::endl;
			std::string queryList[] = {
				"ALTER TABLE `accounts` DROP KEY `group_id`;",
				"ALTER TABLE `players` DROP KEY `group_id`;",
				"DROP TABLE `groups`;"
			};
			for(uint32_t i = 0; i < sizeof(queryList) / sizeof(std::string); ++i)
				db->query(queryList[i]);

			registerDatabaseConfig("db_version", 13);
			return 13;
		}

		case 13:
		{
			std::clog << "> Updating database to version: 14..." << std::endl;
			switch(db->getDatabaseEngine())
			{
				case DATABASE_ENGINE_MYSQL:
				{
					std::string queryList[] = {
						"ALTER TABLE `houses` ADD `doors` INT UNSIGNED NOT NULL DEFAULT 0;",
						"ALTER TABLE `houses` ADD `beds` INT UNSIGNED NOT NULL DEFAULT 0;",
						"ALTER TABLE `houses` ADD `guild` TINYINT(1) UNSIGNED NOT NULL DEFAULT FALSE;"
					};
					for(uint32_t i = 0; i < sizeof(queryList) / sizeof(std::string); ++i)
						db->query(queryList[i]);

					break;
				}

				case DATABASE_ENGINE_SQLITE:
				{
					std::string queryList[] = {
						"ALTER TABLE `houses` ADD `doors` INTEGER NOT NULL DEFAULT 0;",
						"ALTER TABLE `houses` ADD `beds` INTEGER NOT NULL DEFAULT 0;",
						"ALTER TABLE `houses` ADD `guild` BOOLEAN NOT NULL DEFAULT FALSE;"
					};
					for(uint32_t i = 0; i < sizeof(queryList) / sizeof(std::string); ++i)
						db->query(queryList[i]);

					break;
				}

				case DATABASE_ENGINE_POSTGRESQL:
				default:
				{
					//TODO
					break;
				}
			}

			registerDatabaseConfig("db_version", 14);
			return 14;
		}

		case 14:
		{
			std::clog << "> Updating database to version: 15..." << std::endl;
			db->query("DROP TABLE `player_deaths`;"); //no support for moving, sorry!
			switch(db->getDatabaseEngine())
			{
				case DATABASE_ENGINE_MYSQL:
				{
					std::string queryList[] = {
"CREATE TABLE `player_deaths`\
(\
	`id` INT NOT NULL AUTO_INCREMENT,\
	`player_id` INT NOT NULL,\
	`date` BIGINT UNSIGNED NOT NULL,\
	`level` INT UNSIGNED NOT NULL,\
	FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE,\
	PRIMARY KEY(`id`),\
	INDEX(`date`)\
) ENGINE = InnoDB;",
"CREATE TABLE `killers`\
(\
	`id` INT NOT NULL AUTO_INCREMENT,\
	`death_id` INT NOT NULL,\
	`final_hit` TINYINT(1) UNSIGNED NOT NULL DEFAULT FALSE,\
	PRIMARY KEY(`id`),\
	FOREIGN KEY (`death_id`) REFERENCES `player_deaths` (`id`) ON DELETE CASCADE\
) ENGINE = InnoDB;",
"CREATE TABLE `player_killers`\
(\
	`kill_id` INT NOT NULL,\
	`player_id` INT NOT NULL,\
	FOREIGN KEY (`kill_id`) REFERENCES `killers` (`id`) ON DELETE CASCADE,\
	FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE\
) ENGINE = InnoDB;",
"CREATE TABLE `environment_killers`\
(\
	`kill_id` INT NOT NULL,\
	`name` VARCHAR(255) NOT NULL,\
	FOREIGN KEY (`kill_id`) REFERENCES `killers` (`id`) ON DELETE CASCADE\
) ENGINE = InnoDB;"
					};
					for(uint32_t i = 0; i < sizeof(queryList) / sizeof(std::string); ++i)
						db->query(queryList[i]);

					break;
				}

				case DATABASE_ENGINE_SQLITE:
				{
					std::string queryList[] = {
"CREATE TABLE `player_deaths` (\
	`id` INTEGER PRIMARY KEY,\
	`player_id` INTEGER NOT NULL,\
	`date` INTEGER NOT NULL,\
	`level` INTEGER NOT NULL,\
	FOREIGN KEY (`player_id`) REFERENCES `players` (`id`)\
);",
"CREATE TABLE `killers` (\
	`id` INTEGER PRIMARY KEY,\
	`death_id` INTEGER NOT NULL,\
	`final_hit` BOOLEAN NOT NULL DEFAULT FALSE,\
	FOREIGN KEY (`death_id`) REFERENCES `player_deaths` (`id`)\
);",
"CREATE TABLE `player_killers` (\
	`kill_id` INTEGER NOT NULL,\
	`player_id` INTEGER NOT NULL,\
	FOREIGN KEY (`kill_id`) REFERENCES `killers` (`id`),\
	FOREIGN KEY (`player_id`) REFERENCES `players` (`id`)\
);",
"CREATE TABLE `environment_killers` (\
	`kill_id` INTEGER NOT NULL,\
	`name` VARCHAR(255) NOT NULL,\
	FOREIGN KEY (`kill_id`) REFERENCES `killers` (`id`)\
);"
					};
					for(uint32_t i = 0; i < sizeof(queryList) / sizeof(std::string); ++i)
						db->query(queryList[i]);

					break;
				}

				case DATABASE_ENGINE_POSTGRESQL:
				default:
				{
					//TODO
					break;
				}
			}

			registerDatabaseConfig("db_version", 15);
			return 15;
		}

		case 15:
		{
			std::clog << "> Updating database to version: 16..." << std::endl;
			switch(db->getDatabaseEngine())
			{
				case DATABASE_ENGINE_MYSQL:
				{
					std::string queryList[] = {
						"ALTER TABLE `players` DROP `redskull`;",
						"ALTER TABLE `players` CHANGE `redskulltime` `redskulltime` INT NOT NULL DEFAULT 0;",
						"ALTER TABLE `killers` ADD `unjustified` TINYINT(1) UNSIGNED NOT NULL DEFAULT FALSE;",
						"UPDATE `players` SET `redskulltime` = 0;"
					};
					for(uint32_t i = 0; i < sizeof(queryList) / sizeof(std::string); ++i)
						db->query(queryList[i]);

					break;
				}

				case DATABASE_ENGINE_SQLITE:
				{
					std::string queryList[] = {
						//we cannot DROP redskull, and redskulltime is already INTEGER
						"ALTER TABLE `killers` ADD `unjustified` BOOLEAN NOT NULL DEFAULT FALSE;",
						"UPDATE `players` SET `redskulltime` = 0;"
					};
					for(uint32_t i = 0; i < sizeof(queryList) / sizeof(std::string); ++i)
						db->query(queryList[i]);

					break;
				}

				case DATABASE_ENGINE_POSTGRESQL:
				default:
				{
					//TODO
					break;
				}
			}

			registerDatabaseConfig("db_version", 16);
			return 16;
		}

		case 16:
		{
			std::clog << "> Updating database to version: 17..." << std::endl;
			switch(db->getDatabaseEngine())
			{
				case DATABASE_ENGINE_MYSQL:
					db->query("CREATE TABLE IF NOT EXISTS `player_namelocks`\
(\
	`player_id` INT NOT NULL DEFAULT 0,\
	`name` VARCHAR(255) NOT NULL,\
	`new_name` VARCHAR(255) NOT NULL,\
	`date` BIGINT NOT NULL DEFAULT 0,\
	KEY (`player_id`),\
	FOREIGN KEY (`player_id`) REFERENCES `players`(`id`) ON DELETE CASCADE\
) ENGINE = InnoDB;");

					break;
				case DATABASE_ENGINE_SQLITE:
					db->query("CREATE TABLE IF NOT EXISTS `player_namelocks` (\
	`player_id` INTEGER NOT NULL,\
	`name` VARCHAR(255) NOT NULL,\
	`new_name` VARCHAR(255) NOT NULL,\
	`date` INTEGER NOT NULL DEFAULT 0,\
	FOREIGN KEY (`player_id`) REFERENCES `players` (`id`)\
);");
					break;
				default:
					//TODO
					break;
			}

			registerDatabaseConfig("db_version", 17);
			return 17;
		}

		case 17:
		{
			std::clog << "> Updating database to version: 18..." << std::endl;
			switch(db->getDatabaseEngine())
			{
				case DATABASE_ENGINE_MYSQL:
				{
					std::string queryList[] = {
						"ALTER TABLE `player_depotitems` DROP KEY `player_id`;",
						"ALTER TABLE `player_depotitems` DROP `depot_id`;",
						"ALTER TABLE `player_depotitems` ADD KEY (`player_id`);",
						"ALTER TABLE `house_data` ADD FOREIGN KEY (`house_id`, `world_id`) REFERENCES `houses`(`id`, `world_id`) ON DELETE CASCADE;",
						"ALTER TABLE `house_lists` ADD FOREIGN KEY (`house_id`, `world_id`) REFERENCES `houses`(`id`, `world_id`) ON DELETE CASCADE;",
						"ALTER TABLE `guild_invites` ADD FOREIGN KEY (`player_id`) REFERENCES `players`(`id`) ON DELETE CASCADE;",
						"ALTER TABLE `guild_invites` ADD FOREIGN KEY (`guild_id`) REFERENCES `guilds`(`id`) ON DELETE CASCADE;",
						"ALTER TABLE `tiles` ADD `house_id` INT UNSIGNED NOT NULL AFTER `world_id`;",
						"ALTER TABLE `tiles` ADD FOREIGN KEY (`house_id`, `world_id`) REFERENCES `houses`(`id`, `world_id`) ON DELETE CASCADE;",
						"ALTER TABLE `houses` ADD `clear` TINYINT(1) UNSIGNED NOT NULL DEFAULT FALSE;"
					};
					for(uint32_t i = 0; i < sizeof(queryList) / sizeof(std::string); ++i)
						db->query(queryList[i]);

					break;
				}

				case DATABASE_ENGINE_SQLITE:
				{
					std::string queryList[] = {
						"ALTER TABLE `house_data` ADD FOREIGN KEY (`house_id`, `world_id`) REFERENCES `houses`(`id`, `world_id`);",
						"ALTER TABLE `house_lists` ADD FOREIGN KEY (`house_id`, `world_id`) REFERENCES `houses`(`id`, `world_id`);",
						"ALTER TABLE `guild_invites` ADD FOREIGN KEY (`player_id`) REFERENCES `players`(`id`);",
						"ALTER TABLE `guild_invites` ADD FOREIGN KEY (`guild_id`) REFERENCES `guilds`(`id`);",
						"ALTER TABLE `tiles` ADD `house_id` INTEGER NOT NULL;",
						"ALTER TABLE `tiles` ADD FOREIGN KEY (`house_id`, `world_id`) REFERENCES `houses`(`id`, `world_id`);",
						"ALTER TABLE `houses` ADD `clear` BOOLEAN NOT NULL DEFAULT FALSE;"
					};
					for(uint32_t i = 0; i < sizeof(queryList) / sizeof(std::string); ++i)
						db->query(queryList[i]);

					break;
				}

				default:
					//TODO
					break;
			}

			registerDatabaseConfig("db_version", 18);
			return 18;
		}

		case 18:
		{
			std::clog << "> Updating database to version: 19..." << std::endl;
			switch(db->getDatabaseEngine())
			{
				case DATABASE_ENGINE_MYSQL:
				{
					std::string queryList[] = {
						"ALTER TABLE `houses` ADD `tiles` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `beds`;",
						"CREATE TABLE `house_auctions`\
(\
	`house_id` INT UNSIGNED NOT NULL,\
	`world_id` TINYINT(2) UNSIGNED NOT NULL DEFAULT 0,\
	`player_id` INT NOT NULL,\
	`bid` INT UNSIGNED NOT NULL DEFAULT 0,\
	`limit` INT UNSIGNED NOT NULL DEFAULT 0,\
	`endtime` BIGINT UNSIGNED NOT NULL DEFAULT 0,\
	UNIQUE (`house_id`, `world_id`),\
	FOREIGN KEY (`house_id`, `world_id`) REFERENCES `houses`(`id`, `world_id`) ON DELETE CASCADE,\
	FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE\
) ENGINE = InnoDB;"
					};
					for(uint32_t i = 0; i < sizeof(queryList) / sizeof(std::string); ++i)
						db->query(queryList[i]);

					break;
				}

				case DATABASE_ENGINE_SQLITE:
				{
					std::string queryList[] = {
						"ALTER TABLE `houses` ADD `tiles` INTEGER NOT NULL DEFAULT 0;",
						"CREATE TABLE `house_auctions` (\
	`house_id` INTEGER NOT NULL,\
	`world_id` INTEGER NOT NULL DEFAULT 0,\
	`player_id` INTEGER NOT NULL,\
	`bid` INTEGER NOT NULL DEFAULT 0,\
	`limit` INTEGER NOT NULL DEFAULT 0,\
	`endtime` INTEGER NOT NULL DEFAULT 0,\
	UNIQUE (`house_id`, `world_id`),\
	FOREIGN KEY (`house_id`, `world_id`) REFERENCES `houses` (`id`, `world_id`)\
	FOREIGN KEY (`player_id`) REFERENCES `players` (`id`)\
);"
					};
					for(uint32_t i = 0; i < sizeof(queryList) / sizeof(std::string); ++i)
						db->query(queryList[i]);

					break;
				}

				default:
					//TODO
					break;
			}

			registerDatabaseConfig("db_version", 19);
			return 19;
		}

		case 19:
		{
			std::clog << "> Updating database to version: 20..." << std::endl;
			switch(db->getDatabaseEngine())
			{
				case DATABASE_ENGINE_MYSQL:
				{
					std::string queryList[] = {
						"ALTER TABLE `players` CHANGE `redskulltime` `skulltime` INT NOT NULL DEFAULT 0;",
						"ALTER TABLE `players` ADD `skull` TINYINT(1) UNSIGNED NOT NULL DEFAULT 0 AFTER `save`;"
					};
					for(uint32_t i = 0; i < sizeof(queryList) / sizeof(std::string); ++i)
						db->query(queryList[i]);

					break;
				}

				case DATABASE_ENGINE_SQLITE:
				{
					std::string queryList[] = {
						"ALTER TABLE `players` ADD `skulltime` INTEGER NOT NULL DEFAULT 0;",
						"ALTER TABLE `players` ADD `skull` INTEGER NOT NULL DEFAULT 0;",
						"UPDATE `players` SET `skulltime` = `redskulltime`, `redskulltime` = 0;"
					};
					for(uint32_t i = 0; i < sizeof(queryList) / sizeof(std::string); ++i)
						db->query(queryList[i]);

					break;
				}

				default:
					//TODO
					break;
			}


			registerDatabaseConfig("db_version", 20);
			return 20;
		}

		case 20:
		{
			std::clog << "> Updating database to version: 21..." << std::endl;
			std::string queryList[] = {
				"UPDATE `bans` SET `type` = 3 WHERE `type` = 5;",
				"UPDATE `bans` SET `param` = 2 WHERE `type` = 2;",
				"UPDATE `bans` SET `param` = 0 WHERE `type` IN (3,4);"
			};
			for(uint32_t i = 0; i < sizeof(queryList) / sizeof(std::string); ++i)
				db->query(queryList[i]);

			registerDatabaseConfig("db_version", 21);
			return 21;
		}

		case 21:
		{
			std::clog << "> Updating database to version: 22..." << std::endl;
			switch(db->getDatabaseEngine())
			{
				case DATABASE_ENGINE_MYSQL:
					db->query("CREATE TABLE `account_viplist` (`account_id` INT NOT NULL, `world_id` TINYINT(2) UNSIGNED NOT NULL DEFAULT 0, `player_id` INT NOT NULL, KEY (`account_id`), KEY (`player_id`), KEY (`world_id`), UNIQUE (`account_id`, `player_id`), FOREIGN KEY (`account_id`) REFERENCES `accounts`(`id`) ON DELETE CASCADE, FOREIGN KEY (`player_id`) REFERENCES `players`(`id`) ON DELETE CASCADE) ENGINE = InnoDB;");
					break;
				case DATABASE_ENGINE_SQLITE:
					db->query("CREATE TABLE `account_viplist` (`account_id` INTEGER NOT NULL, `world_id` INTEGER NOT NULL DEFAULT 0, `player_id` INTEGER NOT NULL, UNIQUE (`account_id`, `player_id`), FOREIGN KEY `account_id` REFERENCES `accounts` (`id`), FOREIGN KEY `player_id` REFERENCES `players` (`id`));");
					break;
				default:
					break;
			}

			registerDatabaseConfig("db_version", 22);
			return 22;
		}

		case 22:
		{
			std::clog << "> Updating database to version 23..." << std::endl;
			if(g_config.getBool(ConfigManager::ACCOUNT_MANAGER))
			{
				query << "SELECT `id`, `key` FROM `accounts` WHERE `key` ";
				if(db->getDatabaseEngine() == DATABASE_ENGINE_SQLITE)
					query << "NOT LIKE";
				else
					query << "!=";

				query << " '0';";
				if(DBResult* result = db->storeQuery(query.str()))
				{
					do
					{
						std::string key = result->getDataString("key");
						_encrypt(key, false);

						query.str("");
						query << "UPDATE `accounts` SET `key` = " << db->escapeString(key) << " WHERE `id` = " << result->getDataInt("id") << db->getUpdateLimiter();
						db->query(query.str());
					}
					while(result->next());
					result->free();
				}
			}

			query.str("");
			query << "DELETE FROM `server_config` WHERE `config` " << db->getStringComparer() << "'password_type';";
			db->query(query.str());

			registerDatabaseConfig("encryption", g_config.getNumber(ConfigManager::ENCRYPTION));
			registerDatabaseConfig("db_version", 23);
			return 23;
		}

		case 23:
		{
			std::clog << "> Updating database to version 24..." << std::endl;
			query << "ALTER TABLE `guilds` ADD `checkdata` ";
			if(db->getDatabaseEngine() == DATABASE_ENGINE_SQLITE)
				query << "INTEGER NOT NULL;";
			else
				query << "INT NOT NULL AFTER `creationdata`;";

			db->query(query.str());
			query.str("");
			registerDatabaseConfig("db_version", 24);
			return 24;
		}

		case 24:
		{
			std::clog << "> Updating database to version 25..." << std::endl;
			switch(db->getDatabaseEngine())
			{
				case DATABASE_ENGINE_SQLITE:
				{
					query << "DROP TABLE `server_config`;";
					db->query(query.str());

					query << "CREATE TABLE `server_config` (`config` VARCHAR(35) NOT NULL DEFAULT '', `value` VARCHAR(255) NOT NULL DEFAULT '', UNIQUE (`config`));";
					registerDatabaseConfig("encryption", g_config.getNumber(ConfigManager::ENCRYPTION));
					break;
				}

				case DATABASE_ENGINE_MYSQL:
				{
					query << "ALTER TABLE `server_config` CHANGE `value` `value` VARCHAR(255) NOT NULL DEFAULT '';";
					break;
				}

				default:
					break;
			}

			db->query(query.str());
			query.str("");

			registerDatabaseConfig("db_version", 25);
			return 25;
		}

		case 25:
		{
			std::clog << "> Updating database to version 26..." << std::endl;
			switch(db->getDatabaseEngine())
			{
				case DATABASE_ENGINE_SQLITE:
				{
					query << "ALTER TABLE `accounts` ADD `salt` VARCHAR(40) NOT NULL DEFAULT '';";
					break;
				}

				case DATABASE_ENGINE_MYSQL:
				{
					query << "ALTER TABLE `accounts` ADD `salt` VARCHAR(40) NOT NULL DEFAULT '' AFTER `password`;";
					break;
				}

				default:
					break;
			}

			db->query(query.str());
			query.str("");

			registerDatabaseConfig("db_version", 26);
			return 26;
		}

		case 26:
		{
			std::clog << "> Updating database to version 27..." << std::endl;
			if(db->getDatabaseEngine() == DATABASE_ENGINE_MYSQL)
			{
				query << "ALTER TABLE `player_storage` CHANGE `key` `key` VARCHAR(32) NOT NULL DEFAULT '0'";
				db->query(query.str());
				query.str("");

				query << "ALTER TABLE `global_storage` CHANGE `key` `key` VARCHAR(32) NOT NULL DEFAULT '0'";
				db->query(query.str());
				query.str("");
			}

			registerDatabaseConfig("db_version", 27);
			return 27;
		}

		case 27:
		{
			std::clog << "> Updating database to version 28..." << std::endl;
			if(db->getDatabaseEngine() == DATABASE_ENGINE_MYSQL)
			{
				query << "ALTER TABLE `players` ADD `currmount` INT NOT NULL DEFAULT 0 AFTER `lookaddons`;";
				db->query(query.str());
				query.str("");
			}

			registerDatabaseConfig("db_version", 28);
			return 28;
		}

		case 28:
		{
			std::clog << "> Updating database to version 29..." << std::endl;
			switch(db->getDatabaseEngine())
			{
				case DATABASE_ENGINE_SQLITE:
				{
					query << "ALTER TABLE `players` ADD `lookmount` INT NOT NULL DEFAULT 0;";
					break;
				}

				case DATABASE_ENGINE_MYSQL:
				{
					query << "ALTER TABLE `players` CHANGE `currmount` `lookmount` INT NOT NULL DEFAULT 0";
					break;
				}

				default:
					break;
			}

			db->query(query.str());
			query.str("");

			registerDatabaseConfig("db_version", 29);
			return 29;
		}

		case 29:
		{
			std::clog << "> Updating database to version 30..." << std::endl;
			switch(db->getDatabaseEngine())
			{
				case DATABASE_ENGINE_SQLITE:
				{
					query << "CREATE TABLE `tile_store` ( `house_id` INTEGER NOT NULL, `world_id` INTEGER NOT NULL DEFAULT 0, `data` LONGBLOB NOT NULL, FOREIGN KEY (`house_id`) REFERENCES `houses` (`id`) );";
					break;
				}

				case DATABASE_ENGINE_MYSQL:
				{
					query << "CREATE TABLE `tile_store` ( `house_id` INT UNSIGNED NOT NULL, `world_id` TINYINT(4) UNSIGNED NOT NULL DEFAULT 0, `data` LONGBLOB NOT NULL, FOREIGN KEY (`house_id`) REFERENCES `houses` (`id`) ON DELETE CASCADE ) ENGINE = InnoDB;";
					break;
				}

				default:
					break;
			}

			db->query(query.str());
			query.str("");

			registerDatabaseConfig("db_version", 30);
			return 30;
		}

		case 30:
		{
			std::clog << "> Updating database to version 31... (PVP Blessing System)" << std::endl;
			switch(db->getDatabaseEngine())
			{
				case DATABASE_ENGINE_SQLITE:
				{
					query << "ALTER TABLE `players` ADD `pvp_blessing` BOOLEAN NOT NULL DEFAULT FALSE;";
					break;
				}

				case DATABASE_ENGINE_MYSQL:
				{
					query << "ALTER TABLE `players` ADD `pvp_blessing` TINYINT(1) NOT NULL DEFAULT 0 AFTER `blessings`;";
					break;
				}

				default:
					break;
			}

			db->query(query.str());
			query.str("");

			registerDatabaseConfig("db_version", 31);
			return 31;
		}

		case 31:
		{
			std::clog << "> Updating database to version 32... (warSystem MySQL)" << std::endl;
			if(db->getDatabaseEngine() == DATABASE_ENGINE_MYSQL)
			{
				query << "CREATE TABLE IF NOT EXISTS `player_statements`\
(\
	`id` INT NOT NULL AUTO_INCREMENT,\
	`player_id` INT NOT NULL,\
	`channel_id` INT NOT NULL DEFAULT 0,\
	`text` VARCHAR (255) NOT NULL,\
	`date` BIGINT NOT NULL DEFAULT 0,\
	PRIMARY KEY (`id`), KEY (`player_id`), KEY (`channel_id`),\
	FOREIGN KEY (`player_id`) REFERENCES `players`(`id`) ON DELETE CASCADE\
) ENGINE = InnoDB;";

				db->query(query.str());
				query.str("");

				query << "CREATE TABLE IF NOT EXISTS `guild_wars`\
(\
	`id` INT NOT NULL AUTO_INCREMENT,\
	`guild_id` INT NOT NULL,\
	`enemy_id` INT NOT NULL,\
	`begin` BIGINT NOT NULL DEFAULT 0,\
	`end` BIGINT NOT NULL DEFAULT 0,\
	`frags` INT UNSIGNED NOT NULL DEFAULT 0,\
	`payment` BIGINT UNSIGNED NOT NULL DEFAULT 0,\
	`guild_kills` INT UNSIGNED NOT NULL DEFAULT 0,\
	`enemy_kills` INT UNSIGNED NOT NULL DEFAULT 0,\
	`status` TINYINT(1) UNSIGNED NOT NULL DEFAULT 0,\
	PRIMARY KEY (`id`), KEY `status` (`status`),\
	KEY `guild_id` (`guild_id`), KEY `enemy_id` (`enemy_id`),\
	FOREIGN KEY (`guild_id`) REFERENCES `guilds`(`id`) ON DELETE CASCADE,\
	FOREIGN KEY (`enemy_id`) REFERENCES `guilds`(`id`) ON DELETE CASCADE\
) ENGINE=InnoDB;";

				db->query(query.str());
				query.str("");

				query << "CREATE TABLE IF NOT EXISTS `guild_kills`\
(\
	`id` INT NOT NULL AUTO_INCREMENT PRIMARY KEY,\
	`guild_id` INT NOT NULL,\
	`war_id` INT NOT NULL,\
	`death_id` INT NOT NULL,\
	FOREIGN KEY (`guild_id`) REFERENCES `guilds`(`id`) ON DELETE CASCADE,\
	FOREIGN KEY (`war_id`) REFERENCES `guild_wars`(`id`) ON DELETE CASCADE,\
	FOREIGN KEY (`death_id`) REFERENCES `player_deaths`(`id`) ON DELETE CASCADE\
) ENGINE = InnoDB;";

				db->query(query.str());
				query.str("");

				query << "ALTER TABLE `killers` ADD `war` INT NOT NULL DEFAULT 0;";
				db->query(query.str());
				query.str("");

				query << "ALTER TABLE `guilds` ADD `balance` BIGINT UNSIGNED NOT NULL AFTER `motd`;";
				db->query(query.str());
				query.str("");
			}

			registerDatabaseConfig("db_version", 32);
			return 32;
		}

		case 32:
		{
			std::clog << "> Updating database to version 33... (Modern Depot System ths no work on 9.60-)" << std::endl;
			registerDatabaseConfig("db_version", 33);
			return 33;
		}

		case 33:
		{
			std::clog << "> Updating database to version 34... (Market System this no work on 9.4+)" << std::endl;
			switch(db->getDatabaseEngine())
			{
				case DATABASE_ENGINE_MYSQL:
				{
					db->query("CREATE TABLE `market_offers` (`id` INT UNSIGNED NOT NULL AUTO_INCREMENT, `player_id` INT NOT NULL, `sale` TINYINT(1) NOT NULL DEFAULT 0, `itemtype` INT UNSIGNED NOT NULL, `amount` SMALLINT UNSIGNED NOT NULL, `created` BIGINT UNSIGNED NOT NULL, `anonymous` TINYINT(1) NOT NULL DEFAULT 0, `price` INT UNSIGNED NOT NULL DEFAULT 0, PRIMARY KEY (`id`), KEY(`sale`, `itemtype`), KEY(`created`), FOREIGN KEY (`player_id`) REFERENCES `players`(`id`) ON DELETE CASCADE) ENGINE = InnoDB;");
					db->query("CREATE TABLE `market_history` (`id` INT UNSIGNED NOT NULL AUTO_INCREMENT, `player_id` INT NOT NULL, `sale` TINYINT(1) NOT NULL DEFAULT 0, `itemtype` INT UNSIGNED NOT NULL, `amount` SMALLINT UNSIGNED NOT NULL, `price` INT UNSIGNED NOT NULL DEFAULT 0, `expires_at` BIGINT UNSIGNED NOT NULL, `inserted` BIGINT UNSIGNED NOT NULL, `state` TINYINT(1) UNSIGNED NOT NULL, PRIMARY KEY(`id`), KEY(`player_id`, `sale`), FOREIGN KEY (`player_id`) REFERENCES `players`(`id`) ON DELETE CASCADE) ENGINE = InnoDB;");
					break;
				}

				case DATABASE_ENGINE_SQLITE:
				{
					db->query("CREATE TABLE `market_offers` (`id` INTEGER PRIMARY KEY NOT NULL, `player_id` INTEGER NOT NULL, `sale` BOOLEAN NOT NULL DEFAULT 0, `itemtype` UNSIGNED INTEGER NOT NULL, `amount` UNSIGNED INTEGER NOT NULL, `created` UNSIGNED INTEGER NOT NULL, `anonymous` BOOLEAN NOT NULL DEFAULT 0, `price` UNSIGNED INTEGER NOT NULL DEFAULT 0, FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE);");
					db->query("CREATE INDEX market_offers_idx ON market_offers(created);");
					db->query("CREATE INDEX market_offers_idx2 ON market_offers(sale, itemtype);");
					db->query("CREATE TABLE `market_history` (`id` INTEGER PRIMARY KEY NOT NULL, `player_id` INTEGER NOT NULL, `sale` BOOLEAN NOT NULL DEFAULT 0, `itemtype` UNSIGNED INTEGER NOT NULL, `amount` UNSIGNED INTEGER NOT NULL, `price` UNSIGNED INTEGER NOT NULL DEFAULT 0, `expires_at` UNSIGNED INTEGER NOT NULL, `inserted` UNSIGNED INTEGER NOT NULL, `state` UNSIGNED INTEGER NOT NULL, FOREIGN KEY (`player_id`) REFERENCES `players` (`id`) ON DELETE CASCADE);");
					db->query("CREATE INDEX market_history_idx ON market_history(player_id, sale);");
					break;
				}

				default: break;
			}

			registerDatabaseConfig("db_version", 34);
			return 34;
		}

		case 34:
		{
			std::clog << "> Updating database to version 35... (warSystem SQLite)" << std::endl;
			switch(db->getDatabaseEngine())
			{
				case DATABASE_ENGINE_SQLITE:
				{
					db->query("CREATE TABLE IF NOT EXISTS `guild_wars` (\
						`id` INTEGER NOT NULL,\
						`guild_id` INT NOT NULL,\
						`enemy_id` INT NOT NULL,\
						`begin` BIGINT NOT NULL DEFAULT '0',\
						`end` BIGINT NOT NULL DEFAULT '0',\
						`frags` INT NOT NULL DEFAULT '0',\
						`payment` BIGINT NOT NULL DEFAULT '0',\
						`guild_kills` INT NOT NULL DEFAULT '0',\
						`enemy_kills` INT NOT NULL DEFAULT '0',\
						`status` TINYINT(1) NOT NULL DEFAULT '0',\
						PRIMARY KEY (`id`),\
						FOREIGN KEY (`guild_id`) REFERENCES `guilds`(`id`),\
						FOREIGN KEY (`enemy_id`) REFERENCES `guilds`(`id`)\
						);");

					db->query("CREATE TABLE IF NOT EXISTS `guild_kills` (\
						`id` INT NOT NULL PRIMARY KEY,\
						`guild_id` INT NOT NULL,\
						`war_id` INT NOT NULL,\
						`death_id` INT NOT NULL\
					);");
					
					db->query("CREATE TABLE IF NOT EXISTS `player_statements` (\
						`id` INTEGER PRIMARY KEY,\
						`player_id` INTEGER NOT NULL,\
						`channel_id` INTEGER NOT NULL DEFAULT `0`,\
						`text` VARCHAR (255) NOT NULL,\
						`date` INTEGER NOT NULL DEFAULT `0`,\
						FOREIGN KEY (`player_id`) REFERENCES `players` (`id`)\
						);");

					db->query("ALTER TABLE `guilds` ADD `balance` BIGINT NOT NULL DEFAULT '0';");
					db->query("ALTER TABLE `killers` ADD `war` BIGINT NOT NULL DEFAULT 0;");
					break;
				}

				default:
					break;
			}

			registerDatabaseConfig("db_version", 35);
			return 35;
		}

		case 35:
		{
			std::clog << "> Updating database to version 36... (modernDepotSystem)" << std::endl;
			registerDatabaseConfig("db_version", 36);
			return 36;
		}

		case 36:
		{
			std::clog << "> Updating database to version 37... (loginHistoryIPSystem)" << std::endl;
			switch(db->getDatabaseEngine())
			{
				case DATABASE_ENGINE_MYSQL:
				{
					db->query("CREATE TABLE `login_history` (`account_id` INT(11) NOT NULL DEFAULT 0, `player_id` INT(11) NOT NULL DEFAULT 0, `type` TINYINT(1) NOT NULL DEFAULT 0, `login` TINYINT(1) NOT NULL DEFAULT 0, `ip` INT(11) NOT NULL DEFAULT 0, `date` INT(11) NOT NULL DEFAULT 0, FOREIGN KEY (`player_id`) REFERENCES `players` (`id`));");
					db->query("ALTER TABLE `players` ADD `ip` varchar(17) NOT NULL DEFAULT '0';");
					break;
				}

				case DATABASE_ENGINE_SQLITE:
				{
					db->query("CREATE TABLE 'login_history' (\
						'account_id' INT(11) NOT NULL DEFAULT 0,\
						'player_id' INT(11) NOT NULL DEFAULT 0,\
						'type' TINYINT(1) NOT NULL DEFAULT 0,\
						'login' TINYINT(1) NOT NULL DEFAULT 0,\
						'ip' INTEGER(11) NOT NULL DEFAULT 0,\
						'date' INTEGER(11) NOT NULL DEFAULT 0\
						);");
					db->query("ALTER TABLE `players` ADD `ip` varchar(17) NOT NULL DEFAULT '0';");
					break;
				}

				default:
					break;
			}

			registerDatabaseConfig("db_version", 37);
			return 37;
		}

		case 37:
		{
			std::clog << "> Updating database to version 38... (offLineTrainingSystem)" << std::endl;

			db->query("ALTER TABLE `players` ADD `offlinetraining_time` SMALLINT UNSIGNED NOT NULL DEFAULT 43200;");
			db->query("ALTER TABLE `players` ADD `offlinetraining_skill` INT NOT NULL DEFAULT -1;");
			registerDatabaseConfig("db_version", 38);
			return 38;
		}

		case 38:
		{
			std::clog << "> Updating database to version 39... (FIX)" << std::endl;
			registerDatabaseConfig("db_version", 39);
			return 39;
		}

		case 39:
		{
			std::clog << "> Updating database to version 40... (FIX)" << std::endl;
			registerDatabaseConfig("db_version", 40);
			return 40;
		}

		case 40:
		{
			std::clog << "> Updating database to version 41... (FIX)" << std::endl;
			registerDatabaseConfig("db_version", 41);
			return 41;
		}

		case 41:
		{
			std::clog << "> Updating database to version 42... (FIX)" << std::endl;
			registerDatabaseConfig("db_version", 42);
			return 42;
		}

		case 42:
		{
			std::clog << "> Updating database to version 43... (Cast & AntiDupe System)" << std::endl;
			switch(db->getDatabaseEngine())
			{
				case DATABASE_ENGINE_MYSQL:
				{
					db->query("ALTER TABLE player_items ADD serial VARCHAR(255) NOT NULL DEFAULT '';");
					db->query("ALTER TABLE player_depotitems ADD serial VARCHAR(255) NOT NULL DEFAULT '';");
					db->query("ALTER TABLE tile_items ADD serial VARCHAR(255) NOT NULL DEFAULT '';");
					db->query("ALTER TABLE tile_store ADD serial VARCHAR(255) NOT NULL DEFAULT '';");
					db->query("ALTER TABLE house_data ADD serial VARCHAR(255) NOT NULL DEFAULT '';");
					db->query("ALTER TABLE `players` ADD `broadcasting` tinyint(4) DEFAULT '0'");
					db->query("ALTER TABLE `players` ADD `viewers` INT(1) DEFAULT '0'");
					break;
				}

				case DATABASE_ENGINE_SQLITE:
				{
					db->query("ALTER TABLE `player_items` ADD `serial` VARCHAR(255) NOT NULL DEFAULT '';");
					db->query("ALTER TABLE `player_depotitems` ADD `serial` VARCHAR(255) NOT NULL DEFAULT '';");
					db->query("ALTER TABLE `tile_items` ADD `serial` VARCHAR(255) NOT NULL DEFAULT '';");
					db->query("ALTER TABLE `tile_store` ADD `serial` VARCHAR(255) NOT NULL DEFAULT '';");
					db->query("ALTER TABLE `house_data` ADD `serial` VARCHAR(255) NOT NULL DEFAULT '';");
					db->query("ALTER TABLE `players` ADD `broadcasting` BOOLEAN(4) NOT NULL DEFAULT 0;");
					db->query("ALTER TABLE `players` ADD `viewers` INTEGER(1) NOT NULL DEFAULT 0;");
					break;
				}

				default:
					break;
			}

			registerDatabaseConfig("db_version", 43);
			return 43;
		}

		default:
			break;
	}

	return 0;
}

bool DatabaseManager::getDatabaseConfig(std::string config, int32_t &value)
{
	value = 0;

	Database* db = Database::getInstance();
	DBResult* result;

	std::ostringstream query;
	query << "SELECT `value` FROM `server_config` WHERE `config` = " << db->escapeString(config) << ";";
	if(!(result = db->storeQuery(query.str())))
		return false;

	value = atoi(result->getDataString("value").c_str());
	result->free();
	return true;
}

bool DatabaseManager::getDatabaseConfig(std::string config, std::string &value)
{
	value = "";

	Database* db = Database::getInstance();
	DBResult* result;

	std::ostringstream query;
	query << "SELECT `value` FROM `server_config` WHERE `config` = " << db->escapeString(config) << ";";
	if(!(result = db->storeQuery(query.str())))
		return false;

	value = result->getDataString("value");
	result->free();
	return true;
}

void DatabaseManager::registerDatabaseConfig(std::string config, int32_t value)
{
	Database* db = Database::getInstance();
	std::ostringstream query;

	int32_t tmp = 0;
	if(!getDatabaseConfig(config, tmp))
		query << "INSERT INTO `server_config` VALUES (" << db->escapeString(config) << ", '" << value << "');";
	else
		query << "UPDATE `server_config` SET `value` = '" << value << "' WHERE `config` = " << db->escapeString(config) << ";";

	db->query(query.str());
}

void DatabaseManager::registerDatabaseConfig(std::string config, std::string value)
{
	Database* db = Database::getInstance();
	std::ostringstream query;

	std::string tmp;
	if(!getDatabaseConfig(config, tmp))
		query << "INSERT INTO `server_config` VALUES (" << db->escapeString(config) << ", " << db->escapeString(value) << ");";
	else
		query << "UPDATE `server_config` SET `value` = " << db->escapeString(value) << " WHERE `config` = " << db->escapeString(config) << ";";

	db->query(query.str());
}

void DatabaseManager::checkEncryption()
{
	Encryption_t newValue = (Encryption_t)g_config.getNumber(ConfigManager::ENCRYPTION);
	int32_t value = (int32_t)ENCRYPTION_PLAIN;
	if(getDatabaseConfig("encryption", value))
	{
		if(newValue != (Encryption_t)value)
		{
			switch(newValue)
			{
				case ENCRYPTION_SHA1:
				{
					if((Encryption_t)value != ENCRYPTION_PLAIN)
					{
						std::clog << "> WARNING: You cannot change the encryption to SHA1, change it back in config.lua." << std::endl;
						return;
					}

					Database* db = Database::getInstance();
					std::ostringstream query;
					if(db->getDatabaseEngine() == DATABASE_ENGINE_SQLITE)
					{
						query << "SELECT `id`, `password`, `key` FROM `accounts`;";
						if(DBResult* result = db->storeQuery(query.str()))
						{
							do
							{
								query << "UPDATE `accounts` SET `password` = " << db->escapeString(transformToSHA1(result->getDataString("password"), false)) << ", `key` = " << db->escapeString(transformToSHA1(result->getDataString("key"), false)) << " WHERE `id` = " << result->getDataInt("id") << ";";
								db->query(query.str());
							}
							while(result->next());
							result->free();
						}
					}
					else
						db->query("UPDATE `accounts` SET `password` = SHA1(`password`), `key` = SHA1(`key`);");

					registerDatabaseConfig("encryption", (int32_t)newValue);
					std::clog << "> Encryption set to SHA1." << std::endl;
					break;
				}
				default:
				{
					std::clog << "> WARNING: You cannot switch from hashed passwords to plain text, change back the passwordType in config.lua to the passwordType you were previously using." << std::endl;
					break;
				}
			}
		}
	}
	else
	{
		registerDatabaseConfig("encryption", (int32_t)newValue);
		if(g_config.getBool(ConfigManager::ACCOUNT_MANAGER))
		{
			switch(newValue)
			{
				case ENCRYPTION_SHA1:
				{
					Database* db = Database::getInstance();
					std::ostringstream query;
					query << "UPDATE `accounts` SET `password` = " << db->escapeString(transformToSHA1("1", false)) << " WHERE `id` = 1 AND `password` = '1';";
					db->query(query.str());
					break;
				}
				default:
					break;
			}
		}
	}
}

void DatabaseManager::checkTriggers()
{
	Database* db = Database::getInstance();
	switch(db->getDatabaseEngine())
	{
		case DATABASE_ENGINE_MYSQL:
		{
			std::string triggerName[] =
			{
				"ondelete_accounts",
				"oncreate_guilds",
				"ondelete_guilds",
				"oncreate_players",
				"ondelete_players",
			};

			std::string triggerStatement[] =
			{
				"CREATE TRIGGER `ondelete_accounts` BEFORE DELETE ON `accounts` FOR EACH ROW BEGIN DELETE FROM `bans` WHERE `type` NOT IN(1, 2) AND `value` = OLD.`id`; END;",
				"CREATE TRIGGER `oncreate_guilds` AFTER INSERT ON `guilds` FOR EACH ROW BEGIN INSERT INTO `guild_ranks` (`name`, `level`, `guild_id`) VALUES ('Leader', 3, NEW.`id`); INSERT INTO `guild_ranks` (`name`, `level`, `guild_id`) VALUES ('Vice-Leader', 2, NEW.`id`); INSERT INTO `guild_ranks` (`name`, `level`, `guild_id`) VALUES ('Member', 1, NEW.`id`); END;",
				"CREATE TRIGGER `ondelete_guilds` BEFORE DELETE ON `guilds` FOR EACH ROW BEGIN UPDATE `players` SET `guildnick` = '', `rank_id` = 0 WHERE `rank_id` IN (SELECT `id` FROM `guild_ranks` WHERE `guild_id` = OLD.`id`); END;",
				"CREATE TRIGGER `oncreate_players` AFTER INSERT ON `players` FOR EACH ROW BEGIN INSERT INTO `player_skills` (`player_id`, `skillid`, `value`) VALUES (NEW.`id`, 0, 10); INSERT INTO `player_skills` (`player_id`, `skillid`, `value`) VALUES (NEW.`id`, 1, 10); INSERT INTO `player_skills` (`player_id`, `skillid`, `value`) VALUES (NEW.`id`, 2, 10); INSERT INTO `player_skills` (`player_id`, `skillid`, `value`) VALUES (NEW.`id`, 3, 10); INSERT INTO `player_skills` (`player_id`, `skillid`, `value`) VALUES (NEW.`id`, 4, 10); INSERT INTO `player_skills` (`player_id`, `skillid`, `value`) VALUES (NEW.`id`, 5, 10); INSERT INTO `player_skills` (`player_id`, `skillid`, `value`) VALUES (NEW.`id`, 6, 10); END;",
				"CREATE TRIGGER `ondelete_players` BEFORE DELETE ON `players` FOR EACH ROW BEGIN DELETE FROM `bans` WHERE `type` = 2 AND `value` = OLD.`id`; UPDATE `houses` SET `owner` = 0 WHERE `owner` = OLD.`id`; END;"
			};

			std::ostringstream query;
			for(uint32_t i = 0; i < sizeof(triggerName) / sizeof(std::string); ++i)
			{
				if(!triggerExists(triggerName[i]))
				{
					std::clog << "> Trigger: " << triggerName[i] << " does not exist, creating it..." << std::endl;
					db->query(triggerStatement[i]);
				}
			}

			break;
		}

		case DATABASE_ENGINE_SQLITE:
		{
			std::string triggerName[] =
			{
				"oncreate_guilds",
				"oncreate_players",
				"ondelete_accounts",
				"ondelete_players",
				"ondelete_guilds",
				"oninsert_players",
				"onupdate_players",
				"oninsert_guilds",
				"onupdate_guilds",
				"ondelete_houses",
				"ondelete_tiles",
				"oninsert_guild_ranks",
				"onupdate_guild_ranks",
				"oninsert_house_lists",
				"onupdate_house_lists",
				"oninsert_player_depotitems",
				"onupdate_player_depotitems",
				"oninsert_player_skills",
				"onupdate_player_skills",
				"oninsert_player_storage",
				"onupdate_player_storage",
				"oninsert_player_viplist",
				"onupdate_player_viplist",
				"oninsert_account_viplist",
				"onupdate_account_viplist",
				"oninsert_tile_items",
				"onupdate_tile_items",
				"oninsert_player_spells",
				"onupdate_player_spells",
				"oninsert_player_deaths",
				"onupdate_player_deaths",
				"oninsert_killers",
				"onupdate_killers",
				"oninsert_environment_killers",
				"onupdate_environment_killers",
				"oninsert_player_killers",
				"onupdate_player_killers"
			};

			std::string triggerStatement[] =
			{
				"CREATE TRIGGER `oncreate_guilds` \
AFTER INSERT ON `guilds` \
BEGIN \
	INSERT INTO `guild_ranks` (`name`, `level`, `guild_id`) VALUES (`Leader`, 3, NEW.`id`);\
	INSERT INTO `guild_ranks` (`name`, `level`, `guild_id`) VALUES (`Vice-Leader`, 2, NEW.`id`);\
	INSERT INTO `guild_ranks` (`name`, `level`, `guild_id`) VALUES (`Member`, 1, NEW.`id`);\
END;",

				"CREATE TRIGGER `oncreate_players`\
AFTER INSERT\
ON `players`\
BEGIN\
	INSERT INTO `player_skills` (`player_id`, `skillid`, `value`) VALUES (NEW.`id`, 0, 10);\
	INSERT INTO `player_skills` (`player_id`, `skillid`, `value`) VALUES (NEW.`id`, 1, 10);\
	INSERT INTO `player_skills` (`player_id`, `skillid`, `value`) VALUES (NEW.`id`, 2, 10);\
	INSERT INTO `player_skills` (`player_id`, `skillid`, `value`) VALUES (NEW.`id`, 3, 10);\
	INSERT INTO `player_skills` (`player_id`, `skillid`, `value`) VALUES (NEW.`id`, 4, 10);\
	INSERT INTO `player_skills` (`player_id`, `skillid`, `value`) VALUES (NEW.`id`, 5, 10);\
	INSERT INTO `player_skills` (`player_id`, `skillid`, `value`) VALUES (NEW.`id`, 6, 10);\
END;",
				"CREATE TRIGGER `ondelete_accounts`\
BEFORE DELETE\
ON `accounts`\
FOR EACH ROW\
BEGIN\
	DELETE FROM `players` WHERE `account_id` = OLD.`id`;\
	DELETE FROM `account_viplist` WHERE `account_id` = OLD.`id`;\
	DELETE FROM `bans` WHERE `type` IN (3, 4) AND `value` = OLD.`id`;\
END;",

				"CREATE TRIGGER `ondelete_players`\
BEFORE DELETE\
ON `players`\
FOR EACH ROW\
BEGIN\
	SELECT RAISE(ROLLBACK, 'DELETE on table `players` violates foreign: `ownerid` from table `guilds`')\
	WHERE (SELECT `id` FROM `guilds` WHERE `ownerid` = OLD.`id`) IS NOT NULL;\
\
	DELETE FROM `account_viplist` WHERE `player_id` = OLD.`id`;\
	DELETE FROM `player_viplist` WHERE `player_id` = OLD.`id` OR `vip_id` = OLD.`id`;\
	DELETE FROM `player_storage` WHERE `player_id` = OLD.`id`;\
	DELETE FROM `player_skills` WHERE `player_id` = OLD.`id`;\
	DELETE FROM `player_items` WHERE `player_id` = OLD.`id`;\
	DELETE FROM `player_depotitems` WHERE `player_id` = OLD.`id`;\
	DELETE FROM `player_spells` WHERE `player_id` = OLD.`id`;\
	DELETE FROM `player_killers` WHERE `player_id` = OLD.`id`;\
	DELETE FROM `player_deaths` WHERE `player_id` = OLD.`id`;\
	DELETE FROM `guild_invites` WHERE `player_id` = OLD.`id`;\
	DELETE FROM `bans` WHERE `type` IN (2, 5) AND `value` = OLD.`id`;\
	UPDATE `houses` SET `owner` = 0 WHERE `owner` = OLD.`id`;\
END;",
				"CREATE TRIGGER `ondelete_guilds`\
BEFORE DELETE\
ON `guilds`\
FOR EACH ROW\
BEGIN\
	UPDATE `players` SET `guildnick` = '', `rank_id` = 0 WHERE `rank_id` IN (SELECT `id` FROM `guild_ranks` WHERE `guild_id` = OLD.`id`);\
	DELETE FROM `guild_ranks` WHERE `guild_id` = OLD.`id`;\
	DELETE FROM `guild_invites` WHERE `guild_id` = OLD.`id`;\
END;",

				"CREATE TRIGGER `oninsert_players` BEFORE INSERT ON `players` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'INSERT on table `players` violates foreign: `account_id`') WHERE NEW.`account_id` IS NULL OR (SELECT `id` FROM `accounts` WHERE `id` = NEW.`account_id`) IS NULL; END;",
				"CREATE TRIGGER `onupdate_players` BEFORE UPDATE ON `players` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'UPDATE on table `players` violates foreign: `account_id`') WHERE NEW.`account_id` IS NULL OR (SELECT `id` FROM `accounts` WHERE `id` = NEW.`account_id`) IS NULL; END;",

				"CREATE TRIGGER `oninsert_guilds` BEFORE INSERT ON `guilds` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'INSERT on table `guilds` violates foreign: `ownerid`') WHERE NEW.`ownerid` IS NULL OR (SELECT `id` FROM `players` WHERE `id` = NEW.`ownerid`) IS NULL; END;",
				"CREATE TRIGGER `onupdate_guilds` BEFORE UPDATE ON `guilds` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'UPDATE on table `guilds` violates foreign: `ownerid`') WHERE NEW.`ownerid` IS NULL OR (SELECT `id` FROM `players` WHERE `id` = NEW.`ownerid`) IS NULL; END;",

				"CREATE TRIGGER `ondelete_houses` BEFORE DELETE ON `houses` FOR EACH ROW BEGIN DELETE FROM `house_lists` WHERE `house_id` = OLD.`id`; END;",
				"CREATE TRIGGER `ondelete_tiles` BEFORE DELETE ON `tiles` FOR EACH ROW BEGIN DELETE FROM `tile_items` WHERE `tile_id` = OLD.`id`; END;",

				"CREATE TRIGGER `oninsert_guild_ranks` BEFORE INSERT ON `guild_ranks` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'INSERT on table `guild_ranks` violates foreign: `guild_id`') WHERE NEW.`guild_id` IS NULL OR (SELECT `id` FROM `guilds` WHERE `id` = NEW.`guild_id`) IS NULL; END;",
				"CREATE TRIGGER `onupdate_guild_ranks` BEFORE UPDATE ON `guild_ranks` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'UPDATE on table `guild_ranks` violates foreign: `guild_id`') WHERE NEW.`guild_id` IS NULL OR (SELECT `id` FROM `guilds` WHERE `id` = NEW.`guild_id`) IS NULL; END;",

				"CREATE TRIGGER `oninsert_house_lists` BEFORE INSERT ON `house_lists` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'INSERT on table `house_lists` violates foreign: `house_id`') WHERE NEW.`house_id` IS NULL OR (SELECT `id` FROM `houses` WHERE `id` = NEW.`house_id`) IS NULL; END;",
				"CREATE TRIGGER `onupdate_house_lists` BEFORE UPDATE ON `house_lists` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'UPDATE on table `house_lists` violates foreign: `house_id`') WHERE NEW.`house_id` IS NULL OR (SELECT `id` FROM `houses` WHERE `id` = NEW.`house_id`) IS NULL; END;",

				"CREATE TRIGGER `oninsert_player_depotitems` BEFORE INSERT ON `player_depotitems` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'INSERT on table `player_depotitems` violates foreign: `player_id`') WHERE NEW.`player_id` IS NULL OR (SELECT `id` FROM `players` WHERE `id` = NEW.`player_id`) IS NULL; END;",
				"CREATE TRIGGER `onupdate_player_depotitems` BEFORE UPDATE ON `player_depotitems` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'UPDATE on table `player_depotitems` violates foreign: `player_id`') WHERE NEW.`player_id` IS NULL OR (SELECT `id` FROM `players` WHERE `id` = NEW.`player_id`) IS NULL; END;",

				"CREATE TRIGGER `oninsert_player_skills` BEFORE INSERT ON `player_skills` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'INSERT on table `player_skills` violates foreign: `player_id`') WHERE NEW.`player_id` IS NULL OR (SELECT `id` FROM `players` WHERE `id` = NEW.`player_id`) IS NULL; END;",
				"CREATE TRIGGER `onupdate_player_skills` BEFORE UPDATE ON `player_skills` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'UPDATE on table `player_skills` violates foreign: `player_id`') WHERE NEW.`player_id` IS NULL OR (SELECT `id` FROM `players` WHERE `id` = NEW.`player_id`) IS NULL; END;",

				"CREATE TRIGGER `oninsert_player_storage` BEFORE INSERT ON `player_storage` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'INSERT on table `player_storage` violates foreign: `player_id`') WHERE NEW.`player_id` IS NULL OR (SELECT `id` FROM `players` WHERE `id` = NEW.`player_id`) IS NULL; END;",
				"CREATE TRIGGER `onupdate_player_storage` BEFORE UPDATE ON `player_storage` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'UPDATE on table `player_storage` violates foreign: `player_id`') WHERE NEW.`player_id` IS NULL OR (SELECT `id` FROM `players` WHERE `id` = NEW.`player_id`) IS NULL; END;",

				"CREATE TRIGGER `oninsert_player_viplist` BEFORE INSERT ON `player_viplist` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'INSERT on table `player_viplist` violates foreign: `player_id`') WHERE NEW.`player_id` IS NULL OR (SELECT `id` FROM `players` WHERE `id` = NEW.`player_id`) IS NULL; SELECT RAISE(ROLLBACK, 'INSERT on table `player_viplist` violates foreign: `vip_id`') WHERE NEW.`vip_id` IS NULL OR (SELECT `id` FROM `players` WHERE `id` = NEW.`vip_id`) IS NULL; END;",
				"CREATE TRIGGER `onupdate_player_viplist` BEFORE UPDATE ON `player_viplist` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'UPDATE on table `player_viplist` violates foreign: `vip_id`') WHERE NEW.`vip_id` IS NULL OR (SELECT `id` FROM `players` WHERE `id` = NEW.`vip_id`) IS NULL; END;",

				"CREATE TRIGGER `oninsert_account_viplist` BEFORE INSERT ON `account_viplist` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'INSERT on table `account_viplist` violates foreign: `account_id`') WHERE NEW.`account_id` IS NULL OR (SELECT `id` FROM `accounts` WHERE `id` = NEW.`account_id`) IS NULL; SELECT RAISE(ROLLBACK, 'INSERT on table `account_viplist` violates foreign: `player_id`') WHERE NEW.`player_id` IS NULL OR (SELECT `id` FROM `players` WHERE `id` = NEW.`player_id`) IS NULL; END;",
				"CREATE TRIGGER `onupdate_account_viplist` BEFORE UPDATE ON `account_viplist` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'UPDATE on table `account_viplist` violates foreign: `player_id`') WHERE NEW.`player_id` IS NULL OR (SELECT `id` FROM `players` WHERE `id` = NEW.`player_id`) IS NULL; END;",

				"CREATE TRIGGER `oninsert_tile_items` BEFORE INSERT ON `tile_items` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'INSERT on table `tile_items` violates foreign: `tile_id`') WHERE NEW.`tile_id` IS NULL OR (SELECT `id` FROM `tiles` WHERE `id` = NEW.`tile_id`) IS NULL; END;",
				"CREATE TRIGGER `onupdate_tile_items` BEFORE UPDATE ON `tile_items` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'UPDATE on table `tile_items` violates foreign: `tile_id`') WHERE NEW.`tile_id` IS NULL OR (SELECT `id` FROM `tiles` WHERE `id` = NEW.`tile_id`) IS NULL; END;",

				"CREATE TRIGGER `oninsert_player_spells` BEFORE INSERT ON `player_spells` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'INSERT on table `player_spells` violates foreign: `player_id`') WHERE NEW.`player_id` IS NULL OR (SELECT `id` FROM `players` WHERE `id` = NEW.`player_id`) IS NULL; END;",
				"CREATE TRIGGER `onupdate_player_spells` BEFORE UPDATE ON `player_spells` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'UPDATE on table `player_spells` violates foreign: `player_id`') WHERE NEW.`player_id` IS NULL OR (SELECT `id` FROM `players` WHERE `id` = NEW.`player_id`) IS NULL; END;",
				"CREATE TRIGGER `oninsert_player_deaths` BEFORE INSERT ON `player_deaths` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'INSERT on table `player_deaths` violates foreign: `player_id`') WHERE NEW.`player_id` IS NULL OR (SELECT `id` FROM `players` WHERE `id` = NEW.`player_id`) IS NULL; END;",
				"CREATE TRIGGER `onupdate_player_deaths` BEFORE UPDATE ON `player_deaths` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'UPDATE on table `player_deaths` violates foreign: `player_id`') WHERE NEW.`player_id` IS NULL OR (SELECT `id` FROM `players` WHERE `id` = NEW.`player_id`) IS NULL; END;",

				"CREATE TRIGGER `oninsert_killers` BEFORE INSERT ON `killers` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'INSERT on table `killers` violates foreign: `death_id`') WHERE NEW.`death_id` IS NULL OR (SELECT `id` FROM `player_deaths` WHERE `id` = NEW.`death_id`) IS NULL; END;",
				"CREATE TRIGGER `onupdate_killers` BEFORE UPDATE ON `killers` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'UPDATE on table `killers` violates foreign: `death_id`') WHERE NEW.`death_id` IS NULL OR (SELECT `id` FROM `player_deaths` WHERE `id` = NEW.`death_id`) IS NULL; END;",
				"CREATE TRIGGER `oninsert_environment_killers` BEFORE INSERT ON `environment_killers` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'INSERT on table `enviroment_killers` violates foreign: `kill_id`') WHERE NEW.`kill_id` IS NULL OR (SELECT `id` FROM `killers` WHERE `id` = NEW.`kill_id`) IS NULL; END;",
				"CREATE TRIGGER `onupdate_environment_killers` BEFORE UPDATE ON `environment_killers` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'INSERT on table `enviroment_killers` violates foreign: `kill_id`') WHERE NEW.`kill_id` IS NULL OR (SELECT `id` FROM `killers` WHERE `id` = NEW.`kill_id`) IS NULL; END;",
				"CREATE TRIGGER `oninsert_player_killers` BEFORE INSERT ON `player_killers` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'INSERT on table `player_killers` violates foreign: `player_id`') WHERE NEW.`player_id` IS NULL OR (SELECT `id` FROM `players` WHERE `id` = NEW.`player_id`) IS NULL; SELECT RAISE(ROLLBACK, 'INSERT on table `player_killers` violates foreign: `kill_id`') WHERE NEW.`kill_id` IS NULL OR (SELECT `id` FROM `killers` WHERE `id` = NEW.`kill_id`) IS NULL; END;",
				"CREATE TRIGGER `onupdate_player_killers` BEFORE UPDATE ON `player_killers` FOR EACH ROW BEGIN SELECT RAISE(ROLLBACK, 'UPDATE on table `player_killers` violates foreign: `player_id`') WHERE NEW.`player_id` IS NULL OR (SELECT `id` FROM `players` WHERE `id` = NEW.`player_id`) IS NULL; SELECT RAISE(ROLLBACK, 'UPDATE on table `killers` violates foreign: `kill_id`') WHERE NEW.`kill_id` IS NULL OR (SELECT `id` FROM `killers` WHERE `id` = NEW.`kill_id`) IS NULL; END;"
			};

			std::ostringstream query;
			for(uint32_t i = 0; i < sizeof(triggerName) / sizeof(std::string); ++i)
			{
				if(!triggerExists(triggerName[i]))
				{
					std::clog << "> Trigger: " << triggerName[i] << " does not exist, creating it..." << std::endl;
					db->query(triggerStatement[i]);
				}
			}

			break;
		}

		case DATABASE_ENGINE_POSTGRESQL:
			//TODO: PostgreSQL support
			break;

		default:
			break;
	}
}
