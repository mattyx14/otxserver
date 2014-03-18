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

#ifndef __IOBAN__
#define __IOBAN__
#include "otsystem.h"

#include "enums.h"

enum Ban_t
{
	BAN_NONE = 0,
	BAN_IP = 1,
	BAN_PLAYER = 2,
	BAN_ACCOUNT = 3,
	BAN_NOTATION = 4,
	BAN_STATEMENT = 5
};

enum PlayerBan_t
{
	PLAYERBAN_NONE = 0,
	PLAYERBAN_REPORT = 1,
	PLAYERBAN_LOCK = 2,
	PLAYERBAN_BANISHMENT = 3
};

struct Ban
{
	Ban_t type;
	ViolationAction_t action;
	uint32_t id, value, param, added, adminId, reason;
	int32_t expires;
	std::string comment, statement;

	Ban()
	{
		type = BAN_NONE;
		action = ACTION_PLACEHOLDER;
		id = value = param = added = adminId = reason = expires = 0;
	}
};

typedef std::vector<Ban> BansVec;

class IOBan
{
	protected:
		IOBan() {}

	public:
		virtual ~IOBan() {}
		static IOBan* getInstance()
		{
			static IOBan instance;
			return &instance;
		}

		bool isIpBanished(uint32_t ip, uint32_t mask = 0xFFFFFFFF) const;
		bool isPlayerBanished(uint32_t guid, PlayerBan_t type) const;
		bool isPlayerBanished(std::string name, PlayerBan_t type) const;
		bool isAccountBanished(uint32_t account, uint32_t playerId = 0) const;

		bool addIpBanishment(uint32_t ip, int64_t banTime, uint32_t reasonId,
			std::string comment, uint32_t gamemaster, uint32_t mask = 0xFFFFFFFF, std::string statement = "") const;
		bool addPlayerBanishment(uint32_t playerId, int64_t banTime, uint32_t reasonId, ViolationAction_t actionId,
			std::string comment, uint32_t gamemaster, PlayerBan_t type, std::string statement = "") const;
		bool addPlayerBanishment(std::string name, int64_t banTime, uint32_t reasonId, ViolationAction_t actionId,
			std::string comment, uint32_t gamemaster, PlayerBan_t type, std::string statement = "") const;
		bool addAccountBanishment(uint32_t account, int64_t banTime, uint32_t reasonId, ViolationAction_t actionId,
			std::string comment, uint32_t gamemaster, uint32_t playerId, std::string statement = "") const;
		bool addNotation(uint32_t account, uint32_t reasonId,
			std::string comment, uint32_t gamemaster, uint32_t playerId, std::string statement = "") const;
		bool addStatement(uint32_t playerId, uint32_t reasonId,
			std::string comment, uint32_t gamemaster, int16_t channelId = -1, std::string statement = "") const;
		bool addStatement(std::string name, uint32_t reasonId,
			std::string comment, uint32_t gamemaster, int16_t channelId = -1, std::string statement = "") const;

		bool removeIpBanishment(uint32_t ip, uint32_t mask = 0xFFFFFFFF) const;
		bool removePlayerBanishment(uint32_t guid, PlayerBan_t type) const;
		bool removePlayerBanishment(std::string name, PlayerBan_t type) const;
		bool removeAccountBanishment(uint32_t account, uint32_t playerId = 0) const;
		bool removeNotations(uint32_t account, uint32_t playerId = 0) const;
		bool removeStatements(uint32_t playerId, int16_t channelId = -1) const;
		bool removeStatements(std::string name, int16_t channelId = -1) const;

		bool getData(Ban& ban) const;
		std::vector<Ban> getList(Ban_t type, uint32_t value = 0, uint32_t param = 0);

		uint32_t getNotationsCount(uint32_t account, uint32_t playerId = 0) const;
		uint32_t getStatementsCount(uint32_t playerId, int16_t channelId = -1) const;
		uint32_t getStatementsCount(std::string name, int16_t channelId = -1) const;

		bool clearTemporials() const;
};
#endif
