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

#ifndef __RAIDS__
#define __RAIDS__
#include "otsystem.h"
#include "const.h"

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "baseevents.h"
#include "position.h"
#include "tools.h"

enum RefType_t
{
	REF_NONE = 0,
	REF_SINGLE,
	REF_BLOCK
};

struct MonsterSpawn
{
	std::string name;
	uint32_t min, max;
};

class Raid;
class RaidEvent;

typedef std::list<Raid*> RaidList;
typedef std::vector<RaidEvent*> RaidEventVector;
typedef std::list<MonsterSpawn*> MonsterSpawnList;

#define MAXIMUM_TRIES_PER_MONSTER 10
#define CHECK_RAIDS_INTERVAL 60
#define MAX_RAND_RANGE 10000000
#define MAX_ITEM_CHANCE 100000
#define RAID_MINTICKS 100

class Raids
{
	public:
		virtual ~Raids() {clear();}
		static Raids* getInstance()
		{
			static Raids instance;
			return &instance;
		}

		bool loadFromXml();
		bool parseRaidNode(xmlNodePtr raidNode, bool checkDuplicate, FileType_t pathing);

		bool startup();
		void checkRaids();

		void clear();
		bool reload();

		Raid* getRunning() const {return running;}
		void setRunning(Raid* newRunning) {running = newRunning;}

		uint64_t getLastRaidEnd() const {return lastRaidEnd;}
		void setLastRaidEnd(uint64_t newLastRaidEnd) {lastRaidEnd = newLastRaidEnd;}

		bool isLoaded() const {return loaded;}
		bool isStarted() const {return started;}

		std::string getScriptBaseName() const {return "raids";}
		Raid* getRaidByName(const std::string& name);

	private:
		Raids();
		bool loaded, started;

		RaidList raidList;
		Raid* running;

		uint32_t checkRaidsEvent;
		uint64_t lastRaidEnd;
};

class Raid
{
	public:
		Raid(const std::string& _name, uint32_t _interval, uint64_t _margin,
			RefType_t _refType, bool _ref, bool _enabled);
		virtual ~Raid();

		bool loadFromXml(const std::string& _filename);

		bool startRaid();
		bool resetRaid(bool checkExecution);

		bool executeRaidEvent(RaidEvent* raidEvent);
		void stopEvents();

		RaidEvent* getNextRaidEvent();
		std::string getName() const {return name;}

		uint64_t getMargin() const {return margin;}
		uint32_t getInterval() const {return interval;}

		bool isLoaded() const {return loaded;}
		bool isEnabled() const {return enabled;}

		bool usesRef() const {return refType != REF_NONE;}
		bool hasRef() const {return refCount > 0;}

		void addRef() {++refCount;}
		void unRef() {--refCount; if(refCount <= 0) resetRaid(true);}

	private:
		std::string name;
		uint32_t interval;
		uint64_t margin;
		RefType_t refType;
		bool ref, enabled;

		bool loaded;
		uint32_t refCount, eventCount, nextEvent;
		RaidEventVector raidEvents;
};

class RaidEvent
{
	public:
		RaidEvent(Raid* raid, bool ref): m_delay(RAID_MINTICKS),
			m_ref(ref), m_raid(raid) {}
		virtual ~RaidEvent() {}

		virtual bool configureRaidEvent(xmlNodePtr eventNode);
		virtual bool executeEvent(const std::string&) const {return false;}

		uint32_t getDelay() const {return m_delay;}
		static bool compareEvents(const RaidEvent* lhs, const RaidEvent* rhs)
		{
			return lhs->getDelay() < rhs->getDelay();
		}

	private:
		uint32_t m_delay;

	protected:
		bool m_ref;
		Raid* m_raid;
};

class AnnounceEvent : public RaidEvent
{
	public:
		AnnounceEvent(Raid* raid, bool ref): RaidEvent(raid, ref),
			m_messageType(MSG_EVENT_ADVANCE) {}
		virtual ~AnnounceEvent() {}

		virtual bool configureRaidEvent(xmlNodePtr eventNode);
		virtual bool executeEvent(const std::string& name) const;

	private:
		std::string m_message;
		MessageClasses m_messageType;
};

class EffectEvent : public RaidEvent
{
	public:
		EffectEvent(Raid* raid, bool ref): RaidEvent(raid, ref),
			m_effect(MAGIC_EFFECT_NONE) {}
		virtual ~EffectEvent() {}

		virtual bool configureRaidEvent(xmlNodePtr eventNode);
		virtual bool executeEvent(const std::string& name) const;

	private:
		MagicEffect_t m_effect;
		Position m_position;
};

class ItemSpawnEvent : public RaidEvent
{
	public:
		ItemSpawnEvent(Raid* raid, bool ref): RaidEvent(raid, ref),
			m_itemId(0), m_subType(-1), m_chance(MAX_ITEM_CHANCE) {}
		virtual ~ItemSpawnEvent() {}

		virtual bool configureRaidEvent(xmlNodePtr eventNode);
		virtual bool executeEvent(const std::string& name) const;

	private:
		int16_t m_itemId;
		int32_t m_subType;

		uint32_t m_chance;
		Position m_position;
};

class SingleSpawnEvent : public RaidEvent
{
	public:
		SingleSpawnEvent(Raid* raid, bool ref): RaidEvent(raid, ref) {}
		virtual ~SingleSpawnEvent() {}

		virtual bool configureRaidEvent(xmlNodePtr eventNode);
		virtual bool executeEvent(const std::string& name) const;

	private:
		std::string m_monsterName;
		Position m_position;
};

class AreaSpawnEvent : public RaidEvent
{
	public:
		AreaSpawnEvent(Raid* raid, bool ref): RaidEvent(raid, ref) {}
		virtual ~AreaSpawnEvent();

		virtual bool configureRaidEvent(xmlNodePtr eventNode);
		virtual bool executeEvent(const std::string& name) const;

		void addMonster(MonsterSpawn* _spawn);
		void addMonster(const std::string& name, uint32_t min, uint32_t max);

	private:
		MonsterSpawnList m_spawnList;
		Position m_fromPos, m_toPos;
};

class ScriptEvent : public RaidEvent, public Event
{
	public:
		ScriptEvent(Raid* raid, bool ref): RaidEvent(raid, ref),
			Event(&m_interface) {}
		virtual ~ScriptEvent() {}

		virtual bool configureRaidEvent(xmlNodePtr eventNode);
		virtual bool executeEvent(const std::string& name) const;

		virtual bool configureEvent(xmlNodePtr) {return false;}
		static LuaInterface m_interface;

	protected:
		virtual std::string getScriptEventName() const {return "onRaid";}
		virtual std::string getScriptEventParams() const {return "";}
};
#endif
