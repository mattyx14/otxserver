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

#ifndef __ACTIONS__
#define __ACTIONS__
#include "baseevents.h"

#include "luascript.h"
#include "thing.h"

class Action;
class Container;

enum ActionType_t
{
	ACTION_ANY,
	ACTION_UNIQUEID,
	ACTION_ACTIONID,
	ACTION_ITEMID,
	ACTION_RUNEID
};

class Actions : public BaseEvents
{
	public:
		Actions();
		virtual ~Actions();

		bool useItem(Player* player, const Position& pos, uint8_t index, Item* item);
		bool useItemEx(Player* player, const Position& fromPos, const Position& toPos,
			uint8_t toStackPos, Item* item, bool isHotkey, uint32_t creatureId = 0);

		ReturnValue canUse(const Player* player, const Position& pos);
		ReturnValue canUseEx(const Player* player, const Position& pos, const Item* item);
		ReturnValue canUseFar(const Creature* creature, const Position& toPos, bool checkLineOfSight);
		bool hasAction(const Item* item) const {return getAction(item, ACTION_ANY) != NULL;}

	protected:
		Action* defaultAction;

		virtual std::string getScriptBaseName() const {return "actions";}
		virtual void clear();

		virtual Event* getEvent(const std::string& nodeName);
		virtual bool registerEvent(Event* event, xmlNodePtr p, bool override);

		virtual LuaInterface& getInterface() {return m_interface;}
		LuaInterface m_interface;

		void registerItemID(int32_t itemId, Event* event);
		void registerActionID(int32_t actionId, Event* event);
		void registerUniqueID(int32_t uniqueId, Event* event);

		typedef std::map<uint16_t, Action*> ActionUseMap;
		ActionUseMap useItemMap;
		ActionUseMap uniqueItemMap;
		ActionUseMap actionItemMap;

		bool executeUse(Action* action, Player* player, Item* item, const PositionEx& posEx, uint32_t creatureId);
		ReturnValue internalUseItem(Player* player, const Position& pos,
			uint8_t index, Item* item, uint32_t creatureId);
		bool executeUseEx(Action* action, Player* player, Item* item, const PositionEx& fromPosEx,
			const PositionEx& toPosEx, bool isHotkey, uint32_t creatureId);
		ReturnValue internalUseItemEx(Player* player, const PositionEx& fromPosEx, const PositionEx& toPosEx,
			Item* item, bool isHotkey, uint32_t creatureId);

		Action* getAction(const Item* item, ActionType_t type) const;
		void clearMap(ActionUseMap& map);
};

class Action : public Event
{
	public:
		Action(const Action* copy);
		Action(LuaInterface* _interface);
		virtual ~Action() {}

		virtual bool configureEvent(xmlNodePtr p);

		//scripting
		virtual bool executeUse(Player* player, Item* item, const PositionEx& posFrom,
			const PositionEx& posTo, bool extendedUse, uint32_t creatureId);

		bool getAllowFarUse() const {return allowFarUse;}
		void setAllowFarUse(bool v) {allowFarUse = v;}

		bool getCheckLineOfSight() const {return checkLineOfSight;}
		void setCheckLineOfSight(bool v) {checkLineOfSight = v;}

		virtual ReturnValue canExecuteAction(const Player* player, const Position& pos);
		virtual bool hasOwnErrorHandler() {return false;}

	protected:
		virtual std::string getScriptEventName() const {return "onUse";}
		virtual std::string getScriptEventParams() const {return "cid, item, fromPosition, itemEx, toPosition";}

		bool allowFarUse;
		bool checkLineOfSight;
};
#endif
