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

#ifndef __MOUNTS__
#define __MOUNTS__
#include "otsystem.h"

#include "networkmessage.h"
#include "player.h"

class Mount
{
	public:
		Mount(std::string _name, uint16_t _id, uint16_t _clientId, int32_t _speed, int32_t _attackSpeed,
			bool _premium, std::string _storageId, std::string _storageValue)
		{
			name = _name;
			storageId = _storageId;
			storageValue = _storageValue;

			speed = _speed;
			attackSpeed = _attackSpeed;

			clientId = _clientId;
			id = _id;
			premium = _premium;

			manaShield = invisible = regeneration = false;
			healthGain = healthTicks = manaGain = manaTicks = conditionSuppressions = 0;

			memset(skills, 0, sizeof(skills));
			memset(skillsPercent, 0, sizeof(skillsPercent));
			memset(stats, 0 , sizeof(stats));
			memset(statsPercent, 0, sizeof(statsPercent));

			memset(absorb, 0, sizeof(absorb));
			memset(reflect[REFLECT_PERCENT], 0, sizeof(reflect[REFLECT_PERCENT]));
			memset(reflect[REFLECT_CHANCE], 0, sizeof(reflect[REFLECT_CHANCE]));
		}

		bool isTamed(Player* player) const;
		void addAttributes(Player* player);
		void removeAttributes(Player* player);

		uint16_t getId() const {return id;}
		uint16_t getClientId() const {return clientId;}

		const std::string& getName() const {return name;}
		bool isPremium() const {return premium;}

		uint32_t getSpeed() const {return speed;}
		int32_t getAttackSpeed() const {return attackSpeed;}

	private:
		std::string name, storageId, storageValue;
		int32_t speed, attackSpeed;
		uint16_t clientId;
		uint8_t id;
		bool premium, manaShield, invisible, regeneration;

		int16_t absorb[COMBAT_LAST + 1], reflect[REFLECT_LAST + 1][COMBAT_LAST + 1];
		int32_t skills[SKILL_LAST + 1], skillsPercent[SKILL_LAST + 1], stats[STAT_LAST + 1], statsPercent[STAT_LAST + 1],
			healthGain, healthTicks, manaGain, manaTicks, conditionSuppressions;

		friend class Mounts;
		friend class Player;
};

typedef std::list<Mount*> MountList;
class Mounts
{
	public:
		virtual ~Mounts() {clear();}
		static Mounts* getInstance()
		{
			static Mounts instance;
			return &instance;
		}

		void clear();
		bool reload();

		bool loadFromXml();
		bool parseMountNode(xmlNodePtr p);

		inline MountList::const_iterator getFirstMount() const {return mounts.begin();}
		inline MountList::const_iterator getLastMount() const {return mounts.end();}

		Mount* getMountById(uint16_t id) const;
		Mount* getMountByCid(uint16_t id) const;

		uint8_t getMountCount() const {return (uint8_t)mounts.size();}
		bool isPremium() const;

	private:
		MountList mounts;
};
#endif
