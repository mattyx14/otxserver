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
#include "otpch.h"
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "movement.h"
#include "tools.h"

#include "creature.h"
#include "player.h"

#include "tile.h"
#include "vocation.h"

#include "combat.h"
#include "game.h"

extern Game g_game;
extern MoveEvents* g_moveEvents;

MoveEvent* MoveEventScript::event = NULL;

void MoveEventScript::registerFunctions()
{
	LuaInterface::registerFunctions();
	lua_register(m_luaState, "callFunction", MoveEventScript::luaCallFunction);
}

int32_t MoveEventScript::luaCallFunction(lua_State* L)
{
	//callFunction(...)
	MoveEvent* event = MoveEventScript::event;
	if(!event)
	{
		error(__FUNCTION__, "MoveEvent not set!");
		lua_pushboolean(L, false);
		return 1;
	}

	if(event->getEventType() == MOVE_EVENT_EQUIP || event->getEventType() == MOVE_EVENT_DE_EQUIP)
	{
		ScriptEnviroment* env = getEnv();
		bool boolean = popBoolean(L);
		slots_t slot = (slots_t)popNumber(L);

		Item* item = env->getItemByUID(popNumber(L));
		if(!item)
		{
			error(__FUNCTION__, getError(LUA_ERROR_ITEM_NOT_FOUND));
			lua_pushboolean(L, false);
			return 1;
		}

		Player* player = env->getPlayerByUID(popNumber(L));
		if(!player)
		{
			error(__FUNCTION__, getError(LUA_ERROR_PLAYER_NOT_FOUND));
			lua_pushboolean(L, false);
			return 1;
		}

		if(event->getEventType() != MOVE_EVENT_EQUIP)
			lua_pushboolean(L, MoveEvent::DeEquipItem(event, player, item, slot, boolean));
		else
			lua_pushboolean(L, MoveEvent::EquipItem(event, player, item, slot, boolean));

		return 1;
	}
	else if(event->getEventType() == MOVE_EVENT_STEP_IN)
	{
		ScriptEnviroment* env = getEnv();
		Item* item = env->getItemByUID(popNumber(L));
		if(!item)
		{
			error(__FUNCTION__, getError(LUA_ERROR_ITEM_NOT_FOUND));
			lua_pushboolean(L, false);
			return 1;
		}

		Creature* creature = env->getCreatureByUID(popNumber(L));
		if(!creature)
		{
			error(__FUNCTION__, getError(LUA_ERROR_CREATURE_NOT_FOUND));
			lua_pushboolean(L, false);
			return 1;
		}

		lua_pushboolean(L, MoveEvent::StepInField(creature, item));
		return 1;
	}
	else if(event->getEventType() == MOVE_EVENT_ADD_ITEM)
	{
		ScriptEnviroment* env = getEnv();
		Item* item = env->getItemByUID(popNumber(L));
		if(!item)
		{
			error(__FUNCTION__, getError(LUA_ERROR_ITEM_NOT_FOUND));
			lua_pushboolean(L, false);
			return 1;
		}

		lua_pushboolean(L, MoveEvent::AddItemField(item));
		return 1;
	}

	error(__FUNCTION__, "callFunction not available for current event.");
	lua_pushboolean(L, false);
	return 1;
}

MoveEvents::MoveEvents():
	m_lastCacheTile(NULL)
{
	m_interface.initState();
}

inline void MoveEvents::clearMap(MoveListMap& map)
{
	for(MoveListMap::iterator it = map.begin(); it != map.end(); ++it)
	{
		for(int32_t i = MOVE_EVENT_FIRST; i <= MOVE_EVENT_LAST; ++i)
		{
			EventList& moveEventList = it->second.moveEvent[i];
			for(EventList::iterator it = moveEventList.begin(); it != moveEventList.end(); ++it)
				delete (*it);

			moveEventList.clear();
		}
	}

	map.clear();
}

void MoveEvents::clear()
{
	clearMap(m_itemIdMap);
	clearMap(m_actionIdMap);
	clearMap(m_uniqueIdMap);

	for(MovePosListMap::iterator it = m_positionMap.begin(); it != m_positionMap.end(); ++it)
	{
		for(int32_t i = MOVE_EVENT_FIRST; i <= MOVE_EVENT_LAST; ++i)
		{
			EventList& moveEventList = it->second.moveEvent[i];
			for(EventList::iterator it = moveEventList.begin(); it != moveEventList.end(); ++it)
				delete (*it);

			moveEventList.clear();
		}
	}

	m_positionMap.clear();
	m_interface.reInitState();

	m_lastCacheTile = NULL;
	m_lastCacheItemVector.clear();
}

Event* MoveEvents::getEvent(const std::string& nodeName)
{
	std::string tmpNodeName = asLowerCaseString(nodeName);
	if(tmpNodeName == "movevent" || tmpNodeName == "moveevent" || tmpNodeName == "movement")
		return new MoveEvent(&m_interface);

	return NULL;
}

bool MoveEvents::registerEvent(Event* event, xmlNodePtr p, bool override)
{
	MoveEvent* moveEvent = dynamic_cast<MoveEvent*>(event);
	if(!moveEvent)
		return false;

	std::string strValue, endStrValue;
	MoveEvent_t eventType = moveEvent->getEventType();
	if((eventType == MOVE_EVENT_ADD_ITEM || eventType == MOVE_EVENT_REMOVE_ITEM) &&
		readXMLString(p, "tileitem", strValue) && booleanString(strValue))
	{
		switch(eventType)
		{
			case MOVE_EVENT_ADD_ITEM:
				moveEvent->setEventType(MOVE_EVENT_ADD_TILEITEM);
				break;
			case MOVE_EVENT_REMOVE_ITEM:
				moveEvent->setEventType(MOVE_EVENT_REMOVE_TILEITEM);
				break;
			default:
				break;
		}
	}

	StringVec strVector;
	IntegerVec intVector, endIntVector;

	bool success = true;
	if(readXMLString(p, "itemid", strValue))
	{
		strVector = explodeString(strValue, ";");
		for(StringVec::iterator it = strVector.begin(); it != strVector.end(); ++it)
		{
			intVector = vectorAtoi(explodeString((*it), "-"));
			if(!intVector[0])
				continue;

			bool equip = moveEvent->getEventType() == MOVE_EVENT_EQUIP;
			addEvent(moveEvent, intVector[0], m_itemIdMap, override);
			if(equip)
			{
				ItemType& it = Item::items.getItemType(intVector[0]);
				it.wieldInfo = moveEvent->getWieldInfo();
				it.minReqLevel = moveEvent->getReqLevel();
				it.minReqMagicLevel = moveEvent->getReqMagLv();
				it.vocationString = moveEvent->getVocationString();
			}

			if(intVector.size() > 1)
			{
				while(intVector[0] < intVector[1])
				{
					addEvent(new MoveEvent(moveEvent), ++intVector[0], m_itemIdMap, override);
					if(equip)
					{
						ItemType& tit = Item::items.getItemType(intVector[0]);
						tit.wieldInfo = moveEvent->getWieldInfo();
						tit.minReqLevel = moveEvent->getReqLevel();
						tit.minReqMagicLevel = moveEvent->getReqMagLv();
						tit.vocationString = moveEvent->getVocationString();
					}
				}
			}
		}
	}

	if(readXMLString(p, "fromid", strValue) && readXMLString(p, "toid", endStrValue))
	{
		intVector = vectorAtoi(explodeString(strValue, ";"));
		endIntVector = vectorAtoi(explodeString(endStrValue, ";"));
		if(intVector[0] && endIntVector[0] && intVector.size() == endIntVector.size())
		{
			for(size_t i = 0, size = intVector.size(); i < size; ++i)
			{
				bool equip = moveEvent->getEventType() == MOVE_EVENT_EQUIP;
				addEvent(moveEvent, intVector[i], m_itemIdMap, override);
				if(equip)
				{
					ItemType& it = Item::items.getItemType(intVector[i]);
					it.wieldInfo = moveEvent->getWieldInfo();
					it.minReqLevel = moveEvent->getReqLevel();
					it.minReqMagicLevel = moveEvent->getReqMagLv();
					it.vocationString = moveEvent->getVocationString();
				}

				while(intVector[i] < endIntVector[i])
				{
					addEvent(new MoveEvent(moveEvent), ++intVector[i], m_itemIdMap, override);
					if(equip)
					{
						ItemType& tit = Item::items.getItemType(intVector[i]);
						tit.wieldInfo = moveEvent->getWieldInfo();
						tit.minReqLevel = moveEvent->getReqLevel();
						tit.minReqMagicLevel = moveEvent->getReqMagLv();
						tit.vocationString = moveEvent->getVocationString();
					}
				}
			}
		}
		else
			std::clog << "[Warning - MoveEvents::registerEvent] Malformed entry (from item: \"" << strValue << "\", to item: \"" << endStrValue << "\")" << std::endl;
	}

	if(readXMLString(p, "uniqueid", strValue))
	{
		strVector = explodeString(strValue, ";");
		for(StringVec::iterator it = strVector.begin(); it != strVector.end(); ++it)
		{
			intVector = vectorAtoi(explodeString((*it), "-"));
			if(!intVector[0])
				continue;

			addEvent(moveEvent, intVector[0], m_uniqueIdMap, override);
			if(intVector.size() > 1)
			{
				while(intVector[0] < intVector[1])
					addEvent(new MoveEvent(moveEvent), ++intVector[0], m_uniqueIdMap, override);
			}
		}
	}

	if(readXMLString(p, "fromuid", strValue) && readXMLString(p, "touid", endStrValue))
	{
		intVector = vectorAtoi(explodeString(strValue, ";"));
		endIntVector = vectorAtoi(explodeString(endStrValue, ";"));
		if(intVector[0] && endIntVector[0] && intVector.size() == endIntVector.size())
		{
			for(size_t i = 0, size = intVector.size(); i < size; ++i)
			{
				addEvent(moveEvent, intVector[i], m_uniqueIdMap, override);
				while(intVector[i] < endIntVector[i])
					addEvent(new MoveEvent(moveEvent), ++intVector[i], m_uniqueIdMap, override);
			}
		}
		else
			std::clog << "[Warning - MoveEvents::registerEvent] Malformed entry (from unique: \"" << strValue << "\", to unique: \"" << endStrValue << "\")" << std::endl;
	}

	if(readXMLString(p, "actionid", strValue) || readXMLString(p, "aid", strValue))
	{
		strVector = explodeString(strValue, ";");
		for(StringVec::iterator it = strVector.begin(); it != strVector.end(); ++it)
		{
			intVector = vectorAtoi(explodeString((*it), "-"));
			if(!intVector[0])
				continue;

			addEvent(moveEvent, intVector[0], m_actionIdMap, override);
			if(intVector.size() > 1)
			{
				while(intVector[0] < intVector[1])
					addEvent(new MoveEvent(moveEvent), ++intVector[0], m_actionIdMap, override);
			}
		}
	}

	if(readXMLString(p, "fromaid", strValue) && readXMLString(p, "toaid", endStrValue))
	{
		intVector = vectorAtoi(explodeString(strValue, ";"));
		endIntVector = vectorAtoi(explodeString(endStrValue, ";"));
		if(intVector[0] && endIntVector[0] && intVector.size() == endIntVector.size())
		{
			for(size_t i = 0, size = intVector.size(); i < size; ++i)
			{
				addEvent(moveEvent, intVector[i], m_actionIdMap, override);
				while(intVector[i] < endIntVector[i])
					addEvent(new MoveEvent(moveEvent), ++intVector[i], m_actionIdMap, override);
			}
		}
		else
			std::clog << "[Warning - MoveEvents::registerEvent] Malformed entry (from action: \"" << strValue << "\", to action: \"" << endStrValue << "\")" << std::endl;
	}

	if(readXMLString(p, "pos", strValue) || readXMLString(p, "position", strValue))
	{
		strVector = explodeString(strValue, ";");
		for(StringVec::iterator it = strVector.begin(); it != strVector.end(); ++it)
		{
			intVector = vectorAtoi(explodeString((*it), ","));
			if(intVector.size() > 2)
				addEvent(moveEvent, Position(intVector[0], intVector[1], intVector[2]), m_positionMap, override);
			else
				success = false;
		}
	}

	return success;
}

void MoveEvents::addEvent(MoveEvent* moveEvent, int32_t id, MoveListMap& map, bool override)
{
	MoveListMap::iterator it = map.find(id);
	if(it != map.end())
	{
		EventList& moveEventList = it->second.moveEvent[moveEvent->getEventType()];
		for(EventList::iterator it = moveEventList.begin(); it != moveEventList.end(); ++it)
		{
			if((*it)->getSlot() != moveEvent->getSlot())
				continue;

			if(override)
			{
				delete *it;
				*it = moveEvent;
			}
			else
				std::clog << "[Warning - MoveEvents::addEvent] Duplicate move event found: " << id << std::endl;

			return;
		}

		moveEventList.push_back(moveEvent);
	}
	else
	{
		MoveEventList moveEventList;
		moveEventList.moveEvent[moveEvent->getEventType()].push_back(moveEvent);
		map[id] = moveEventList;
	}
}

MoveEvent* MoveEvents::getEvent(Item* item, MoveEvent_t eventType)
{
	MoveListMap::iterator it;
	if(item->getUniqueId())
	{
		it = m_uniqueIdMap.find(item->getUniqueId());
		if(it != m_uniqueIdMap.end())
		{
			EventList& moveEventList = it->second.moveEvent[eventType];
			if(!moveEventList.empty())
				return *moveEventList.begin();
		}
	}

	if(item->getActionId())
	{
		it = m_actionIdMap.find(item->getActionId());
		if(it != m_actionIdMap.end())
		{
			EventList& moveEventList = it->second.moveEvent[eventType];
			if(!moveEventList.empty())
				return *moveEventList.begin();
		}
	}

	it = m_itemIdMap.find(item->getID());
	if(it != m_itemIdMap.end())
	{
		EventList& moveEventList = it->second.moveEvent[eventType];
		if(!moveEventList.empty())
			return *moveEventList.begin();
	}

	return NULL;
}

MoveEvent* MoveEvents::getEvent(Item* item, MoveEvent_t eventType, slots_t slot)
{
	uint32_t slotp = 0;
	switch(slot)
	{
		case SLOT_HEAD:
			slotp = SLOTP_HEAD;
			break;
		case SLOT_NECKLACE:
			slotp = SLOTP_NECKLACE;
			break;
		case SLOT_BACKPACK:
			slotp = SLOTP_BACKPACK;
			break;
		case SLOT_ARMOR:
			slotp = SLOTP_ARMOR;
			break;
		case SLOT_RIGHT:
			slotp = SLOTP_RIGHT;
			break;
		case SLOT_LEFT:
			slotp = SLOTP_LEFT;
			break;
		case SLOT_LEGS:
			slotp = SLOTP_LEGS;
			break;
		case SLOT_FEET:
			slotp = SLOTP_FEET;
			break;
		case SLOT_AMMO:
			slotp = SLOTP_AMMO;
			break;
		case SLOT_RING:
			slotp = SLOTP_RING;
			break;
		default:
			break;
	}

	MoveListMap::iterator it = m_itemIdMap.find(item->getID());
	if(it == m_itemIdMap.end())
		return NULL;

	EventList& moveEventList = it->second.moveEvent[eventType];
	for(EventList::iterator it = moveEventList.begin(); it != moveEventList.end(); ++it)
	{
		if(((*it)->getSlot() & slotp))
			return *it;
	}

	return NULL;
}

void MoveEvents::addEvent(MoveEvent* moveEvent, Position pos, MovePosListMap& map, bool override)
{
	MovePosListMap::iterator it = map.find(pos);
	if(it != map.end())
	{
		bool add = true;
		if(!it->second.moveEvent[moveEvent->getEventType()].empty())
		{
			if(!override)
			{
				std::clog << "[Warning - MoveEvents::addEvent] Duplicate move event found: " << pos << std::endl;
				add = false;
			}
			else
				it->second.moveEvent[moveEvent->getEventType()].clear();
		}

		if(add)
			it->second.moveEvent[moveEvent->getEventType()].push_back(moveEvent);
	}
	else
	{
		MoveEventList moveEventList;
		moveEventList.moveEvent[moveEvent->getEventType()].push_back(moveEvent);
		map[pos] = moveEventList;
	}
}

MoveEvent* MoveEvents::getEvent(const Tile* tile, MoveEvent_t eventType)
{
	MovePosListMap::iterator it = m_positionMap.find(tile->getPosition());
	if(it == m_positionMap.end())
		return NULL;

	EventList& moveEventList = it->second.moveEvent[eventType];
	if(!moveEventList.empty())
		return *moveEventList.begin();

	return NULL;
}

bool MoveEvents::hasEquipEvent(Item* item)
{
	MoveEvent* event = NULL;
	return (event = getEvent(item, MOVE_EVENT_EQUIP)) && !event->isScripted()
		&& (event = getEvent(item, MOVE_EVENT_DE_EQUIP)) && !event->isScripted();
}

bool MoveEvents::hasTileEvent(Item* item)
{
	return getEvent(item, MOVE_EVENT_STEP_IN) || getEvent(item, MOVE_EVENT_STEP_OUT) || getEvent(
		item, MOVE_EVENT_ADD_TILEITEM) || getEvent(item, MOVE_EVENT_REMOVE_TILEITEM);
}

uint32_t MoveEvents::onCreatureMove(Creature* actor, Creature* creature, const Tile* fromTile, const Tile* toTile, bool isStepping)
{
	MoveEvent_t eventType = MOVE_EVENT_STEP_OUT;
	const Tile* tile = fromTile;
	if(isStepping)
	{
		eventType = MOVE_EVENT_STEP_IN;
		tile = toTile;
	}

	Position fromPos;
	if(fromTile)
		fromPos = fromTile->getPosition();

	Position toPos;
	if(toTile)
		toPos = toTile->getPosition();

	uint32_t ret = 1;
	MoveEvent* moveEvent = NULL;
	if((moveEvent = getEvent(tile, eventType)))
		ret &= moveEvent->fireStepEvent(actor, creature, NULL, Position(), fromPos, toPos);

	if(!tile)
		return ret;

	Item* tileItem = NULL;
	if(m_lastCacheTile == tile)
	{
		if(m_lastCacheItemVector.empty())
			return ret;

		//We cannot use iterators here since the scripts can invalidate the iterator
		for(uint32_t i = 0; i < m_lastCacheItemVector.size(); ++i)
		{
			if((tileItem = m_lastCacheItemVector[i]) && (moveEvent = getEvent(tileItem, eventType)))
				ret &= moveEvent->fireStepEvent(actor, creature, tileItem, tile->getPosition(), fromPos, toPos);
		}

		return ret;
	}

	m_lastCacheTile = tile;
	m_lastCacheItemVector.clear();

	//We cannot use iterators here since the scripts can invalidate the iterator
	Thing* thing = NULL;
	for(int32_t i = tile->__getFirstIndex(), j = tile->__getLastIndex(); i < j; ++i) //already checked the ground
	{
		if(!(thing = tile->__getThing(i)) || !(tileItem = thing->getItem()))
			continue;

		if((moveEvent = getEvent(tileItem, eventType)))
		{
			m_lastCacheItemVector.push_back(tileItem);
			ret &= moveEvent->fireStepEvent(actor, creature, tileItem, tile->getPosition(), fromPos, toPos);
		}
		else if(hasTileEvent(tileItem))
			m_lastCacheItemVector.push_back(tileItem);
	}

	return ret;
}

bool MoveEvents::onPlayerEquip(Player* player, Item* item, slots_t slot, bool isCheck)
{
	if(MoveEvent* moveEvent = getEvent(item, MOVE_EVENT_EQUIP, slot))
		return moveEvent->fireEquip(player, item, slot, isCheck);

	return true;
}

bool MoveEvents::onPlayerDeEquip(Player* player, Item* item, slots_t slot, bool isRemoval)
{
	if(MoveEvent* moveEvent = getEvent(item, MOVE_EVENT_DE_EQUIP, slot))
		return moveEvent->fireEquip(player, item, slot, isRemoval);

	return true;
}

uint32_t MoveEvents::onItemMove(Creature* actor, Item* item, Tile* tile, bool isAdd)
{
	MoveEvent_t eventType = MOVE_EVENT_REMOVE_ITEM, tileEventType = MOVE_EVENT_REMOVE_TILEITEM;
	if(isAdd)
	{
		eventType = MOVE_EVENT_ADD_ITEM;
		tileEventType = MOVE_EVENT_ADD_TILEITEM;
	}

	uint32_t ret = 1;
	MoveEvent* moveEvent = getEvent(tile, eventType);
	if(moveEvent)
		ret &= moveEvent->fireAddRemItem(actor, item, NULL, tile->getPosition());

	moveEvent = getEvent(item, eventType);
	if(moveEvent)
		ret &= moveEvent->fireAddRemItem(actor, item, NULL, tile->getPosition());

	if(!tile)
		return ret;

	Item* tileItem = NULL;
	if(m_lastCacheTile == tile)
	{
		if(m_lastCacheItemVector.empty())
			return ret;

		//We cannot use iterators here since the scripts can invalidate the iterator
		for(uint32_t i = 0; i < m_lastCacheItemVector.size(); ++i)
		{
			if((tileItem = m_lastCacheItemVector[i]) && tileItem != item
				&& (moveEvent = getEvent(tileItem, tileEventType)))
				ret &= moveEvent->fireAddRemItem(actor, item, tileItem, tile->getPosition());
		}

		return ret;
	}

	m_lastCacheTile = tile;
	m_lastCacheItemVector.clear();

	//we cannot use iterators here since the scripts can invalidate the iterator
	Thing* thing = NULL;
	for(int32_t i = tile->__getFirstIndex(), j = tile->__getLastIndex(); i < j; ++i) //already checked the ground
	{
		if(!(thing = tile->__getThing(i)) || !(tileItem = thing->getItem()) || tileItem == item)
			continue;

		if((moveEvent = getEvent(tileItem, tileEventType)))
		{
			m_lastCacheItemVector.push_back(tileItem);
			ret &= moveEvent->fireAddRemItem(actor, item, tileItem, tile->getPosition());
		}
		else if(hasTileEvent(tileItem))
			m_lastCacheItemVector.push_back(tileItem);
	}

	return ret;
}

void MoveEvents::onAddTileItem(const Tile* tile, Item* item)
{
	if(m_lastCacheTile != tile)
		return;

	std::vector<Item*>::iterator it = std::find(m_lastCacheItemVector.begin(), m_lastCacheItemVector.end(), item);
	if(it == m_lastCacheItemVector.end() && hasTileEvent(item))
		m_lastCacheItemVector.push_back(item);
}

void MoveEvents::onRemoveTileItem(const Tile* tile, Item* item)
{
	if(m_lastCacheTile != tile)
		return;

	for(uint32_t i = 0; i < m_lastCacheItemVector.size(); ++i)
	{
		if(m_lastCacheItemVector[i] != item)
			continue;

		m_lastCacheItemVector[i] = NULL;
		break;
	}
}

MoveEvent::MoveEvent(LuaInterface* _interface):
Event(_interface)
{
	stepFunction = NULL;
	moveFunction = NULL;
	equipFunction = NULL;
	slot = SLOTP_WHEREEVER;

	m_eventType = MOVE_EVENT_NONE;
	wieldInfo = 0;
	reqLevel = 0;
	reqMagLevel = 0;
	premium = false;
}

MoveEvent::MoveEvent(const MoveEvent* copy):
Event(copy)
{
	stepFunction = copy->stepFunction;
	moveFunction = copy->moveFunction;
	equipFunction = copy->equipFunction;
	slot = copy->slot;

	m_eventType = copy->m_eventType;
	if(copy->m_eventType == MOVE_EVENT_EQUIP)
	{
		wieldInfo = copy->wieldInfo;
		reqLevel = copy->reqLevel;
		reqMagLevel = copy->reqMagLevel;
		vocationString = copy->vocationString;
		premium = copy->premium;
		vocEquipMap = copy->vocEquipMap;
	}
}

std::string MoveEvent::getScriptEventName() const
{
	switch(m_eventType)
	{
		case MOVE_EVENT_STEP_IN:
			return "onStepIn";

		case MOVE_EVENT_STEP_OUT:
			return "onStepOut";

		case MOVE_EVENT_EQUIP:
			return "onEquip";

		case MOVE_EVENT_DE_EQUIP:
			return "onDeEquip";

		case MOVE_EVENT_ADD_ITEM:
			return "onAddItem";

		case MOVE_EVENT_REMOVE_ITEM:
			return "onRemoveItem";

		default:
			break;
	}

	std::clog << "[Error - MoveEvent::getScriptEventName] No valid event type." << std::endl;
	return "";
}

std::string MoveEvent::getScriptEventParams() const
{
	switch(m_eventType)
	{
		case MOVE_EVENT_STEP_IN:
		case MOVE_EVENT_STEP_OUT:
			return "cid, item, position, lastPosition, fromPosition, toPosition, actor";

		case MOVE_EVENT_EQUIP:
		case MOVE_EVENT_DE_EQUIP:
			return "cid, item, slot, boolean";

		case MOVE_EVENT_ADD_ITEM:
		case MOVE_EVENT_REMOVE_ITEM:
			return "moveItem, tileItem, position, cid";

		default:
			break;
	}

	std::clog << "[Error - MoveEvent::getScriptEventParams] No valid event type." << std::endl;
	return "";
}

bool MoveEvent::configureEvent(xmlNodePtr p)
{
	std::string strValue;
	int32_t intValue;
	if(readXMLString(p, "type", strValue) || readXMLString(p, "event", strValue))
	{
		std::string tmpStrValue = asLowerCaseString(strValue);
		if(tmpStrValue == "stepin")
			m_eventType = MOVE_EVENT_STEP_IN;
		else if(tmpStrValue == "stepout")
			m_eventType = MOVE_EVENT_STEP_OUT;
		else if(tmpStrValue == "equip")
			m_eventType = MOVE_EVENT_EQUIP;
		else if(tmpStrValue == "deequip")
			m_eventType = MOVE_EVENT_DE_EQUIP;
		else if(tmpStrValue == "additem")
			m_eventType = MOVE_EVENT_ADD_ITEM;
		else if(tmpStrValue == "removeitem")
			m_eventType = MOVE_EVENT_REMOVE_ITEM;
		else
		{
			if(tmpStrValue == "function" || tmpStrValue == "buffer" || tmpStrValue == "script")
				std::clog << "[Error - MoveEvent::configureMoveEvent] No event type found." << std::endl;
			else
				std::clog << "[Error - MoveEvent::configureMoveEvent] Unknown event type \"" << strValue << "\"" << std::endl;

			return false;
		}

		if(m_eventType == MOVE_EVENT_EQUIP || m_eventType == MOVE_EVENT_DE_EQUIP)
		{
			if(readXMLString(p, "slot", strValue))
			{
				std::string tmpStrValue = asLowerCaseString(strValue);
				if(tmpStrValue == "head")
					slot = SLOTP_HEAD;
				else if(tmpStrValue == "necklace")
					slot = SLOTP_NECKLACE;
				else if(tmpStrValue == "backpack")
					slot = SLOTP_BACKPACK;
				else if(tmpStrValue == "armor")
					slot = SLOTP_ARMOR;
				else if(tmpStrValue == "left-hand")
					slot = SLOTP_LEFT;
				else if(tmpStrValue == "right-hand")
					slot = SLOTP_RIGHT;
				else if(tmpStrValue == "hands" || tmpStrValue == "two-handed")
					slot = SLOTP_TWO_HAND;
				else if(tmpStrValue == "hand" || tmpStrValue == "shield")
					slot = SLOTP_RIGHT | SLOTP_LEFT;
				else if(tmpStrValue == "legs")
					slot = SLOTP_LEGS;
				else if(tmpStrValue == "feet")
					slot = SLOTP_FEET;
				else if(tmpStrValue == "ring")
					slot = SLOTP_RING;
				else if(tmpStrValue == "ammo" || tmpStrValue == "ammunition")
					slot = SLOTP_AMMO;
				else if(tmpStrValue == "pickupable")
					slot = SLOTP_RIGHT | SLOTP_LEFT | SLOTP_AMMO;
				else if(tmpStrValue == "wherever" || tmpStrValue == "any")
					slot = SLOTP_WHEREEVER;
				else
					std::clog << "[Warning - MoveEvent::configureMoveEvent] Unknown slot type \"" << strValue << "\"" << std::endl;
			}

			wieldInfo = 0;
			if(readXMLInteger(p, "lvl", intValue) || readXMLInteger(p, "level", intValue))
			{
	 			reqLevel = intValue;
				if(reqLevel > 0)
					wieldInfo |= WIELDINFO_LEVEL;
			}

			if(readXMLInteger(p, "maglv", intValue) || readXMLInteger(p, "maglevel", intValue))
			{
	 			reqMagLevel = intValue;
				if(reqMagLevel > 0)
					wieldInfo |= WIELDINFO_MAGLV;
			}

			if(readXMLString(p, "prem", strValue) || readXMLString(p, "premium", strValue))
			{
				premium = booleanString(strValue);
				if(premium)
					wieldInfo |= WIELDINFO_PREMIUM;
			}

			StringVec vocStringVec;
			std::string error = "";
			for(xmlNodePtr vocationNode = p->children; vocationNode; vocationNode = vocationNode->next)
			{
				if(!parseVocationNode(vocationNode, vocEquipMap, vocStringVec, error))
					std::clog << "[Warning - MoveEvent::configureEvent] " << error << std::endl;
			}

			if(!vocEquipMap.empty())
				wieldInfo |= WIELDINFO_VOCREQ;

			vocationString = parseVocationString(vocStringVec);
		}
	}
	else
	{
		std::clog << "[Error - MoveEvent::configureMoveEvent] No event type found." << std::endl;
		return false;
	}

	return true;
}

bool MoveEvent::loadFunction(const std::string& functionName)
{
	std::string tmpFunctionName = asLowerCaseString(functionName);
	if(tmpFunctionName == "onstepinfield")
		stepFunction = StepInField;
	else if(tmpFunctionName == "onaddfield")
		moveFunction = AddItemField;
	else if(tmpFunctionName == "onequipitem")
		equipFunction = EquipItem;
	else if(tmpFunctionName == "ondeequipitem")
		equipFunction = DeEquipItem;
	else
	{
		std::clog << "[Warning - MoveEvent::loadFunction] Function \"" << functionName << "\" does not exist." << std::endl;
		return false;
	}

	m_scripted = EVENT_SCRIPT_FALSE;
	return true;
}

MoveEvent_t MoveEvent::getEventType() const
{
	if(m_eventType == MOVE_EVENT_NONE)
		std::clog << "[Error - MoveEvent::getEventType] MOVE_EVENT_NONE" << std::endl;

	return m_eventType;
}

void MoveEvent::setEventType(MoveEvent_t type)
{
	m_eventType = type;
}

uint32_t MoveEvent::StepInField(Creature* creature, Item* item)
{
	if(MagicField* field = item->getMagicField())
	{
		field->onStepInField(creature);
		return 1;
	}

	return LUA_ERROR_ITEM_NOT_FOUND;
}

uint32_t MoveEvent::AddItemField(Item* item)
{
	if(MagicField* field = item->getMagicField())
	{
		if(Tile* tile = item->getTile())
		{
			if(CreatureVector* creatures = tile->getCreatures())
			{
				for(CreatureVector::iterator cit = creatures->begin(); cit != creatures->end(); ++cit)
					field->onStepInField(*cit);
			}
		}

		return 1;
	}

	return LUA_ERROR_ITEM_NOT_FOUND;
}

bool MoveEvent::EquipItem(MoveEvent* moveEvent, Player* player, Item* item, slots_t slot, bool isCheck)
{
	if(player->isItemAbilityEnabled(slot))
		return true;

	if(!player->hasFlag(PlayerFlag_IgnoreEquipCheck) && moveEvent->getWieldInfo() != 0)
	{
		if(player->getLevel() < (uint32_t)moveEvent->getReqLevel() || player->getMagicLevel() < (uint32_t)moveEvent->getReqMagLv())
			return false;

		if(moveEvent->isPremium() && !player->isPremium())
			return false;

		if(!moveEvent->getVocEquipMap().empty() && moveEvent->getVocEquipMap().find(player->getVocationId()) == moveEvent->getVocEquipMap().end())
			return false;
	}

	if(isCheck)
		return true;

	const ItemType& it = Item::items[item->getID()];
	if(it.transformEquipTo)
	{
		Item* newItem = g_game.transformItem(item, it.transformEquipTo);
		g_game.startDecay(newItem);
	}

	player->setItemAbility(slot, true);
	if(!it.hasAbilities())
		return true;

	if(it.abilities->invisible)
	{
		Condition* condition = Condition::createCondition((ConditionId_t)slot, CONDITION_INVISIBLE, -1, 0);
		player->addCondition(condition);
	}

	if(it.abilities->manaShield)
	{
		Condition* condition = Condition::createCondition((ConditionId_t)slot, CONDITION_MANASHIELD, -1, 0);
		player->addCondition(condition);
	}

	if(it.abilities->speed)
		g_game.changeSpeed(player, it.abilities->speed);

	if(it.abilities->conditionSuppressions)
	{
		player->setConditionSuppressions(it.abilities->conditionSuppressions, false);
		player->sendIcons();
	}

	if(it.abilities->regeneration)
	{
		Condition* condition = Condition::createCondition((ConditionId_t)slot, CONDITION_REGENERATION, -1, 0);
		if(it.abilities->healthGain)
			condition->setParam(CONDITIONPARAM_HEALTHGAIN, it.abilities->healthGain);

		if(it.abilities->healthTicks)
			condition->setParam(CONDITIONPARAM_HEALTHTICKS, it.abilities->healthTicks);

		if(it.abilities->manaGain)
			condition->setParam(CONDITIONPARAM_MANAGAIN, it.abilities->manaGain);

		if(it.abilities->manaTicks)
			condition->setParam(CONDITIONPARAM_MANATICKS, it.abilities->manaTicks);

		player->addCondition(condition);
	}

	bool needUpdateSkills = false;
	for(uint32_t i = SKILL_FIRST; i <= SKILL_LAST; ++i)
	{
		if(it.abilities->skills[i])
		{
			player->setVarSkill((skills_t)i, it.abilities->skills[i]);
			if(!needUpdateSkills)
				needUpdateSkills = true;
		}

		if(it.abilities->skillsPercent[i])
		{
			player->setVarSkill((skills_t)i, (int32_t)(player->getSkill((skills_t)i, SKILL_LEVEL) * ((it.abilities->skillsPercent[i] - 100) / 100.f)));
			if(!needUpdateSkills)
				needUpdateSkills = true;
		}
	}

	if(needUpdateSkills)
		player->sendSkills();

	bool needUpdateStats = false;
	for(uint32_t s = STAT_FIRST; s <= STAT_LAST; ++s)
	{
		if(it.abilities->stats[s])
		{
			player->setVarStats((stats_t)s, it.abilities->stats[s]);
			if(!needUpdateStats)
				needUpdateStats = true;
		}

		if(it.abilities->statsPercent[s])
		{
			player->setVarStats((stats_t)s, (int32_t)(player->getDefaultStats((stats_t)s) * ((it.abilities->statsPercent[s] - 100) / 100.f)));
			if(!needUpdateStats)
				needUpdateStats = true;
		}
	}

	if(needUpdateStats)
		player->sendStats();

	return true;
}

bool MoveEvent::DeEquipItem(MoveEvent*, Player* player, Item* item, slots_t slot, bool isRemoval)
{
	if(!player->isItemAbilityEnabled(slot))
		return true;

	const ItemType& it = Item::items[item->getID()];
	if(isRemoval && it.transformDeEquipTo)
	{
		g_game.transformItem(item, it.transformDeEquipTo);
		g_game.startDecay(item);
	}

	player->setItemAbility(slot, false);
	if(!it.hasAbilities())
		return true;

	if(it.abilities->invisible)
		player->removeCondition(CONDITION_INVISIBLE, (ConditionId_t)slot);

	if(it.abilities->manaShield)
		player->removeCondition(CONDITION_MANASHIELD, (ConditionId_t)slot);

	if(it.abilities->speed)
		g_game.changeSpeed(player, -it.abilities->speed);

	if(it.abilities->conditionSuppressions)
	{
		player->setConditionSuppressions(it.abilities->conditionSuppressions, true);
		player->sendIcons();
	}

	if(it.abilities->regeneration)
		player->removeCondition(CONDITION_REGENERATION, (ConditionId_t)slot);

	bool needUpdateSkills = false;
	for(uint32_t i = SKILL_FIRST; i <= SKILL_LAST; ++i)
	{
		if(it.abilities->skills[i])
		{
			needUpdateSkills = true;
			player->setVarSkill((skills_t)i, -it.abilities->skills[i]);
		}

		if(it.abilities->skillsPercent[i])
		{
			needUpdateSkills = true;
			player->setVarSkill((skills_t)i, -(int32_t)(player->getSkill((skills_t)i, SKILL_LEVEL) * ((it.abilities->skillsPercent[i] - 100) / 100.f)));
		}
	}

	if(needUpdateSkills)
		player->sendSkills();

	bool needUpdateStats = false;
	for(uint32_t s = STAT_FIRST; s <= STAT_LAST; ++s)
	{
		if(it.abilities->stats[s])
		{
			needUpdateStats = true;
			player->setVarStats((stats_t)s, -it.abilities->stats[s]);
		}

		if(it.abilities->statsPercent[s])
		{
			needUpdateStats = true;
			player->setVarStats((stats_t)s, -(int32_t)(player->getDefaultStats((stats_t)s) * ((it.abilities->statsPercent[s] - 100) / 100.f)));
		}
	}

	if(needUpdateStats)
		player->sendStats();

	return true;
}

uint32_t MoveEvent::fireStepEvent(Creature* actor, Creature* creature, Item* item, const Position& pos, const Position& fromPos, const Position& toPos)
{
	if(isScripted())
		return executeStep(actor, creature, item, pos, fromPos, toPos);

	return stepFunction(creature, item);
}

uint32_t MoveEvent::executeStep(Creature* actor, Creature* creature, Item* item, const Position& pos, const Position& fromPos, const Position& toPos)
{
	//onStepIn(cid, item, position, lastPosition, fromPosition, toPosition, actor)
	//onStepOut(cid, item, position, lastPosition, fromPosition, toPosition, actor)
	if(m_interface->reserveEnv())
	{
		MoveEventScript::event = this;
		ScriptEnviroment* env = m_interface->getEnv();
		if(m_scripted == EVENT_SCRIPT_BUFFER)
		{
			env->setRealPos(creature->getPosition());
			std::stringstream scriptstream;
			scriptstream << "local cid = " << env->addThing(creature) << std::endl;

			env->streamThing(scriptstream, "item", item, env->addThing(item));
			env->streamPosition(scriptstream, "position", pos, 0);
			env->streamPosition(scriptstream, "lastPosition", creature->getLastPosition(), 0);
			env->streamPosition(scriptstream, "fromPosition", fromPos, 0);
			env->streamPosition(scriptstream, "toPosition", toPos, 0);
			scriptstream << "local actor = " << env->addThing(actor) << std::endl;

			if(m_scriptData)
				scriptstream << *m_scriptData;

			bool result = true;
			if(m_interface->loadBuffer(scriptstream.str()))
			{
				lua_State* L = m_interface->getState();
				result = m_interface->getGlobalBool(L, "_result", true);
			}

			m_interface->releaseEnv();
			return result;
		}
		else
		{
			#ifdef __DEBUG_LUASCRIPTS__
			std::stringstream desc;
			desc << creature->getName() << " itemid: " << item->getID() << " - " << pos;
			env->setEvent(desc.str());
			#endif

			env->setScriptId(m_scriptId, m_interface);
			env->setRealPos(creature->getPosition());

			lua_State* L = m_interface->getState();
			m_interface->pushFunction(m_scriptId);
			lua_pushnumber(L, env->addThing(creature));

			LuaInterface::pushThing(L, item, env->addThing(item));
			LuaInterface::pushPosition(L, pos, 0);
			LuaInterface::pushPosition(L, creature->getLastPosition(), 0);
			LuaInterface::pushPosition(L, fromPos, 0);
			LuaInterface::pushPosition(L, toPos, 0);

			lua_pushnumber(L, env->addThing(actor));
			bool result = m_interface->callFunction(7);

			m_interface->releaseEnv();
			return result;
		}
	}
	else
	{
		std::clog << "[Error - MoveEvent::executeStep] Call stack overflow." << std::endl;
		return 0;
	}
}

bool MoveEvent::fireEquip(Player* player, Item* item, slots_t slot, bool boolean)
{
	if(isScripted())
		return executeEquip(player, item, slot, boolean);

	return equipFunction(this, player, item, slot, boolean);
}

bool MoveEvent::executeEquip(Player* player, Item* item, slots_t slot, bool boolean)
{
	//onEquip(cid, item, slot, boolean)
	//onDeEquip(cid, item, slot, boolean)
	if(m_interface->reserveEnv())
	{
		MoveEventScript::event = this;
		ScriptEnviroment* env = m_interface->getEnv();
		if(m_scripted == EVENT_SCRIPT_BUFFER)
		{
			env->setRealPos(player->getPosition());
			std::stringstream scriptstream;

			scriptstream << "local cid = " << env->addThing(player) << std::endl;
			env->streamThing(scriptstream, "item", item, env->addThing(item));
			scriptstream << "local slot = " << slot << std::endl;
			scriptstream << "local boolean = " << (boolean ? "true" : "false") << std::endl;

			if(m_scriptData)
				scriptstream << *m_scriptData;

			bool result = true;
			if(m_interface->loadBuffer(scriptstream.str()))
			{
				lua_State* L = m_interface->getState();
				result = m_interface->getGlobalBool(L, "_result", true);
			}

			m_interface->releaseEnv();
			return result;
		}
		else
		{
			#ifdef __DEBUG_LUASCRIPTS__
			std::stringstream desc;
			desc << player->getName() << " itemid: " << item->getID() << " slot: " << slot;
			env->setEvent(desc.str());
			#endif

			env->setScriptId(m_scriptId, m_interface);
			env->setRealPos(player->getPosition());

			lua_State* L = m_interface->getState();
			m_interface->pushFunction(m_scriptId);

			lua_pushnumber(L, env->addThing(player));
			LuaInterface::pushThing(L, item, env->addThing(item));
			lua_pushnumber(L, slot);
			lua_pushboolean(L, boolean);

			bool result = m_interface->callFunction(4);
			m_interface->releaseEnv();
			return result;
		}
	}
	else
	{
		std::clog << "[Error - MoveEvent::executeEquip] Call stack overflow." << std::endl;
		return false;
	}
}

uint32_t MoveEvent::fireAddRemItem(Creature* actor, Item* item, Item* tileItem, const Position& pos)
{
	if(isScripted())
		return executeAddRemItem(actor, item, tileItem, pos);

	return moveFunction(item);
}

uint32_t MoveEvent::executeAddRemItem(Creature* actor, Item* item, Item* tileItem, const Position& pos)
{
	//onAddItem(moveItem, tileItem, position, cid)
	//onRemoveItem(moveItem, tileItem, position, cid)
	if(m_interface->reserveEnv())
	{
		MoveEventScript::event = this;
		ScriptEnviroment* env = m_interface->getEnv();
		if(m_scripted == EVENT_SCRIPT_BUFFER)
		{
			env->setRealPos(pos);
			std::stringstream scriptstream;

			env->streamThing(scriptstream, "moveItem", item, env->addThing(item));
			env->streamThing(scriptstream, "tileItem", tileItem, env->addThing(tileItem));

			env->streamPosition(scriptstream, "position", pos, 0);
			scriptstream << "local cid = " << env->addThing(actor) << std::endl;

			if(m_scriptData)
				scriptstream << *m_scriptData;

			bool result = true;
			if(m_interface->loadBuffer(scriptstream.str()))
			{
				lua_State* L = m_interface->getState();
				result = m_interface->getGlobalBool(L, "_result", true);
			}

			m_interface->releaseEnv();
			return result;
		}
		else
		{
			#ifdef __DEBUG_LUASCRIPTS__
			std::stringstream desc;
			if(tileItem)
				desc << "tileid: " << tileItem->getID();

			desc << " itemid: " << item->getID() << " - " << pos;
			env->setEvent(desc.str());
			#endif

			env->setScriptId(m_scriptId, m_interface);
			env->setRealPos(pos);

			lua_State* L = m_interface->getState();
			m_interface->pushFunction(m_scriptId);

			LuaInterface::pushThing(L, item, env->addThing(item));
			LuaInterface::pushThing(L, tileItem, env->addThing(tileItem));
			LuaInterface::pushPosition(L, pos, 0);

			lua_pushnumber(L, env->addThing(actor));
			bool result = m_interface->callFunction(4);

			m_interface->releaseEnv();
			return result;
		}
	}
	else
	{
		std::clog << "[Error - MoveEvent::executeAddRemItem] Call stack overflow." << std::endl;
		return 0;
	}
}
