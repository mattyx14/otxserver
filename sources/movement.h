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

#ifndef __MOVEMENT__
#define __MOVEMENT__

#include "baseevents.h"
#include "creature.h"

class MoveEvent;
class MoveEventScript : public LuaInterface
{
	public:
		MoveEventScript() : LuaInterface("MoveEvents Interface") {}
		virtual ~MoveEventScript() {}

		static MoveEvent* event;

	protected:
		virtual void registerFunctions();
		static int32_t luaCallFunction(lua_State* L);
};

enum MoveEvent_t
{
	MOVE_EVENT_FIRST = 0,
	MOVE_EVENT_STEP_IN = MOVE_EVENT_FIRST,
	MOVE_EVENT_STEP_OUT = 1,
	MOVE_EVENT_EQUIP = 2,
	MOVE_EVENT_DE_EQUIP = 3,
	MOVE_EVENT_ADD_ITEM = 4,
	MOVE_EVENT_REMOVE_ITEM = 5,
	MOVE_EVENT_ADD_TILEITEM = 6,
	MOVE_EVENT_REMOVE_TILEITEM = 7,
	MOVE_EVENT_NONE = 8,
	MOVE_EVENT_LAST = MOVE_EVENT_REMOVE_TILEITEM
};

typedef std::list<MoveEvent*> EventList;
class MoveEvents : public BaseEvents
{
	public:
		MoveEvents();
		virtual ~MoveEvents() {clear();}

		uint32_t onCreatureMove(Creature* actor, Creature* creature, const Tile* fromTile, const Tile* toTile, bool isStepping);
		bool onPlayerEquip(Player* player, Item* item, slots_t slot, bool isCheck);
		bool onPlayerDeEquip(Player* player, Item* item, slots_t slot, bool isRemoval);
		uint32_t onItemMove(Creature* actor, Item* item, Tile* tile, bool isAdd);

		MoveEvent* getEvent(Item* item, uint16_t uniqueId, uint16_t actionId, MoveEvent_t eventType);
		bool hasEquipEvent(Item* item);
		bool hasTileEvent(Item* item);

		void onRemoveTileItem(const Tile* tile, Item* item);
		void onAddTileItem(const Tile* tile, Item* item);

	protected:
		struct MoveEventList
		{
			EventList moveEvent[MOVE_EVENT_NONE];
		};

		virtual std::string getScriptBaseName() const {return "movements";}
		virtual void clear();

		virtual Event* getEvent(const std::string& nodeName);
		virtual bool registerEvent(Event* event, xmlNodePtr p, bool override);

		virtual LuaInterface& getInterface() {return m_interface;}
		MoveEventScript m_interface;

		void registerItemID(int32_t itemId, MoveEvent_t eventType);
		void registerActionID(int32_t actionId, MoveEvent_t eventType);
		void registerUniqueID(int32_t uniqueId, MoveEvent_t eventType);

		typedef std::map<int32_t, MoveEventList> MoveListMap;
		MoveListMap m_itemIdMap;
		MoveListMap m_uniqueIdMap;
		MoveListMap m_actionIdMap;

		typedef std::map<Position, MoveEventList> MovePosListMap;
		MovePosListMap m_positionMap;
		void clearMap(MoveListMap& map);

		void addEvent(MoveEvent* moveEvent, int32_t id, MoveListMap& map, bool override);
		MoveEvent* getEvent(Item* item, MoveEvent_t eventType, slots_t slot);

		void addEvent(MoveEvent* moveEvent, Position pos, MovePosListMap& map, bool override);
		MoveEvent* getEvent(const Tile* tile, MoveEvent_t eventType);

		const Tile* m_lastCacheTile;
		std::vector<Item*> m_lastCacheItemVector;
};

typedef uint32_t (MoveFunction)(Item* item);
typedef uint32_t (StepFunction)(Creature* creature, Item* item);
typedef bool (EquipFunction)(MoveEvent* moveEvent, Player* player, Item* item, slots_t slot, bool boolean);

class MoveEvent : public Event
{
	public:
		MoveEvent(LuaInterface* _interface);
		MoveEvent(const MoveEvent* copy);
		virtual ~MoveEvent() {}

		MoveEvent_t getEventType() const;
		void setEventType(MoveEvent_t type);

		virtual bool configureEvent(xmlNodePtr p);
		virtual bool loadFunction(const std::string& functionName);

		uint32_t fireStepEvent(Creature* actor, Creature* creature, Item* item, const Position& pos, const Position& fromPos, const Position& toPos);
		uint32_t fireAddRemItem(Creature* actor, Item* item, Item* tileItem, const Position& pos);
		bool fireEquip(Player* player, Item* item, slots_t slot, bool boolean);

		uint32_t executeStep(Creature* actor, Creature* creature, Item* item, const Position& pos, const Position& fromPos, const Position& toPos);
		bool executeEquip(Player* player, Item* item, slots_t slot, bool boolean);
		uint32_t executeAddRemItem(Creature* actor, Item* item, Item* tileItem, const Position& pos);

		static StepFunction StepInField;
		static MoveFunction AddItemField;
		static EquipFunction EquipItem;
		static EquipFunction DeEquipItem;

		uint32_t getWieldInfo() const {return wieldInfo;}
		uint32_t getSlot() const {return slot;}
		int32_t getReqLevel() const {return reqLevel;}
		int32_t getReqMagLv() const {return reqMagLevel;}
		bool isPremium() const {return premium;}

		const VocationMap& getVocEquipMap() const {return vocEquipMap;}
		const std::string& getVocationString() const {return vocationString;}

	protected:
		MoveEvent_t m_eventType;

		virtual std::string getScriptEventName() const;
		virtual std::string getScriptEventParams() const;

		MoveFunction* moveFunction;
		StepFunction* stepFunction;
		EquipFunction* equipFunction;

		uint32_t wieldInfo, slot;
		int32_t reqLevel, reqMagLevel;
		bool premium;

		VocationMap vocEquipMap;
		std::string vocationString;
};
#endif
