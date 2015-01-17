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

#ifndef __OUTFIT__
#define __OUTFIT__

#include "otsystem.h"
#include "enums.h"
#include "const.h"
#include "tools.h"

enum AddonRequirement_t
{
	REQUIREMENT_NONE = 0,
	REQUIREMENT_FIRST,
	REQUIREMENT_SECOND,
	REQUIREMENT_BOTH,
	REQUIREMENT_ANY
};

struct Outfit
{
	Outfit()
	{
		memset(skills, 0, sizeof(skills));
		memset(skillsPercent, 0, sizeof(skillsPercent));
		memset(stats, 0, sizeof(stats));
		memset(statsPercent, 0, sizeof(statsPercent));

		memset(absorb, 0, sizeof(absorb));
		memset(reflect[REFLECT_PERCENT], 0, sizeof(reflect[REFLECT_PERCENT]));
		memset(reflect[REFLECT_CHANCE], 0, sizeof(reflect[REFLECT_CHANCE]));

		isDefault = true;
		requirement = REQUIREMENT_BOTH;
		isPremium = manaShield = invisible = regeneration = false;
		outfitId = lookType = addons = accessLevel = speed = attackSpeed = 0;
		healthGain = healthTicks = manaGain = manaTicks = conditionSuppressions = 0;
	}

	bool isDefault, isPremium, manaShield, invisible, regeneration;
	AddonRequirement_t requirement;
	int16_t absorb[COMBAT_LAST + 1], reflect[REFLECT_LAST + 1][COMBAT_LAST + 1];

	uint16_t accessLevel, addons;
	int32_t skills[SKILL_LAST + 1], skillsPercent[SKILL_LAST + 1], stats[STAT_LAST + 1], statsPercent[STAT_LAST + 1],
		speed, attackSpeed, healthGain, healthTicks, manaGain, manaTicks, conditionSuppressions;

	uint32_t outfitId, lookType;
	std::string name, storageId, storageValue;
	IntegerVec groups;
};

typedef std::list<Outfit> OutfitList;
typedef std::map<uint32_t, Outfit> OutfitMap;

class Outfits
{
	public:
		virtual ~Outfits() {}
		static Outfits* getInstance()
		{
			static Outfits instance;
			return &instance;
		}

		bool loadFromXml();
		bool parseOutfitNode(xmlNodePtr p);

		const OutfitMap& getOutfits(uint16_t sex) {return outfitsMap[sex];}

		bool getOutfit(uint32_t outfitId, uint16_t sex, Outfit& outfit);
		bool getOutfit(uint32_t lookType, Outfit& outfit);

		bool addAttributes(uint32_t playerId, uint32_t outfitId, uint16_t sex, uint16_t addons);
		bool removeAttributes(uint32_t playerId, uint32_t outfitId, uint16_t sex);

		uint32_t getOutfitId(uint32_t lookType);

		int16_t getOutfitAbsorb(uint32_t lookType, uint16_t sex, CombatType_t combat);
		int16_t getOutfitReflect(uint32_t lookType, uint16_t sex, CombatType_t combat);

	private:
		Outfits() {}

		OutfitList allOutfits;
		std::map<uint16_t, OutfitMap> outfitsMap;
};
#endif
