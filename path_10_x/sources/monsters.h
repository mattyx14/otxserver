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

#ifndef __MONSTERS__
#define __MONSTERS__
#include "otsystem.h"

#include "creature.h"
#define MAX_LOOTCHANCE 100000
#define MAX_STATICWALK 100

struct LootBlock;
typedef std::list<LootBlock> LootItems;

enum LootMessage_t
{
	LOOTMSG_IGNORE = -1,
	LOOTMSG_NONE = 0,
	LOOTMSG_PLAYER = 1,
	LOOTMSG_PARTY = 2,
	LOOTMSG_BOTH = 3
};

struct LootBlock
{
	std::vector<uint16_t> ids;
	uint16_t count;
	int32_t subType, actionId, uniqueId;
	uint32_t chance;
	std::string text;
	LootItems childLoot;

	LootBlock()
	{
		count = 1;
		chance = 0;
		subType = actionId = uniqueId = -1;
	}
};

struct summonBlock_t
{
	std::string name;
	uint32_t chance, interval, amount;
};

class BaseSpell;

struct spellBlock_t
{
	bool combatSpell, isMelee;
	int32_t minCombatValue, maxCombatValue;
	uint32_t chance, speed, range;
	BaseSpell* spell;
};

struct voiceBlock_t
{
	bool yellText;
	std::string text;
};

typedef std::list<summonBlock_t> SummonList;
typedef std::list<spellBlock_t> SpellList;
typedef std::vector<voiceBlock_t> VoiceVector;
typedef std::map<CombatType_t, int32_t> ElementMap;

class MonsterType
{
	public:
		MonsterType() {reset();}
		virtual ~MonsterType() {reset();}

		void reset();

		void dropLoot(Container* corpse);
		ItemList createLoot(const LootBlock& lootBlock);
		bool createChildLoot(Container* parent, const LootBlock& lootBlock);

		bool isSummonable, isIllusionable, isConvinceable, isAttackable, isHostile, isLureable,
			isWalkable, canPushItems, canPushCreatures, pushable, hideName, hideHealth, eliminable;

		Outfit_t outfit;
		RaceType_t race;
		Skulls_t skull;
		PartyShields_t partyShield;
		GuildEmblems_t guildEmblem;
		LootMessage_t lootMessage;

		int32_t defense, armor, health, healthMin, healthMax, baseSpeed, lookCorpse, corpseUnique, corpseAction,
			maxSummons, targetDistance, runAwayHealth, conditionImmunities, damageImmunities,
			lightLevel, lightColor, changeTargetSpeed, changeTargetChance;
		uint32_t yellChance, yellSpeedTicks, staticAttackChance, manaCost;
		uint64_t experience;

		std::string name, nameDescription, file;

		SummonList summonList;
		LootItems lootItems;
		ElementMap elementMap;
		SpellList spellAttackList;
		SpellList spellDefenseList;
		VoiceVector voiceVector;
		StringVec scriptList;
};

class Monsters
{
	public:
		Monsters(): loaded(false) {}
		virtual ~Monsters();

		bool reload() {return loadFromXml(true);}
		bool loadFromXml(bool reloading = false);

		bool loadMonster(const std::string& file, const std::string& monsterName, bool reloading = false);

		MonsterType* getMonsterType(const std::string& name);
		MonsterType* getMonsterType(uint32_t mid);

		uint32_t getIdByName(const std::string& name);
		bool isLoaded() const {return loaded;}
		static uint16_t getLootRandom();

	private:
		bool loaded;

		bool loadLoot(xmlNodePtr, LootBlock&);
		bool loadChildLoot(xmlNodePtr, LootBlock&);

		ConditionDamage* getDamageCondition(ConditionType_t conditionType,
			int32_t maxDamage, int32_t minDamage, int32_t startDamage, uint32_t tickInterval);
		bool deserializeSpell(xmlNodePtr node, spellBlock_t& sb, const std::string& description = "");

		typedef std::map<std::string, uint32_t> MonsterNameMap;
		MonsterNameMap monsterNames;

		typedef std::map<uint32_t, MonsterType*> MonsterMap;
		MonsterMap monsters;
};
#endif
