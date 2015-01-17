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

#ifndef __SPELLS__
#define __SPELLS__
#include "otsystem.h"

#include "enums.h"
#include "player.h"
#include "luascript.h"

#include "baseevents.h"
#include "actions.h"
#include "talkaction.h"

class InstantSpell;
class ConjureSpell;
class RuneSpell;
class Spell;

typedef std::map<uint32_t, RuneSpell*> RunesMap;
typedef std::map<std::string, InstantSpell*> InstantsMap;
typedef std::map<SpellGroup_t, uint32_t> SpellGroup;

class Spells : public BaseEvents
{
	public:
		Spells();
		virtual ~Spells() {clear();}

		Spell* getSpellByName(const std::string& name);

		RuneSpell* getRuneSpell(uint32_t id);
		RuneSpell* getRuneSpellByName(const std::string& name);

		InstantSpell* getInstantSpell(const std::string& words);
		InstantSpell* getInstantSpellByName(const std::string& name);
		InstantSpell* getInstantSpellByIndex(const Player* player, uint32_t index);

		uint32_t getInstantSpellCount(const Player* player);
		ReturnValue onPlayerSay(Player* player, const std::string& words);
		virtual std::string getScriptBaseName() const {return "spells";}
		static Position getCasterPosition(Creature* creature, Direction dir);

	protected:
		virtual void clear();

		virtual Event* getEvent(const std::string& nodeName);
		virtual bool registerEvent(Event* event, xmlNodePtr p, bool override);

		virtual LuaInterface& getInterface() {return m_interface;}
		LuaInterface m_interface;

		RunesMap runes;
		InstantsMap instants;

		uint32_t spellId;
		friend class CombatSpell;
};

typedef bool (InstantSpellFunction)(const InstantSpell* spell, Creature* creature, const std::string& param);
typedef bool (ConjureSpellFunction)(const ConjureSpell* spell, Creature* creature, const std::string& param);
typedef bool (RuneSpellFunction)(const RuneSpell* spell, Creature* creature, Item* item, const Position& posFrom, const Position& posTo);

class BaseSpell
{
	public:
		BaseSpell() {}
		virtual ~BaseSpell() {}

		virtual bool castSpell(Creature* creature);
		virtual bool castSpell(Creature* creature, Creature* target);
};

class CombatSpell : public Event, public BaseSpell
{
	public:
		CombatSpell(Combat* _combat, bool _needTarget, bool _needDirection);
		virtual ~CombatSpell();

		virtual bool castSpell(Creature* creature);
		virtual bool castSpell(Creature* creature, Creature* target);
		virtual bool configureEvent(xmlNodePtr) {return true;}

		//scripting
		bool executeCastSpell(Creature* creature, const LuaVariant& var);

		bool loadScriptCombat();
		Combat* getCombat() {return combat;}

	protected:
		virtual std::string getScriptEventName() const {return "onCastSpell";}
		virtual std::string getScriptEventParams() const {return "cid, var";}

		bool needDirection;
		bool needTarget;
		Combat* combat;
};

class Spell : public BaseSpell
{
	public:
		Spell();
		virtual ~Spell() {}

		bool configureSpell(xmlNodePtr xmlspell);
		const std::string& getName() const {return name;}

		void postSpell(Player* player) const;
		void postSpell(Player* player, uint32_t manaCost, uint32_t soulCost) const;

		int32_t getManaCost(const Player* player) const;
		int32_t getSoulCost() const {return soul;}
		uint32_t getLevel() const {return level;}
		int32_t getMagicLevel() const {return magLevel;}
		int32_t getMana() const {return mana;}
		int32_t getManaPercent() const {return manaPercent;}
		uint32_t getExhaustion() const {return exhaustion;}
		SpellGroup getGroupExhaustions() const {return groupExhaustions;}
		uint16_t getId() const {return spellId;}
		void setId(uint16_t id) {spellId = id;}

		bool isEnabled() const {return enabled;}
		bool isPremium() const {return premium;}

		virtual bool isInstant() const = 0;
		bool isLearnable() const {return learnable;}

		static ReturnValue CreateIllusion(Creature* creature, const Outfit_t& outfit, int32_t time);
		static ReturnValue CreateIllusion(Creature* creature, const std::string& name, int32_t time);
		static ReturnValue CreateIllusion(Creature* creature, uint32_t itemId, int32_t time);

	protected:
		bool checkSpell(Player* player) const;
		bool checkInstantSpell(Player* player, Creature* creature);
		bool checkInstantSpell(Player* player, const Position& toPos);
		bool checkRuneSpell(Player* player, const Position& toPos);

	private:
		uint16_t spellId;

	protected:
		int32_t level;
		int32_t magLevel;
		int32_t skills[SKILL_LAST + 1];

		bool premium;
		bool learnable;
		bool enabled;

		int32_t mana;
		int32_t manaPercent;
		int32_t soul;
		int32_t range;
		uint32_t exhaustion;

		bool needTarget;
		bool needWeapon;
		bool blockingSolid;
		bool blockingCreature;
		bool selfTarget;
		bool isAggressive;

		SpellGroup groupExhaustions;
		std::string name;

		VocationMap vocSpellMap;
		StringVec vocStringVec;
};

class InstantSpell : public TalkAction, public Spell
{
	public:
		InstantSpell(LuaInterface* _interface);
		virtual ~InstantSpell() {}

		virtual bool configureEvent(xmlNodePtr p);
		virtual bool loadFunction(const std::string& functionName);

		virtual bool castInstant(Player* player, const std::string& param);

		virtual bool castSpell(Creature* creature);
		virtual bool castSpell(Creature* creature, Creature* target);

		//scripting
		bool executeCastSpell(Creature* creature, const LuaVariant& var);

		virtual bool isInstant() const {return true;}
		bool getHasParam() const {return hasParam;}
		bool canCast(const Player* player) const;
		bool canThrowSpell(const Creature* creature, const Creature* target) const;

	protected:
		virtual std::string getScriptEventName() const {return "onCastSpell";}
		virtual std::string getScriptEventParams() const {return "cid, var";}

		static InstantSpellFunction SearchPlayer;
		static InstantSpellFunction SummonMonster;
		static InstantSpellFunction Levitate;
		static InstantSpellFunction Illusion;

		bool internalCastSpell(Creature* creature, const LuaVariant& var);

		bool needDirection;
		bool hasParam;
		bool checkLineOfSight;
		bool casterTargetOrDirection;
		uint8_t limitRange;

		InstantSpellFunction* function;
};

class ConjureSpell : public InstantSpell
{
	public:
		ConjureSpell(LuaInterface* _interface);
		virtual ~ConjureSpell() {}

		virtual bool configureEvent(xmlNodePtr p);
		virtual bool loadFunction(const std::string& functionName);

		virtual bool castInstant(Player* player, const std::string& param);

		virtual bool castSpell(Creature*) {return false;}
		virtual bool castSpell(Creature*, Creature*) {return false;}

		uint32_t getConjureId() const {return conjureId;}
		uint32_t getConjureCount() const {return conjureCount;}
		uint32_t getReagentId() const {return conjureReagentId;}

	protected:
		virtual std::string getScriptEventName() const {return "onCastSpell";}
		virtual std::string getScriptEventParams() const {return "cid, var";}

		static ReturnValue internalConjureItem(Player* player, uint32_t conjureId, uint32_t conjureCount,
			bool transform = false, uint32_t reagentId = 0);

		static ConjureSpellFunction ConjureItem;

		bool internalCastSpell(Creature* creature, const LuaVariant& var);
		Position getCasterPosition(Creature* creature);

		ConjureSpellFunction* function;

		uint32_t conjureId;
		uint32_t conjureCount;
		uint32_t conjureReagentId;
};

class RuneSpell : public Action, public Spell
{
	public:
		RuneSpell(LuaInterface* _interface);
		virtual ~RuneSpell() {}

		virtual bool configureEvent(xmlNodePtr p);
		virtual bool loadFunction(const std::string& functionName);

		virtual ReturnValue canExecuteAction(const Player* player, const Position& toPos);
		virtual bool hasOwnErrorHandler() {return true;}

		virtual bool executeUse(Player* player, Item* item, const PositionEx& posFrom,
			const PositionEx& posTo, bool extendedUse, uint32_t creatureId);

		virtual bool castSpell(Creature* creature);
		virtual bool castSpell(Creature* creature, Creature* target);

		//scripting
		bool executeCastSpell(Creature* creature, const LuaVariant& var);
		virtual bool isInstant() const {return false;}
		uint32_t getRuneItemId() {return runeId;}

	protected:
		virtual std::string getScriptEventName() const {return "onCastSpell";}
		virtual std::string getScriptEventParams() const {return "cid, var";}

		static RuneSpellFunction Illusion;
		static RuneSpellFunction Convince;
		static RuneSpellFunction Soulfire;

		bool internalCastSpell(Creature* creature, const LuaVariant& var);

		bool hasCharges;
		uint32_t runeId;

		RuneSpellFunction* function;
};
#endif
