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
	BAN_NOTATION = 4
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
	uint32_t id, value, param, added, adminId;
	int32_t expires;
	std::string comment;

	Ban()
	{
		type = BAN_NONE;
		id = value = param = added = adminId = expires = 0;
	}
};

typedef std::vector<Ban> BansVec;

class IOBan
{
	protected:
		IOBan() {}

	public:
		~IOBan() {}
		static IOBan* getInstance()
		{
			static IOBan instance;
			return &instance;
		}

		bool isIpBanished(uint32_t ip, uint32_t mask = 0xFFFFFFFF) const;
		bool isPlayerBanished(uint32_t guid, PlayerBan_t type) const;
		bool isPlayerBanished(std::string name, PlayerBan_t type) const;
		bool isAccountBanished(uint32_t account, uint32_t playerId = 0) const;

		bool addIpBanishment(uint32_t ip, int64_t banTime, std::string comment, uint32_t gamemaster, uint32_t mask = 0xFFFFFFFF) const;
		bool addPlayerBanishment(uint32_t playerId, int64_t banTime, std::string comment, uint32_t gamemaster, PlayerBan_t type) const;
		bool addPlayerBanishment(std::string name, int64_t banTime, std::string comment, uint32_t gamemaster, PlayerBan_t type) const;
		bool addAccountBanishment(uint32_t account, int64_t banTime, std::string comment, uint32_t gamemaster, uint32_t playerId) const;
		bool addNotation(uint32_t account, std::string comment, uint32_t gamemaster, uint32_t playerId) const;

		bool removeIpBanishment(uint32_t ip, uint32_t mask = 0xFFFFFFFF) const;
		bool removePlayerBanishment(uint32_t guid, PlayerBan_t type) const;
		bool removePlayerBanishment(std::string name, PlayerBan_t type) const;
		bool removeAccountBanishment(uint32_t account, uint32_t playerId = 0) const;
		bool removeNotations(uint32_t account, uint32_t playerId = 0) const;

		bool getData(Ban& ban) const;
		std::vector<Ban> getList(Ban_t type, uint32_t value = 0, uint32_t param = 0);

		uint32_t getNotationsCount(uint32_t account, uint32_t playerId = 0) const;
};
#endif
