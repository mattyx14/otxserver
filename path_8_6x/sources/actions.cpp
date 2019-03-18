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

#include "const.h"

#include "actions.h"
#include "tools.h"

#include "player.h"
#include "monster.h"
#include "npc.h"

#include "item.h"
#include "container.h"

#include "game.h"
#include "configmanager.h"

#include "combat.h"
#include "spells.h"

#include "house.h"
#include "beds.h"

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

extern Game g_game;
extern Spells* g_spells;
extern Actions* g_actions;
extern ConfigManager g_config;

Actions::Actions():
m_interface("Action Interface")
{
	m_interface.initState();
	defaultAction = NULL;
}

Actions::~Actions()
{
	clear();
}

inline void Actions::clearMap(ActionUseMap& map)
{
	for(ActionUseMap::iterator it = map.begin(); it != map.end(); ++it)
		delete it->second;

	map.clear();
}

void Actions::clear()
{
	clearMap(useItemMap);
	clearMap(uniqueItemMap);
	clearMap(actionItemMap);

	m_interface.reInitState();
	delete defaultAction;
	defaultAction = NULL;
}

Event* Actions::getEvent(const std::string& nodeName)
{
	if(asLowerCaseString(nodeName) == "action")
		return new Action(&m_interface);

	return NULL;
}

bool Actions::registerEvent(Event* event, xmlNodePtr p, bool override)
{
	Action* action = dynamic_cast<Action*>(event);
	if(!action)
		return false;

	std::string strValue;
	if(readXMLString(p, "default", strValue) && booleanString(strValue))
	{
		if(!defaultAction)
			defaultAction = action;
		else if(override)
		{
			delete defaultAction;
			defaultAction = action;
		}
		else
			std::clog << "[Warning - Actions::registerEvent] You cannot define more than one default action, if you want to do so "
				<< "Please define \"override\"." << std::endl;

		return true;
	}

	bool success = true;
	std::string endValue;
	if(readXMLString(p, "itemid", strValue))
	{
		IntegerVec intVector;
		if(!parseIntegerVec(strValue, intVector))
		{
			std::clog << "[Warning - Actions::registerEvent] Invalid itemid - '" << strValue << "'" << std::endl;
			return false;
		}

		if(useItemMap.find(intVector[0]) != useItemMap.end())
		{
			if(!override)
			{
				std::clog << "[Warning - Actions::registerEvent] Duplicate registered item id: " << intVector[0] << std::endl;
				success = false;
			}
			else
				delete useItemMap[intVector[0]];
		}

		if(success)
			useItemMap[intVector[0]] = action;

		for(size_t i = 1, size = intVector.size(); i < size; ++i)
		{
			if(useItemMap.find(intVector[i]) != useItemMap.end())
			{
				if(!override)
				{
					std::clog << "[Warning - Actions::registerEvent] Duplicate registered item id: " << intVector[i] << std::endl;
					continue;
				}
				else
					delete useItemMap[intVector[i]];
			}

			useItemMap[intVector[i]] = new Action(action);
		}
	}
	else if(readXMLString(p, "fromid", strValue) && readXMLString(p, "toid", endValue))
	{
		IntegerVec intVector = vectorAtoi(explodeString(strValue, ";")), endVector = vectorAtoi(explodeString(endValue, ";"));
		if(intVector[0] && endVector[0] && intVector.size() == endVector.size())
		{
			int32_t tmp = 0;
			for(size_t i = 0, size = intVector.size(); i < size; ++i)
			{
				tmp = intVector[i];
				while(intVector[i] <= endVector[i])
				{
					if(useItemMap.find(intVector[i]) != useItemMap.end())
					{
						if(!override)
						{
							std::clog << "[Warning - Actions::registerEvent] Duplicate registered item with id: " << intVector[i] <<
								", in fromid: " << tmp << " and toid: " << endVector[i] << std::endl;
							intVector[i]++;
							continue;
						}
						else
							delete useItemMap[intVector[i]];
					}

					useItemMap[intVector[i]++] = new Action(action);
				}
			}
		}
		else
			std::clog << "[Warning - Actions::registerEvent] Malformed entry (from item: \"" << strValue <<
				"\", to item: \"" << endValue << "\")" << std::endl;
	}

	if(readXMLString(p, "uniqueid", strValue))
	{
		IntegerVec intVector;
		if(!parseIntegerVec(strValue, intVector))
		{
			std::clog << "[Warning - Actions::registerEvent] Invalid uniqueid - '" << strValue << "'" << std::endl;
			return false;
		}

		if(uniqueItemMap.find(intVector[0]) != uniqueItemMap.end())
		{
			if(!override)
			{
				std::clog << "[Warning - Actions::registerEvent] Duplicate registered item uid: " << intVector[0] << std::endl;
				success = false;
			}
			else
				delete uniqueItemMap[intVector[0]];
		}

		if(success)
			uniqueItemMap[intVector[0]] = action;

		for(size_t i = 1, size = intVector.size(); i < size; ++i)
		{
			if(uniqueItemMap.find(intVector[i]) != uniqueItemMap.end())
			{
				if(!override)
				{
					std::clog << "[Warning - Actions::registerEvent] Duplicate registered item uid: " << intVector[i] << std::endl;
					continue;
				}
				else
					delete uniqueItemMap[intVector[i]];
			}

			uniqueItemMap[intVector[i]] = new Action(action);
		}
	}
	else if(readXMLString(p, "fromuid", strValue) && readXMLString(p, "touid", endValue))
	{
		IntegerVec intVector = vectorAtoi(explodeString(strValue, ";")), endVector = vectorAtoi(explodeString(endValue, ";"));
		if(intVector[0] && endVector[0] && intVector.size() == endVector.size())
		{
			int32_t tmp = 0;
			for(size_t i = 0, size = intVector.size(); i < size; ++i)
			{
				tmp = intVector[i];
				while(intVector[i] <= endVector[i])
				{
					if(uniqueItemMap.find(intVector[i]) != uniqueItemMap.end())
					{
						if(!override)
						{
							std::clog << "[Warning - Actions::registerEvent] Duplicate registered item with uid: " << intVector[i] <<
								", in fromuid: " << tmp << " and touid: " << endVector[i] << std::endl;
							intVector[i]++;
							continue;
						}
						else
							delete uniqueItemMap[intVector[i]];
					}

					uniqueItemMap[intVector[i]++] = new Action(action);
				}
			}
		}
		else
			std::clog << "[Warning - Actions::registerEvent] Malformed entry (from unique: \"" << strValue <<
				"\", to unique: \"" << endValue << "\")" << std::endl;
	}

	if(readXMLString(p, "actionid", strValue) || readXMLString(p, "aid", strValue))
	{
		IntegerVec intVector;
		if(!parseIntegerVec(strValue, intVector))
		{
			std::clog << "[Warning - Actions::registerEvent] Invalid actionid - '" << strValue << "'" << std::endl;
			return false;
		}

		if(actionItemMap.find(intVector[0]) != actionItemMap.end())
		{
			if(!override)
			{
				std::clog << "[Warning - Actions::registerEvent] Duplicate registered item aid: " << intVector[0] << std::endl;
				success = false;
			}
			else
				delete actionItemMap[intVector[0]];
		}

		if(success)
			actionItemMap[intVector[0]] = action;

		for(size_t i = 1, size = intVector.size(); i < size; ++i)
		{
			if(actionItemMap.find(intVector[i]) != actionItemMap.end())
			{
				if(!override)
				{
					std::clog << "[Warning - Actions::registerEvent] Duplicate registered item aid: " << intVector[i] << std::endl;
					continue;
				}
				else
					delete actionItemMap[intVector[i]];
			}

			actionItemMap[intVector[i]] = new Action(action);
		}
	}
	else if(readXMLString(p, "fromaid", strValue) && readXMLString(p, "toaid", endValue))
	{
		IntegerVec intVector = vectorAtoi(explodeString(strValue, ";")), endVector = vectorAtoi(explodeString(endValue, ";"));
		if(intVector[0] && endVector[0] && intVector.size() == endVector.size())
		{
			int32_t tmp = 0;
			for(size_t i = 0, size = intVector.size(); i < size; ++i)
			{
				tmp = intVector[i];
				while(intVector[i] <= endVector[i])
				{
					if(actionItemMap.find(intVector[i]) != actionItemMap.end())
					{
						if(!override)
						{
							std::clog << "[Warning - Actions::registerEvent] Duplicate registered item with aid: " << intVector[i] <<
								", in fromaid: " << tmp << " and toaid: " << endVector[i] << std::endl;
							intVector[i]++;
							continue;
						}
						else
							delete actionItemMap[intVector[i]];
					}

					actionItemMap[intVector[i]++] = new Action(action);
				}
			}
		}
		else
			std::clog << "[Warning - Actions::registerEvent] Malformed entry (from action: \"" << strValue <<
				"\", to action: \"" << endValue << "\")" << std::endl;
	}

	return success;
}

ReturnValue Actions::canUse(const Player* player, const Position& pos)
{
	const Position& playerPos = player->getPosition();
	if(pos.x == 0xFFFF)
		return RET_NOERROR;

	if(playerPos.z > pos.z)
		return RET_FIRSTGOUPSTAIRS;

	if(playerPos.z < pos.z)
		return RET_FIRSTGODOWNSTAIRS;

	if(!Position::areInRange<1,1,0>(playerPos, pos))
		return RET_TOOFARAWAY;

	Tile* tile = g_game.getTile(pos);
	if(tile)
	{
		HouseTile* houseTile = tile->getHouseTile();
		if(houseTile && houseTile->getHouse() && !houseTile->getHouse()->isInvited(player))
			return RET_PLAYERISNOTINVITED;
	}

	return RET_NOERROR;
}

ReturnValue Actions::canUseEx(const Player* player, const Position& pos, const Item* item)
{
	Action* action = NULL;
	if((action = getAction(item, ACTION_UNIQUEID)))
		return action->canExecuteAction(player, pos);

	if((action = getAction(item, ACTION_ACTIONID)))
		return action->canExecuteAction(player, pos);

	if((action = getAction(item, ACTION_ITEMID)))
		return action->canExecuteAction(player, pos);

	if((action = getAction(item, ACTION_RUNEID)))
		return action->canExecuteAction(player, pos);

	if(defaultAction)
		return defaultAction->canExecuteAction(player, pos);

	return RET_NOERROR;
}

ReturnValue Actions::canUseFar(const Creature* creature, const Position& toPos, bool checkLineOfSight)
{
	if(toPos.x == 0xFFFF)
		return RET_NOERROR;

	const Position& creaturePos = creature->getPosition();
	if(creaturePos.z > toPos.z)
		return RET_FIRSTGOUPSTAIRS;

	if(creaturePos.z < toPos.z)
		return RET_FIRSTGODOWNSTAIRS;

	if(!Position::areInRange<7,5,0>(toPos, creaturePos))
		return RET_TOOFARAWAY;

	if(checkLineOfSight && !g_game.canThrowObjectTo(creaturePos, toPos))
		return RET_CANNOTTHROW;

	return RET_NOERROR;
}

Action* Actions::getAction(const Item* item, ActionType_t type) const
{
	if(item->getUniqueId() && (type == ACTION_ANY || type == ACTION_UNIQUEID))
	{
		ActionUseMap::const_iterator it = uniqueItemMap.find(item->getUniqueId());
		if(it != uniqueItemMap.end())
			return it->second;
	}

	if(item->getActionId() && (type == ACTION_ANY || type == ACTION_ACTIONID))
	{
		ActionUseMap::const_iterator it = actionItemMap.find(item->getActionId());
		if(it != actionItemMap.end())
			return it->second;
	}

	if(type == ACTION_ANY || type == ACTION_ITEMID)
	{
		ActionUseMap::const_iterator it = useItemMap.find(item->getID());
		if(it != useItemMap.end())
			return it->second;
	}

	if(type == ACTION_ANY || type == ACTION_RUNEID)
	{
		if(Action* runeSpell = g_spells->getRuneSpell(item->getID()))
			return runeSpell;
	}

	return NULL;
}

bool Actions::executeUse(Action* action, Player* player, Item* item,
	const PositionEx& posEx, uint32_t creatureId)
{
	return action->executeUse(player, item, posEx, posEx, false, creatureId);
}

ReturnValue Actions::internalUseItem(Player* player, const Position& pos, uint8_t index, Item* item, uint32_t creatureId)
{
	if(Door* door = item->getDoor())
	{
		if(!door->canUse(player))
			return RET_CANNOTUSETHISOBJECT;
	}

	int32_t tmp = 0;
	if(item->getParent())
		tmp = item->getParent()->__getIndexOfThing(item);

	PositionEx posEx(pos, tmp);
	Action* action = NULL;
	if((action = getAction(item, ACTION_UNIQUEID)))
	{
		if(executeUse(action, player, item, posEx, creatureId))
			return RET_NOERROR;
	}

	if((action = getAction(item, ACTION_ACTIONID)))
	{
		if(executeUse(action, player, item, posEx, creatureId))
			return RET_NOERROR;
	}

	if((action = getAction(item, ACTION_ITEMID)))
	{
		if(executeUse(action, player, item, posEx, creatureId))
			return RET_NOERROR;
	}

	if((action = getAction(item, ACTION_RUNEID)))
	{
		if(executeUse(action, player, item, posEx, creatureId))
			return RET_NOERROR;
	}

	if(defaultAction)
	{
		if(executeUse(defaultAction, player, item, posEx, creatureId))
			return RET_NOERROR;
	}

	if(BedItem* bed = item->getBed())
	{
		if(!bed->canUse(player))
			return RET_CANNOTUSETHISOBJECT;

		bed->sleep(player);
		return RET_NOERROR;
	}

	if(Container* container = item->getContainer())
	{
		if(container->getCorpseOwner() && !player->canOpenCorpse(container->getCorpseOwner())
			&& g_config.getBool(ConfigManager::CHECK_CORPSE_OWNER))
			return RET_YOUARENOTTHEOWNER;

		Container* tmpContainer = NULL;
		if(Depot* depot = container->getDepot())
		{
			if(player->hasFlag(PlayerFlag_CannotPickupItem))
				return RET_CANNOTUSETHISOBJECT;

			if(Depot* playerDepot = player->getDepot(depot->getDepotId(), true))
			{
				player->useDepot(depot->getDepotId(), true);
				playerDepot->setParent(depot->getParent());
				tmpContainer = playerDepot;
			}
		}

		if(!tmpContainer)
			tmpContainer = container;

		int32_t oldId = player->getContainerID(tmpContainer);
		if(oldId != -1)
		{
			player->onCloseContainer(tmpContainer);
			player->closeContainer(oldId);
		}
		else
		{
			player->addContainer(index, tmpContainer);
			player->onSendContainer(tmpContainer);
		}

		return RET_NOERROR;
	}

	if(item->isReadable())
	{
		if(item->canWriteText())
		{
			player->setWriteItem(item, item->getMaxWriteLength());
			player->sendTextWindow(item, item->getMaxWriteLength(), true);
		}
		else
		{
			player->setWriteItem(NULL);
			player->sendTextWindow(item, 0, false);
		}

		return RET_NOERROR;
	}

	const ItemType& it = Item::items[item->getID()];
	if(it.transformUseTo)
	{
		g_game.transformItem(item, it.transformUseTo);
		g_game.startDecay(item);
		return RET_NOERROR;
	}

	if(item->isPremiumScroll())
	{
		std::ostringstream ss;
		ss << " You have recived " << it.premiumDays << " premium days.";
		player->sendTextMessage(MSG_INFO_DESCR, ss.str());

		player->addPremiumDays(it.premiumDays);
		g_game.internalRemoveItem(NULL, item, 1);
		return RET_NOERROR;
	}

	return RET_CANNOTUSETHISOBJECT;
}

bool Actions::useItem(Player* player, const Position& pos, uint8_t index, Item* item)
{
	if(!player->canDoAction())
		return false;

	player->setNextActionTask(NULL);
	player->stopWalk();
	player->setNextAction(OTSYS_TIME() + g_config.getNumber(ConfigManager::ACTIONS_DELAY_INTERVAL) - 10);

	ReturnValue ret = internalUseItem(player, pos, index, item, 0);
	if(ret == RET_NOERROR)
		return true;

	player->sendCancelMessage(ret);
	return false;
}

bool Actions::executeUseEx(Action* action, Player* player, Item* item, const PositionEx& fromPosEx,
	const PositionEx& toPosEx, bool isHotkey, uint32_t creatureId)
{
	return (action->executeUse(player, item, fromPosEx, toPosEx, isHotkey,
		creatureId) || action->hasOwnErrorHandler());
}

ReturnValue Actions::internalUseItemEx(Player* player, const PositionEx& fromPosEx, const PositionEx& toPosEx,
	Item* item, bool isHotkey, uint32_t creatureId)
{
	Action* action = NULL;
	if((action = getAction(item, ACTION_UNIQUEID)))
	{
		ReturnValue ret = action->canExecuteAction(player, toPosEx);
		if(ret != RET_NOERROR)
			return ret;

		//only continue with next action in the list if the previous returns false
		if(executeUseEx(action, player, item, fromPosEx, toPosEx, isHotkey, creatureId))
			return RET_NOERROR;
	}

	if((action = getAction(item, ACTION_ACTIONID)))
	{
		ReturnValue ret = action->canExecuteAction(player, toPosEx);
		if(ret != RET_NOERROR)
			return ret;

		//only continue with next action in the list if the previous returns false
		if(executeUseEx(action, player, item, fromPosEx, toPosEx, isHotkey, creatureId))
			return RET_NOERROR;

	}

	if((action = getAction(item, ACTION_ITEMID)))
	{
		ReturnValue ret = action->canExecuteAction(player, toPosEx);
		if(ret != RET_NOERROR)
			return ret;

		//only continue with next action in the list if the previous returns false
		if(executeUseEx(action, player, item, fromPosEx, toPosEx, isHotkey, creatureId))
			return RET_NOERROR;
	}

	if((action = getAction(item, ACTION_RUNEID)))
	{
		ReturnValue ret = action->canExecuteAction(player, toPosEx);
		if(ret != RET_NOERROR)
			return ret;

		//only continue with next action in the list if the previous returns false
		if(executeUseEx(action, player, item, fromPosEx, toPosEx, isHotkey, creatureId))
			return RET_NOERROR;
	}

	if(defaultAction)
	{
		ReturnValue ret = defaultAction->canExecuteAction(player, toPosEx);
		if(ret != RET_NOERROR)
			return ret;

		//only continue with next action in the list if the previous returns false
		if(executeUseEx(defaultAction, player, item, fromPosEx, toPosEx, isHotkey, creatureId))
			return RET_NOERROR;
	}

	return RET_CANNOTUSETHISOBJECT;
}

bool Actions::useItemEx(Player* player, const Position& fromPos, const Position& toPos,
	uint8_t toStackPos, Item* item, bool isHotkey, uint32_t creatureId/* = 0*/)
{
	if(!player->canDoAction())
		return false;

	player->setNextActionTask(NULL);
	player->stopWalk();
	player->setNextAction(OTSYS_TIME() + g_config.getNumber(ConfigManager::EX_ACTIONS_DELAY_INTERVAL) - 10);

	int32_t fromStackPos = 0;
	if(item->getParent())
		fromStackPos = item->getParent()->__getIndexOfThing(item);

	PositionEx fromPosEx(fromPos, fromStackPos);
	PositionEx toPosEx(toPos, toStackPos);

	ReturnValue ret = internalUseItemEx(player, fromPosEx, toPosEx, item, isHotkey, creatureId);
	if(ret == RET_NOERROR)
		return true;

	player->sendCancelMessage(ret);
	return false;
}

Action::Action(LuaInterface* _interface):
Event(_interface)
{
	allowFarUse = false;
	checkLineOfSight = true;
}

Action::Action(const Action* copy):
Event(copy)
{
	allowFarUse = copy->allowFarUse;
	checkLineOfSight = copy->checkLineOfSight;
}

bool Action::configureEvent(xmlNodePtr p)
{
	std::string strValue;
	if(readXMLString(p, "allowfaruse", strValue) || readXMLString(p, "allowFarUse", strValue))
		setAllowFarUse(booleanString(strValue));

	if(readXMLString(p, "blockwalls", strValue) || readXMLString(p, "blockWalls", strValue))
		setCheckLineOfSight(booleanString(strValue));

	return true;
}

ReturnValue Action::canExecuteAction(const Player* player, const Position& pos)
{
	if(player->hasCustomFlag(PlayerCustomFlag_CanUseFar))
		return RET_NOERROR;

	if(!getAllowFarUse())
		return g_actions->canUse(player, pos);

	return g_actions->canUseFar(player, pos, getCheckLineOfSight());
}

bool Action::executeUse(Player* player, Item* item, const PositionEx& fromPos, const PositionEx& toPos, bool extendedUse, uint32_t)
{
	//onUse(cid, item, fromPosition, itemEx, toPosition)
	if(m_interface->reserveEnv())
	{
		ScriptEnviroment* env = m_interface->getEnv();
		if(m_scripted == EVENT_SCRIPT_BUFFER)
		{
			env->setRealPos(player->getPosition());
			std::ostringstream scriptstream;

			scriptstream << "local cid = " << env->addThing(player) << std::endl;
			env->streamThing(scriptstream, "item", item, env->addThing(item));
			env->streamPosition(scriptstream, "fromPosition", fromPos, fromPos.stackpos);

			Thing* thing = g_game.internalGetThing(player, toPos, toPos.stackpos);
			if(thing && (thing != item || !extendedUse))
			{
				env->streamThing(scriptstream, "itemEx", thing, env->addThing(thing));
				env->streamPosition(scriptstream, "toPosition", toPos, toPos.stackpos);
			}
			else
			{
				env->streamThing(scriptstream, "itemEx", NULL, 0);
				env->streamPosition(scriptstream, "toPosition", PositionEx());
			}

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
			std::ostringstream desc;
			desc << player->getName() << " - " << item->getID() << " " << fromPos << "|" << toPos;
			env->setEvent(desc.str());
			#endif

			env->setScriptId(m_scriptId, m_interface);
			env->setRealPos(player->getPosition());

			lua_State* L = m_interface->getState();
			m_interface->pushFunction(m_scriptId);

			lua_pushnumber(L, env->addThing(player));
			LuaInterface::pushThing(L, item, env->addThing(item));
			LuaInterface::pushPosition(L, fromPos, fromPos.stackpos);

			Thing* thing = g_game.internalGetThing(player, toPos, toPos.stackpos);
			if(thing && (thing != item || !extendedUse))
			{
				LuaInterface::pushThing(L, thing, env->addThing(thing));
				LuaInterface::pushPosition(L, toPos, toPos.stackpos);
			}
			else
			{
				LuaInterface::pushThing(L, NULL, 0);
				LuaInterface::pushPosition(L, PositionEx());
			}

			bool result = m_interface->callFunction(5);
			m_interface->releaseEnv();
			return result;
		}
	}
	else
	{
		std::clog << "[Error - Action::executeUse]: Call stack overflow." << std::endl;
		return false;
	}
}
