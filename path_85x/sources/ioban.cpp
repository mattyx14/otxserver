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
#include "ioban.h"

#include "game.h"
#include "player.h"

#include "tools.h"
#include "database.h"
#include "iologindata.h"

extern Game g_game;

bool IOBan::isIpBanished(uint32_t ip, uint32_t mask/* = 0xFFFFFFFF*/) const
{
	if(!ip)
		return false;

	Database* db = Database::getInstance();
	DBResult* result;

	DBQuery query;
	query << "SELECT `id`, `value`, `param`, `expires` FROM `bans` WHERE `type` = " << BAN_IP << " AND `active` = 1";
	if(!(result = db->storeQuery(query.str())))
		return false;

	bool ret = false;
	do
	{
		uint32_t value = result->getDataInt("value"), param = result->getDataInt("param");
		if((ip & mask & param) == (value & param & mask))
		{
			int32_t expires = result->getDataLong("expires");
			if(expires > 0 && expires <= (int32_t)time(NULL))
				removeIpBanishment(value, param);
			else if(!ret)
				ret = true;
		}
	}
	while(result->next());
	result->free();
	return ret;
}

bool IOBan::isPlayerBanished(uint32_t playerId, PlayerBan_t type) const
{
	Database* db = Database::getInstance();
	DBResult* result;

	DBQuery query;
	query << "SELECT `id` FROM `bans` WHERE `type` = " << BAN_PLAYER << " AND `value` = " << playerId << " AND `param` = " << type << " AND `active` = 1 LIMIT 1";
	if(!(result = db->storeQuery(query.str())))
		return false;

	result->free();
	return true;
}

bool IOBan::isPlayerBanished(std::string name, PlayerBan_t type) const
{
	uint32_t _guid;
	return IOLoginData::getInstance()->getGuidByName(_guid, name)
		&& isPlayerBanished(_guid, type);
}

bool IOBan::isAccountBanished(uint32_t account, uint32_t playerId/* = 0*/) const
{
	Database* db = Database::getInstance();
	DBResult* result;

	DBQuery query;
	query << "SELECT `expires` FROM `bans` WHERE `type` = " << BAN_ACCOUNT << " AND `value` = " << account;
	if(playerId > 0)
		query << " AND `param` = " << playerId;

	query << " AND `active` = 1 LIMIT 1";
	if(!(result = db->storeQuery(query.str())))
		return false;

	const int32_t expires = result->getDataInt("expires");
	result->free();
	if(expires <= 0 || expires > (int32_t)time(NULL))
		return true;

	removeAccountBanishment(account);
	return false;
}

bool IOBan::addIpBanishment(uint32_t ip, int64_t banTime, uint32_t reasonId,
	std::string comment, uint32_t gamemaster, uint32_t mask/* = 0xFFFFFFFF*/, std::string statement/* = ""*/) const
{
	if(isIpBanished(ip))
		return false;

	PlayerVector players = g_game.getPlayersByIP(ip, mask);
	for(PlayerVector::iterator it = players.begin(); it != players.end(); ++it)
		(*it)->kick(true, true);

	Database* db = Database::getInstance();
	DBQuery query;

	query << "INSERT INTO `bans` (`id`, `type`, `value`, `param`, `expires`, `added`, `admin_id`, `comment`, `reason`, `statement`) ";
	query << "VALUES (NULL, " << BAN_IP << ", " << ip << ", " << mask << ", " << banTime << ", " << time(NULL) << ", " << gamemaster;
	query << ", " << db->escapeString(comment.c_str()) << ", " << reasonId << ", " << db->escapeString(statement.c_str()) << ")";
	return db->query(query.str());
}

bool IOBan::addPlayerBanishment(uint32_t playerId, int64_t banTime, uint32_t reasonId, ViolationAction_t actionId,
	std::string comment, uint32_t gamemaster, PlayerBan_t type, std::string statement/* = ""*/) const
{
	if(isPlayerBanished(playerId, type))
		return false;

	Database* db = Database::getInstance();
	DBQuery query;

	query << "INSERT INTO `bans` (`id`, `type`, `value`, `param`, `expires`, `added`, `admin_id`, `comment`, `reason`, `action`, `statement`) ";
	query << "VALUES (NULL, " << BAN_PLAYER << ", " << playerId << ", " << type << ", " << banTime << ", " << time(NULL) << ", " << gamemaster;
	query << ", " << db->escapeString(comment.c_str()) << ", " << reasonId << ", " << actionId << ", " << db->escapeString(statement.c_str()) << ")";
	return db->query(query.str());
}

bool IOBan::addPlayerBanishment(std::string name, int64_t banTime, uint32_t reasonId, ViolationAction_t actionId, std::string comment, uint32_t gamemaster, PlayerBan_t type, std::string statement/* = ""*/) const
{
	uint32_t _guid;
	return IOLoginData::getInstance()->getGuidByName(_guid, name) &&
		addPlayerBanishment(_guid, banTime, reasonId, actionId, comment, gamemaster, type, statement);
}

bool IOBan::addAccountBanishment(uint32_t account, int64_t banTime, uint32_t reasonId, ViolationAction_t actionId, std::string comment, uint32_t gamemaster, uint32_t playerId, std::string statement/* = ""*/) const
{
	if(isAccountBanished(account))
		return false;

	Database* db = Database::getInstance();
	DBQuery query;

	query << "INSERT INTO `bans` (`id`, `type`, `value`, `param`, `expires`, `added`, `admin_id`, `comment`, `reason`, `action`, `statement`) ";
	query << "VALUES (NULL, " << BAN_ACCOUNT << ", " << account << ", " << playerId << ", " << banTime << ", " << time(NULL) << ", " << gamemaster;
	query << ", " << db->escapeString(comment.c_str()) << ", " << reasonId << ", " << actionId << ", " << db->escapeString(statement.c_str()) << ")";
	return db->query(query.str());
}

bool IOBan::addNotation(uint32_t account, uint32_t reasonId, std::string comment, uint32_t gamemaster, uint32_t playerId, std::string statement/* = ""*/) const
{
	Database* db = Database::getInstance();
	DBQuery query;

	query << "INSERT INTO `bans` (`id`, `type`, `value`, `param`, `expires`, `added`, `admin_id`, `comment`, `reason`, `statement`) ";
	query << "VALUES (NULL, " << BAN_NOTATION << ", " << account << ", " << playerId << ", '-1', " << time(NULL) << ", " << gamemaster;
	query << ", " << db->escapeString(comment.c_str()) << ", " << reasonId << ", " << db->escapeString(statement.c_str()) << ")";
	return db->query(query.str());
}

bool IOBan::addStatement(uint32_t playerId, uint32_t reasonId, std::string comment, uint32_t gamemaster, int16_t channelId/* = -1*/, std::string statement/* = ""*/) const
{
	Database* db = Database::getInstance();
	DBQuery query;

	query << "INSERT INTO `bans` (`id`, `type`, `value`, ";
	if(channelId >= 0)
		query << "`param`, ";

	query << "`expires`, `added`, `admin_id`, `comment`, `reason`, `statement`) VALUES (NULL, " << BAN_STATEMENT << ", " << playerId;
	if(channelId >= 0)
		query << ", " << channelId;

	query << ", '-1', " << time(NULL) << ", " << gamemaster << ", " << db->escapeString(comment.c_str());
	query << ", " << reasonId << ", " << db->escapeString(statement.c_str()) << ")";
	return db->query(query.str());
}

bool IOBan::addStatement(std::string name, uint32_t reasonId, std::string comment, uint32_t gamemaster, int16_t channelId/* = -1*/, std::string statement/* = ""*/) const
{
	uint32_t _guid;
	return IOLoginData::getInstance()->getGuidByName(_guid, name) &&
		addStatement(_guid, reasonId, comment, gamemaster, channelId, statement);
}

bool IOBan::removeIpBanishment(uint32_t ip, uint32_t mask/* = 0xFFFFFFFF*/) const
{
	Database* db = Database::getInstance();
	DBQuery query;

	query << "UPDATE `bans` SET `active` = 0 WHERE `value` = " << ip << " AND `param` = " << mask
		<< " AND `type` = " << BAN_IP << " AND `active` = 1" << db->getUpdateLimiter();
	return db->query(query.str());
}

bool IOBan::removePlayerBanishment(uint32_t guid, PlayerBan_t type) const
{
	Database* db = Database::getInstance();
	DBQuery query;

	query << "UPDATE `bans` SET `active` = 0 WHERE `value` = " << guid << " AND `param` = " << type
		<< " AND `type` = " << BAN_PLAYER << " AND `active` = 1" << db->getUpdateLimiter();
	return db->query(query.str());
}

bool IOBan::removePlayerBanishment(std::string name, PlayerBan_t type) const
{
	uint32_t _guid;
	return IOLoginData::getInstance()->getGuidByName(_guid, name)
		&& removePlayerBanishment(_guid, type);
}

bool IOBan::removeAccountBanishment(uint32_t account, uint32_t playerId/* = 0*/) const
{
	Database* db = Database::getInstance();
	DBQuery query;

	query << "UPDATE `bans` SET `active` = 0 WHERE `value` = " << account;
	if(playerId > 0)
		query << " AND `param` = " << playerId;

	query << " AND `type` = " << BAN_ACCOUNT << " AND `active` = 1" << db->getUpdateLimiter();
	return db->query(query.str());
}

bool IOBan::removeNotations(uint32_t account, uint32_t playerId/* = 0*/) const
{
	Database* db = Database::getInstance();
	DBQuery query;

	query << "UPDATE `bans` SET `active` = 0 WHERE `value` = " << account;
	if(playerId > 0)
		query << " AND `param` = " << playerId;

	query << " AND `type` = " << BAN_NOTATION << " AND `active` = 1";
	return db->query(query.str());
}

bool IOBan::removeStatements(uint32_t playerId, int16_t channelId/* = -1*/) const
{
	Database* db = Database::getInstance();
	DBQuery query;

	query << "UPDATE `bans` SET `active` = 0 WHERE `value` = " << playerId;
	if(channelId >= 0)
		query << " AND `param` = " << channelId;

	query << " AND `type` = " << BAN_STATEMENT << " AND `active` = 1";
	return db->query(query.str());
}

bool IOBan::removeStatements(std::string name, int16_t channelId/* = -1*/) const
{
	uint32_t _guid;
	return IOLoginData::getInstance()->getGuidByName(_guid, name) && removeStatements(_guid, channelId);
}

uint32_t IOBan::getNotationsCount(uint32_t account, uint32_t playerId/* = 0*/) const
{
	Database* db = Database::getInstance();
	DBResult* result;

	DBQuery query;
	query << "SELECT COUNT(`id`) AS `count` FROM `bans` WHERE `value` = " << account;
	if(playerId > 0)
		query << " AND `param` = " << playerId;

	query << " AND `type` = " << BAN_NOTATION << " AND `active` = 1";
	if(!(result = db->storeQuery(query.str())))
		return 0;

	const uint32_t count = result->getDataInt("count");
	result->free();
	return count;
}

uint32_t IOBan::getStatementsCount(uint32_t playerId, int16_t channelId/* = -1*/) const
{
	Database* db = Database::getInstance();
	DBResult* result;

	DBQuery query;
	query << "SELECT COUNT(`id`) AS `count` FROM `bans` WHERE `value` = " << playerId;
	if(channelId >= 0)
		query << " AND `param` = " << channelId;

	query << " AND `type` = " << BAN_STATEMENT << " AND `active` = 1";
	if(!(result = db->storeQuery(query.str())))
		return 0;

	const uint32_t count = result->getDataInt("count");
	result->free();
	return count;
}

uint32_t IOBan::getStatementsCount(std::string name, int16_t channelId/* = -1*/) const
{
	uint32_t _guid;
	if(!IOLoginData::getInstance()->getGuidByName(_guid, name))
		return 0;

	return getStatementsCount(_guid, channelId);
}

bool IOBan::getData(Ban& ban) const
{
	Database* db = Database::getInstance();
	DBQuery query;

	query << "SELECT * FROM `bans` WHERE `value` = " << ban.value;
	if(ban.param)
		query << " AND `param` = " << ban.param;

	if(ban.type != BAN_NONE)
		query << " AND `type` = " << ban.type;

	query << " AND `active` = 1 AND (`expires` > " << time(NULL) << " OR `expires` <= 0) LIMIT 1";
	DBResult* result;
	if(!(result = db->storeQuery(query.str())))
		return false;

	ban.id = result->getDataInt("id");
	ban.type = (Ban_t)result->getDataInt("type");
	ban.value = result->getDataInt("value");
	ban.param = result->getDataInt("param");
	ban.expires = result->getDataLong("expires");
	ban.added = result->getDataLong("added");
	ban.adminId = result->getDataInt("admin_id");
	ban.comment = result->getDataString("comment");
	ban.reason = result->getDataInt("reason");
	ban.action = (ViolationAction_t)result->getDataInt("action");
	ban.statement = result->getDataString("statement");

	result->free();
	return true;
}

BansVec IOBan::getList(Ban_t type, uint32_t value/* = 0*/, uint32_t param/* = 0*/)
{
	Database* db = Database::getInstance();
	DBResult* result;

	DBQuery query;
	query << "SELECT * FROM `bans` WHERE ";
	if(value)
		query << "`value` = " << value << " AND ";

	if(param)
		query << "`param` = " << param << " AND ";

	query << "`type` = " << type << " AND `active` = 1 AND (`expires` > " << time(NULL) << " OR `expires` <= 0)";
	BansVec data;
	if((result = db->storeQuery(query.str())))
	{
		Ban tmp;
		do
		{
			tmp.id = result->getDataInt("id");
			tmp.type = (Ban_t)result->getDataInt("type");
			tmp.value = result->getDataInt("value");
			tmp.param = result->getDataInt("param");
			tmp.expires = result->getDataLong("expires");
			tmp.added = result->getDataLong("added");
			tmp.adminId = result->getDataInt("admin_id");
			tmp.comment = result->getDataString("comment");
			tmp.reason = result->getDataInt("reason");
			tmp.action = (ViolationAction_t)result->getDataInt("action");
			tmp.statement = result->getDataString("statement");
			data.push_back(tmp);
		}
		while(result->next());
		result->free();
	}

	return data;
}

bool IOBan::clearTemporials() const
{
	Database* db = Database::getInstance();
	DBQuery query;

	query << "UPDATE `bans` SET `active` = 0 WHERE `expires` <= " << time(NULL) << " AND `expires` >= 0 AND `active` = 1" << db->getUpdateLimiter();
	return db->query(query.str());
}
