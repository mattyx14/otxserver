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

#ifndef __IOGUILD__
#define __IOGUILD__
#include "otsystem.h"
#include "enums.h"

struct DeathEntry;
typedef std::vector<DeathEntry> DeathList;

class Player;
class IOGuild
{
	public:
		virtual ~IOGuild() {}
		static IOGuild* getInstance()
		{
			static IOGuild instance;
			return &instance;
		}

		bool guildExists(uint32_t guild);
		bool createGuild(Player* player);
		bool disbandGuild(uint32_t guild);

		std::string getMotd(uint32_t guild);
		bool setMotd(uint32_t guild, const std::string& newMessage);

		GuildLevel_t getGuildLevel(uint32_t guid);
		bool setGuildLevel(uint32_t guid, GuildLevel_t level);

		bool invitePlayer(uint32_t gulid, uint32_t guid);
		bool revokeInvite(uint32_t guild, uint32_t guid);
		bool joinGuild(Player* player, uint32_t guildId, bool creation = false);

		std::string getRank(uint32_t guid);
		bool changeRank(uint32_t guild, const std::string& oldName, const std::string& newName);

		bool hasGuild(uint32_t guid);
		bool isInvited(uint32_t guild, uint32_t guid);

		bool getGuildId(uint32_t& id, const std::string& name);
		bool getGuildById(std::string& name, uint32_t id);

		uint32_t getRankIdByLevel(uint32_t guild, GuildLevel_t level);
		uint32_t getRankIdByName(uint32_t guild, const std::string& name);
		bool getRankEx(uint32_t& id, std::string& name, uint32_t guild, GuildLevel_t level);

		uint32_t getGuildId(uint32_t guid);
		bool setGuildNick(uint32_t guid, const std::string& nick);

		bool swapGuildIdToOwner(uint32_t& value);
		bool updateOwnerId(uint32_t guild, uint32_t guid);

		void checkWars();
		void checkEndingWars();
		bool updateWar(War_t& enemy);
		void finishWar(War_t enemy, bool finished);
		void frag(Player* player, uint64_t deathId, const DeathList& list, bool score);

	private:
		IOGuild() {}
};
#endif
