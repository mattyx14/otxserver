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
#include "luascript.h"
#include "scriptmanager.h"

#include <boost/filesystem.hpp>
#include <boost/any.hpp>
#include <iostream>
#include <iomanip>

#include "player.h"
#include "item.h"
#include "teleport.h"
#include "beds.h"

#include "town.h"
#include "house.h"
#include "housetile.h"

#include "database.h"
#include "databasemanager.h"
#include "iologindata.h"
#include "ioban.h"
#include "iomap.h"
#include "iomapserialize.h"

#include "monsters.h"
#include "movement.h"
#include "spells.h"
#include "talkaction.h"
#include "creatureevent.h"

#include "combat.h"
#include "condition.h"

#include "baseevents.h"
#include "raids.h"

#include "configmanager.h"
#include "vocation.h"
#include "status.h"
#include "game.h"
#include "chat.h"
#include "tools.h"

extern ConfigManager g_config;
extern Game g_game;
extern Chat g_chat;
extern Monsters g_monsters;
extern MoveEvents* g_moveEvents;
extern Spells* g_spells;
extern TalkActions* g_talkActions;
extern CreatureEvents* g_creatureEvents;

enum
{
	EVENT_ID_LOADING = 1,
	EVENT_ID_USER = 1000,
};

ScriptEnviroment::AreaMap ScriptEnviroment::m_areaMap;
uint32_t ScriptEnviroment::m_lastAreaId = 0;
ScriptEnviroment::CombatMap ScriptEnviroment::m_combatMap;
uint32_t ScriptEnviroment::m_lastCombatId = 0;
ScriptEnviroment::ConditionMap ScriptEnviroment::m_conditionMap;
uint32_t ScriptEnviroment::m_lastConditionId = 0;
ScriptEnviroment::ConditionMap ScriptEnviroment::m_tempConditionMap;
uint32_t ScriptEnviroment::m_lastTempConditionId = 0;

ScriptEnviroment::ThingMap ScriptEnviroment::m_globalMap;
ScriptEnviroment::TempItemListMap ScriptEnviroment::m_tempItems;
StorageMap ScriptEnviroment::m_storageMap;

ScriptEnviroment::ScriptEnviroment()
{
	m_lastUID = 70000;
	m_loaded = true;
	reset();
}

ScriptEnviroment::~ScriptEnviroment()
{
	for(CombatMap::iterator it = m_combatMap.begin(); it != m_combatMap.end(); ++it)
		delete it->second;

	m_combatMap.clear();
	for(AreaMap::iterator it = m_areaMap.begin(); it != m_areaMap.end(); ++it)
		delete it->second;

	m_areaMap.clear();
	for(ConditionMap::iterator it = m_conditionMap.begin(); it != m_conditionMap.end(); ++it)
		delete it->second;

	m_conditionMap.clear();
	reset();
}

void ScriptEnviroment::reset()
{
	m_scriptId = m_callbackId = 0;
	m_realPos = Position();
	m_timerEvent = false;

	m_interface = NULL;
	for(TempItemListMap::iterator mit = m_tempItems.begin(); mit != m_tempItems.end(); ++mit)
	{
		for(ItemList::iterator it = mit->second.begin(); it != mit->second.end(); ++it)
		{
			if((*it)->getParent() == VirtualCylinder::virtualCylinder)
				g_game.freeThing(*it);
		}
	}

	m_tempItems.clear();
	for(DBResultMap::iterator it = m_tempResults.begin(); it != m_tempResults.end(); ++it)
	{
		if(it->second)
			it->second->free();
	}

	m_tempResults.clear();
	for(ConditionMap::iterator it = m_tempConditionMap.begin(); it != m_tempConditionMap.end(); ++it)
		delete it->second;

	m_tempConditionMap.clear();
	m_localMap.clear();
}

bool ScriptEnviroment::saveGameState()
{
	if(!g_config.getBool(ConfigManager::SAVE_GLOBAL_STORAGE))
		return true;

	Database* db = Database::getInstance();
	DBQuery query;

	query << "DELETE FROM `global_storage` WHERE `world_id` = " << g_config.getNumber(ConfigManager::WORLD_ID) << ";";
	if(!db->query(query.str()))
		return false;

	DBInsert stmt(db);
	stmt.setQuery("INSERT INTO `global_storage` (`key`, `world_id`, `value`) VALUES ");
	for(StorageMap::const_iterator it = m_storageMap.begin(); it != m_storageMap.end(); ++it)
	{
		query.str("");
		query << db->escapeString(it->first) << "," << g_config.getNumber(ConfigManager::WORLD_ID) << "," << db->escapeString(it->second);
		if(!stmt.addRow(query))
			return false;
	}

	return stmt.execute();
}

bool ScriptEnviroment::loadGameState()
{
	Database* db = Database::getInstance();
	DBResult* result;

	DBQuery query;
	query << "SELECT `key`, `value` FROM `global_storage` WHERE `world_id` = " << g_config.getNumber(ConfigManager::WORLD_ID) << ";";
	if((result = db->storeQuery(query.str())))
	{
		do
			m_storageMap[result->getDataString("key")] = result->getDataString("value");
		while(result->next());
		result->free();
	}

	query.str("");
	return true;
}

bool ScriptEnviroment::setCallbackId(int32_t callbackId, LuaInterface* interface)
{
	if(!m_callbackId)
	{
		m_callbackId = callbackId;
		m_interface = interface;
		return true;
	}

	//nested callbacks are not allowed
	if(m_interface)
		m_interface->errorEx("Nested callbacks!");

	return false;
}

void ScriptEnviroment::getInfo(int32_t& scriptId, std::string& desc, LuaInterface*& interface, int32_t& callbackId, bool& timerEvent)
{
	scriptId = m_scriptId;
	desc = m_event;
	interface = m_interface;
	callbackId = m_callbackId;
	timerEvent = m_timerEvent;
}

void ScriptEnviroment::addUniqueThing(Thing* thing)
{
	Item* item = thing->getItem();
	if(!item || !item->getUniqueId())
		return;

	if(m_globalMap[item->getUniqueId()])
	{
		if(item->getActionId() != 2000) //scripted quest system
			std::clog << "Duplicate uniqueId " << item->getUniqueId() << std::endl;
	}
	else
		m_globalMap[item->getUniqueId()] = thing;
}

void ScriptEnviroment::removeUniqueThing(Thing* thing)
{
	Item* item = thing->getItem();
	if(!item || !item->getUniqueId())
		return;

	ThingMap::iterator it = m_globalMap.find(item->getUniqueId());
	if(it != m_globalMap.end())
		m_globalMap.erase(it);
}

uint32_t ScriptEnviroment::addThing(Thing* thing)
{
	if(!thing || thing->isRemoved())
		return 0;

	for(ThingMap::iterator it = m_localMap.begin(); it != m_localMap.end(); ++it)
	{
		if(it->second == thing)
			return it->first;
	}

	if(Creature* creature = thing->getCreature())
	{
		m_localMap[creature->getID()] = thing;
		return creature->getID();
	}

	if(Item* item = thing->getItem())
	{
		uint32_t tmp = item->getUniqueId();
		if(tmp)
		{
			m_localMap[tmp] = thing;
			return tmp;
		}
	}

	while(m_localMap.find(m_lastUID) != m_localMap.end())
		++m_lastUID;

	m_localMap[m_lastUID] = thing;
	return m_lastUID;
}

void ScriptEnviroment::insertThing(uint32_t uid, Thing* thing)
{
	if(m_localMap[uid])
		std::clog << "[Error - ScriptEnviroment::insertThing] Thing uid already taken" << std::endl;
	else
		m_localMap[uid] = thing;
}

Thing* ScriptEnviroment::getThingByUID(uint32_t uid)
{
	Thing* tmp = m_localMap[uid];
	if(tmp && !tmp->isRemoved())
		return tmp;

	tmp = m_globalMap[uid];
	if(tmp && !tmp->isRemoved())
		return tmp;

	if(uid < PLAYER_ID_RANGE)
		return NULL;

	if(!(tmp = g_game.getCreatureByID(uid)) || tmp->isRemoved())
		return NULL;

	m_localMap[uid] = tmp;
	return tmp;
}

Item* ScriptEnviroment::getItemByUID(uint32_t uid)
{
	if(Thing* tmp = getThingByUID(uid))
	{
		if(Item* item = tmp->getItem())
			return item;
	}

	return NULL;
}

Container* ScriptEnviroment::getContainerByUID(uint32_t uid)
{
	if(Item* tmp = getItemByUID(uid))
	{
		if(Container* container = tmp->getContainer())
			return container;
	}

	return NULL;
}

Creature* ScriptEnviroment::getCreatureByUID(uint32_t uid)
{
	if(Thing* tmp = getThingByUID(uid))
	{
		if(Creature* creature = tmp->getCreature())
			return creature;
	}

	return NULL;
}

Player* ScriptEnviroment::getPlayerByUID(uint32_t uid)
{
	if(Thing* tmp = getThingByUID(uid))
	{
		if(Creature* creature = tmp->getCreature())
		{
			if(Player* player = creature->getPlayer())
				return player;
		}
	}

	return NULL;
}

void ScriptEnviroment::removeThing(uint32_t uid)
{
	ThingMap::iterator it;
	it = m_localMap.find(uid);
	if(it != m_localMap.end())
		m_localMap.erase(it);

	it = m_globalMap.find(uid);
	if(it != m_globalMap.end())
		m_globalMap.erase(it);
}

uint32_t ScriptEnviroment::addCombatArea(CombatArea* area)
{
	uint32_t newAreaId = m_lastAreaId + 1;
	m_areaMap[newAreaId] = area;

	m_lastAreaId++;
	return newAreaId;
}

CombatArea* ScriptEnviroment::getCombatArea(uint32_t areaId)
{
	AreaMap::const_iterator it = m_areaMap.find(areaId);
	if(it != m_areaMap.end())
		return it->second;

	return NULL;
}

uint32_t ScriptEnviroment::addCombatObject(Combat* combat)
{
	uint32_t newCombatId = m_lastCombatId + 1;
	m_combatMap[newCombatId] = combat;

	m_lastCombatId++;
	return newCombatId;
}

Combat* ScriptEnviroment::getCombatObject(uint32_t combatId)
{
	CombatMap::iterator it = m_combatMap.find(combatId);
	if(it != m_combatMap.end())
		return it->second;

	return NULL;
}

uint32_t ScriptEnviroment::addConditionObject(Condition* condition)
{
	m_conditionMap[++m_lastConditionId] = condition;
	return m_lastConditionId;
}

uint32_t ScriptEnviroment::addTempConditionObject(Condition* condition)
{
	m_tempConditionMap[++m_lastTempConditionId] = condition;
	return m_lastTempConditionId;
}

Condition* ScriptEnviroment::getConditionObject(uint32_t conditionId, bool loaded)
{
	ConditionMap::iterator it;
	if(loaded)
	{
		it = m_conditionMap.find(conditionId);
		if(it != m_conditionMap.end())
			return it->second;
	}
	else
	{
		it = m_tempConditionMap.find(conditionId);
		if(it != m_tempConditionMap.end())
			return it->second;
	}

	return NULL;
}

void ScriptEnviroment::addTempItem(ScriptEnviroment* env, Item* item)
{
	m_tempItems[env].push_back(item);
}

void ScriptEnviroment::removeTempItem(ScriptEnviroment* env, Item* item)
{
	ItemList::iterator it = std::find(m_tempItems[env].begin(), m_tempItems[env].end(), item);
	if(it != m_tempItems[env].end())
		m_tempItems[env].erase(it);
}

void ScriptEnviroment::removeTempItem(Item* item)
{
	ItemList::iterator it;
	for(TempItemListMap::iterator mit = m_tempItems.begin(); mit != m_tempItems.end(); ++mit)
	{
		it = std::find(mit->second.begin(), mit->second.end(), item);
		if(it != mit->second.end())
			mit->second.erase(it);
	}
}

uint32_t ScriptEnviroment::addResult(DBResult* res)
{
	uint32_t lastId = 0;
	while(m_tempResults.find(lastId) != m_tempResults.end())
		lastId++;

	m_tempResults[lastId] = res;
	return lastId;
}

bool ScriptEnviroment::removeResult(uint32_t id)
{
	DBResultMap::iterator it = m_tempResults.find(id);
	if(it == m_tempResults.end())
		return false;

	if(it->second)
		it->second->free();

	m_tempResults.erase(it);
	return true;
}

DBResult* ScriptEnviroment::getResultByID(uint32_t id)
{
	DBResultMap::iterator it = m_tempResults.find(id);
	if(it != m_tempResults.end())
		return it->second;

	return NULL;
}

bool ScriptEnviroment::getStorage(const std::string& key, std::string& value) const
{
	StorageMap::const_iterator it = m_storageMap.find(key);
	if(it != m_storageMap.end())
	{
		value = it->second;
		return true;
	}

	value = "-1";
	return false;
}

void ScriptEnviroment::streamVariant(std::stringstream& stream, const std::string& local, const LuaVariant& var)
{
	if(!local.empty())
		stream << "local " << local << " = {" << std::endl;

	stream << "type = " << var.type;
	switch(var.type)
	{
		case VARIANT_NUMBER:
			stream << "," << std::endl << "number = " << var.number;
			break;
		case VARIANT_STRING:
			stream << "," << std::endl << "string = \"" << var.text << "\"";
			break;
		case VARIANT_TARGETPOSITION:
		case VARIANT_POSITION:
		{
			stream << "," << std::endl;
			streamPosition(stream, "pos", var.pos);
			break;
		}
		case VARIANT_NONE:
		default:
			break;
	}

	if(!local.empty())
		stream << std::endl << "}" << std::endl;
}

void ScriptEnviroment::streamThing(std::stringstream& stream, const std::string& local, Thing* thing, uint32_t id/* = 0*/)
{
	if(!local.empty())
		stream << "local " << local << " = {" << std::endl;

	if(thing && thing->getItem())
	{
		const Item* item = thing->getItem();
		if(!id)
			id = addThing(thing);

		stream << "uniqueid = " << id << "," << std::endl;
		stream << "uid = " << id << "," << std::endl;
		stream << "itemid = " << item->getID() << "," << std::endl;
		stream << "id = " << item->getID() << "," << std::endl;
		if(item->hasSubType())
			stream << "type = " << item->getSubType() << "," << std::endl;
		else
			stream << "type = 0," << std::endl;

		stream << "actionid = " << item->getActionId() << "," << std::endl;
		stream << "aid = " << item->getActionId() << std::endl;
	}
	else if(thing && thing->getCreature())
	{
		const Creature* creature = thing->getCreature();
		if(!id)
			id = creature->getID();

		stream << "uniqueid = " << id << "," << std::endl;
		stream << "uid = " << id << "," << std::endl;
		stream << "itemid = 1," << std::endl;
		stream << "id = 1," << std::endl;
		if(creature->getPlayer())
			stream << "type = 1," << std::endl;
		else if(creature->getMonster())
			stream << "type = 2," << std::endl;
		else
			stream << "type = 3," << std::endl;

		if(const Player* player = creature->getPlayer())
		{
			stream << "actionid = " << player->getGUID() << "," << std::endl;
			stream << "aid = " << player->getGUID() << std::endl;
		}
		else
		{
			stream << "actionid = 0," << std::endl;
			stream << "aid = 0" << std::endl;
		}
	}
	else
	{
		stream << "uniqueid = 0," << std::endl;
		stream << "uid = 0," << std::endl;
		stream << "itemid = 0," << std::endl;
		stream << "id = 0," << std::endl;
		stream << "type = 0," << std::endl;
		stream << "aid = 0," << std::endl;
		stream << "actionid = 0" << std::endl;
	}

	if(!local.empty())
		stream << "}" << std::endl;
}

void ScriptEnviroment::streamPosition(std::stringstream& stream, const std::string& local, const Position& position, uint32_t stackpos)
{
	if(!local.empty())
		stream << "local " << local << " = {" << std::endl;

	stream << "x = " << position.x << "," << std::endl;
	stream << "y = " << position.y << "," << std::endl;
	stream << "z = " << position.z << "," << std::endl;

	stream << "stackpos = " << stackpos << std::endl;
	if(!local.empty())
		stream << "}" << std::endl;
}

void ScriptEnviroment::streamOutfit(std::stringstream& stream, const std::string& local, const Outfit_t& outfit)
{
	if(!local.empty())
		stream << "local " << local << " = {" << std::endl;

	stream << "lookType = " << outfit.lookType << "," << std::endl;
	stream << "lookTypeEx = " << outfit.lookTypeEx << "," << std::endl;

	stream << "lookHead = " << outfit.lookHead << "," << std::endl;
	stream << "lookBody = " << outfit.lookBody << "," << std::endl;
	stream << "lookLegs = " << outfit.lookLegs << "," << std::endl;
	stream << "lookFeet = " << outfit.lookFeet << "," << std::endl;

	stream << "lookAddons = " << outfit.lookAddons << std::endl;
	if(!local.empty())
		stream << "}" << std::endl;
}

std::string LuaInterface::getError(ErrorCode_t code)
{
	switch(code)
	{
		case LUA_ERROR_PLAYER_NOT_FOUND:
			return "Player not found";
		case LUA_ERROR_MONSTER_NOT_FOUND:
			return "Monster not found";
		case LUA_ERROR_NPC_NOT_FOUND:
			return "NPC not found";
		case LUA_ERROR_CREATURE_NOT_FOUND:
			return "Creature not found";
		case LUA_ERROR_ITEM_NOT_FOUND:
			return "Item not found";
		case LUA_ERROR_THING_NOT_FOUND:
			return "Thing not found";
		case LUA_ERROR_TILE_NOT_FOUND:
			return "Tile not found";
		case LUA_ERROR_HOUSE_NOT_FOUND:
			return "House not found";
		case LUA_ERROR_COMBAT_NOT_FOUND:
			return "Combat not found";
		case LUA_ERROR_CONDITION_NOT_FOUND:
			return "Condition not found";
		case LUA_ERROR_AREA_NOT_FOUND:
			return "Area not found";
		case LUA_ERROR_CONTAINER_NOT_FOUND:
			return "Container not found";
		case LUA_ERROR_VARIANT_NOT_FOUND:
			return "Variant not found";
		case LUA_ERROR_VARIANT_UNKNOWN:
			return "Unknown variant type";
		case LUA_ERROR_SPELL_NOT_FOUND:
			return "Spell not found";
		default:
			break;
	}

	return "Invalid error code!";
}

ScriptEnviroment LuaInterface::m_scriptEnv[21];
int32_t LuaInterface::m_scriptEnvIndex = -1;

LuaInterface::LuaInterface(std::string interfaceName)
{
	m_luaState = NULL;
	m_interfaceName = interfaceName;
	m_lastTimer = 1000;
	m_errors = true;
}

LuaInterface::~LuaInterface()
{
	for(LuaTimerEvents::iterator it = m_timerEvents.begin(); it != m_timerEvents.end(); ++it)
		Scheduler::getInstance().stopEvent(it->second.eventId);

	closeState();
}

bool LuaInterface::reInitState()
{
	closeState();
	return initState();
}

bool LuaInterface::loadBuffer(const std::string& text, Npc* npc/* = NULL*/)
{
	//loads buffer as a chunk at stack top
	int32_t ret = luaL_loadbuffer(m_luaState, text.c_str(), text.length(), "LuaInterface::loadBuffer");
	if(ret)
	{
		m_lastError = popString(m_luaState);
		std::clog << "[Error - LuaInterface::loadBuffer] " << m_lastError << std::endl;
		return false;
	}

	//check that it is loaded as a function
	if(!lua_isfunction(m_luaState, -1))
		return false;

	m_loadingFile = text;
	reserveEnv();

	ScriptEnviroment* env = getEnv();
	env->setScriptId(EVENT_ID_LOADING, this);
	env->setNpc(npc);

	//execute it
	ret = lua_pcall(m_luaState, 0, 0, 0);
	if(ret)
	{
		error(NULL, popString(m_luaState));
		releaseEnv();
		return false;
	}

	releaseEnv();
	return true;
}

bool LuaInterface::loadFile(const std::string& file, Npc* npc/* = NULL*/)
{
	//loads file as a chunk at stack top
	int32_t ret = luaL_loadfile(m_luaState, file.c_str());
	if(ret)
	{
		m_lastError = popString(m_luaState);
		std::clog << "[Error - LuaInterface::loadFile] " << m_lastError << std::endl;
		return false;
	}

	//check that it is loaded as a function
	if(!lua_isfunction(m_luaState, -1))
		return false;

	m_loadingFile = file;
	reserveEnv();

	ScriptEnviroment* env = getEnv();
	env->setScriptId(EVENT_ID_LOADING, this);
	env->setNpc(npc);

	//execute it
	ret = lua_pcall(m_luaState, 0, 0, 0);
	if(ret)
	{
		error(NULL, popString(m_luaState));
		releaseEnv();
		return false;
	}

	releaseEnv();
	return true;
}

bool LuaInterface::loadDirectory(std::string dir, bool recursively, bool loadSystems, Npc* npc/* = NULL*/)
{
	if(dir[dir.size() - 1] != '/')
		dir += '/';

	StringVec files;
	for(boost::filesystem::directory_iterator it(dir), end; it != end; ++it)
	{
		std::string s = BOOST_DIR_ITER_FILENAME(it);
		if(!loadSystems && s[0] == '_')
			continue;

		if(boost::filesystem::is_directory(it->status()))
		{
			if(recursively && !loadDirectory(dir + s, recursively, loadSystems, npc))
				return false;
		}
		else if((s.size() > 4 ? s.substr(s.size() - 4) : "") == ".lua")
			files.push_back(s);
	}

	std::sort(files.begin(), files.end());
	for(StringVec::iterator it = files.begin(); it != files.end(); ++it)
	{
		if(!loadFile(dir + (*it), npc))
			return false;
	}

	return true;
}

int32_t LuaInterface::getEvent(const std::string& eventName)
{
	//get our events table
	lua_getfield(m_luaState, LUA_REGISTRYINDEX, "EVENTS");
	if(!lua_istable(m_luaState, -1))
	{
		lua_pop(m_luaState, 1);
		return -1;
	}

	//get current event function pointer
	lua_getglobal(m_luaState, eventName.c_str());
	if(!lua_isfunction(m_luaState, -1))
	{
		lua_pop(m_luaState, 1);
		return -1;
	}

	//save in our events table
	lua_pushnumber(m_luaState, m_runningEvent);
	lua_pushvalue(m_luaState, -2);

	lua_rawset(m_luaState, -4);
	lua_pop(m_luaState, 2);

	//reset global value of this event
	lua_pushnil(m_luaState);
	lua_setglobal(m_luaState, eventName.c_str());

	m_cacheFiles[m_runningEvent] = m_loadingFile + ":" + eventName;
	++m_runningEvent;
	return m_runningEvent - 1;
}

std::string LuaInterface::getScript(int32_t scriptId)
{
	if(scriptId == EVENT_ID_LOADING)
		return m_loadingFile;

	ScriptsCache::iterator it = m_cacheFiles.find(scriptId);
	if(it != m_cacheFiles.end())
		return it->second;

	return "(Unknown script file)";
}

void LuaInterface::error(const char* function, const std::string& desc)
{
	if(g_config.getBool(ConfigManager::SILENT_LUA))
		return;

	int32_t script, callback;
	bool timer;
	std::string event;

	LuaInterface* interface;
	getEnv()->getInfo(script, event, interface, callback, timer);
	if(interface)
	{
		if(!interface->m_errors)
			return;

		std::clog << std::endl << "[Error - " << interface->getName() << "] " << std::endl;
		if(callback)
			std::clog << "In a callback: " << interface->getScript(callback) << std::endl;

		if(timer)
			std::clog << (callback ? "from" : "In") << " a timer event called from: " << std::endl;

		std::clog << interface->getScript(script) << std::endl << "Description: ";
	}
	else
		std::clog << std::endl << "[Lua Error] ";

	std::clog << event << std::endl;
	if(function)
		std::clog << "(" << function << ") ";

	std::clog << desc << std::endl;
}

bool LuaInterface::pushFunction(int32_t function)
{
	lua_getfield(m_luaState, LUA_REGISTRYINDEX, "EVENTS");
	if(!lua_istable(m_luaState, -1))
		return false;

	lua_pushnumber(m_luaState, function);
	lua_rawget(m_luaState, -2);

	lua_remove(m_luaState, -2);
	return lua_isfunction(m_luaState, -1);
}

bool LuaInterface::initState()
{
	m_luaState = luaL_newstate();
	if(!m_luaState)
		return false;

	luaL_openlibs(m_luaState);
#ifdef __LUAJIT__
	luaJIT_setmode(m_luaState, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON);
#endif

	registerFunctions();
	if(!loadDirectory(getFilePath(FILE_TYPE_OTHER, "lib/"), false, true))
		std::clog << "[Warning - LuaInterface::initState] Cannot load " << getFilePath(FILE_TYPE_OTHER, "lib/") << std::endl;

	lua_newtable(m_luaState);
	lua_setfield(m_luaState, LUA_REGISTRYINDEX, "EVENTS");
	m_runningEvent = EVENT_ID_USER;
	return true;
}

bool LuaInterface::closeState()
{
	if(!m_luaState)
		return false;

	m_cacheFiles.clear();
	for(LuaTimerEvents::iterator it = m_timerEvents.begin(); it != m_timerEvents.end(); ++it)
	{
		for(std::list<int32_t>::iterator lt = it->second.parameters.begin(); lt != it->second.parameters.end(); ++lt)
			luaL_unref(m_luaState, LUA_REGISTRYINDEX, *lt);

		it->second.parameters.clear();
		luaL_unref(m_luaState, LUA_REGISTRYINDEX, it->second.function);
	}

	m_timerEvents.clear();
	lua_close(m_luaState);
	return true;
}

void LuaInterface::executeTimer(uint32_t eventIndex)
{
	LuaTimerEvents::iterator it = m_timerEvents.find(eventIndex);
	if(it == m_timerEvents.end())
		return;

	//push function
	lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, it->second.function);
	//push parameters
	for(std::list<int32_t>::reverse_iterator rt = it->second.parameters.rbegin(); rt != it->second.parameters.rend(); ++rt)
		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, *rt);

	//call the function
	if(reserveEnv())
	{
		ScriptEnviroment* env = getEnv();
		env->setTimerEvent();

		env->setScriptId(it->second.scriptId, this);
		env->setNpc(it->second.npc);

		callFunction(it->second.parameters.size());
		releaseEnv();
	}
	else
		std::clog << "[Error - LuaInterface::executeTimer] Call stack overflow." << std::endl;

	//free resources
	for(std::list<int32_t>::iterator lt = it->second.parameters.begin(); lt != it->second.parameters.end(); ++lt)
		luaL_unref(m_luaState, LUA_REGISTRYINDEX, *lt);

	it->second.parameters.clear();
	luaL_unref(m_luaState, LUA_REGISTRYINDEX, it->second.function);
	m_timerEvents.erase(it);
}

int32_t LuaInterface::handleFunction(lua_State* L)
{
	lua_getfield(L, LUA_GLOBALSINDEX, "debug");
	if(!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		return 1;
	}

	lua_getfield(L, -1, "traceback");
	if(!lua_isfunction(L, -1))
	{
		lua_pop(L, 2);
		return 1;
	}

	lua_pushvalue(L, 1);
	lua_pushinteger(L, 2);

	lua_call(L, 2, 1);
	return 1;
}

bool LuaInterface::callFunction(uint32_t params)
{
	int32_t size = lua_gettop(m_luaState), handler = lua_gettop(m_luaState) - params;
	lua_pushcfunction(m_luaState, handleFunction);

	bool result = false;
	lua_insert(m_luaState, handler);
	if(lua_pcall(m_luaState, params, 1, handler))
		LuaInterface::error(NULL, LuaInterface::popString(m_luaState));
	else
		result = (int32_t)LuaInterface::popBoolean(m_luaState);

	lua_remove(m_luaState, handler);
	if((lua_gettop(m_luaState) + (int32_t)params + 1) != size)
		LuaInterface::error(NULL, "Stack size changed!");

	return result;
}

void LuaInterface::dumpStack(lua_State* L/* = NULL*/)
{
	if(!L)
		L = m_luaState;

	int32_t stack = lua_gettop(L);
	if(!stack)
		return;

	std::clog << "Stack size: " << stack << std::endl;
	for(int32_t i = 1; i <= stack ; ++i)
		std::clog << lua_typename(m_luaState, lua_type(m_luaState, -i)) << " " << lua_topointer(m_luaState, -i) << std::endl;
}

void LuaInterface::pushVariant(lua_State* L, const LuaVariant& var)
{
	lua_newtable(L);
	setField(L, "type", var.type);
	switch(var.type)
	{
		case VARIANT_NUMBER:
			setField(L, "number", var.number);
			break;
		case VARIANT_STRING:
			setField(L, "string", var.text);
			break;
		case VARIANT_TARGETPOSITION:
		case VARIANT_POSITION:
		{
			lua_pushstring(L, "pos");
			pushPosition(L, var.pos);
			pushTable(L);
			break;
		}
		case VARIANT_NONE:
			break;
	}
}

void LuaInterface::pushThing(lua_State* L, Thing* thing, uint32_t id/* = 0*/, Recursive_t recursive/* = RECURSE_FIRST*/)
{
	lua_newtable(L);
	if(thing && thing->getItem())
	{
		const Item* item = thing->getItem();
		if(!id)
			id = getEnv()->addThing(thing);

		setField(L, "uniqueid", id);
		setField(L, "uid", id);
		setField(L, "itemid", item->getID());
		setField(L, "id", item->getID());
		if(item->hasSubType())
			setField(L, "type", item->getSubType());
		else
			setField(L, "type", 0);

		setField(L, "actionid", item->getActionId());
		setField(L, "aid", item->getActionId());
		if(recursive != RECURSE_NONE)
		{
			if(const Container* container = item->getContainer())
			{
				if(recursive == RECURSE_FIRST)
					recursive = RECURSE_NONE;

				ItemList::const_iterator it = container->getItems();
				createTable(L, "items");
				for(int32_t i = 1; it != container->getEnd(); ++it, ++i)
				{
					lua_pushnumber(L, i);
					pushThing(L, *it, getEnv()->addThing(*it), recursive);
					pushTable(L);
				}

				pushTable(L);
			}
		}
	}
	else if(thing && thing->getCreature())
	{
		const Creature* creature = thing->getCreature();
		if(!id)
			id = creature->getID();

		setField(L, "uniqueid", id);
		setField(L, "uid", id);
		setField(L, "itemid", 1);
		setField(L, "id", 1);
		if(creature->getPlayer())
			setField(L, "type", 1);
		else if(creature->getMonster())
			setField(L, "type", 2);
		else
			setField(L, "type", 3);

		if(const Player* player = creature->getPlayer())
		{
			setField(L, "actionid", player->getGUID());
			setField(L, "aid", player->getGUID());
		}
		else
		{
			setField(L, "actionid", 0);
			setField(L, "aid", 0);
		}
	}
	else
	{
		setField(L, "uid", 0);
		setField(L, "uniqueid", 0);
		setField(L, "itemid", 0);
		setField(L, "id", 0);
		setField(L, "type", 0);
		setField(L, "actionid", 0);
		setField(L, "aid", 0);
	}
}

void LuaInterface::pushPosition(lua_State* L, const Position& position, uint32_t stackpos)
{
	lua_newtable(L);
	setField(L, "x", position.x);
	setField(L, "y", position.y);
	setField(L, "z", position.z);
	setField(L, "stackpos", stackpos);
}

void LuaInterface::pushOutfit(lua_State* L, const Outfit_t& outfit)
{
	lua_newtable(L);
	setField(L, "lookType", outfit.lookType);
	setField(L, "lookTypeEx", outfit.lookTypeEx);
	setField(L, "lookHead", outfit.lookHead);
	setField(L, "lookBody", outfit.lookBody);
	setField(L, "lookLegs", outfit.lookLegs);
	setField(L, "lookFeet", outfit.lookFeet);
	setField(L, "lookAddons", outfit.lookAddons);
}

void LuaInterface::pushCallback(lua_State* L, int32_t callback)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, callback);
}

LuaVariant LuaInterface::popVariant(lua_State* L)
{
	LuaVariant var;
	var.type = (LuaVariantType_t)getField(L, "type");
	switch(var.type)
	{
		case VARIANT_NUMBER:
			var.number = getFieldUnsigned(L, "number");
			break;
		case VARIANT_STRING:
			var.text = getField(L, "string");
			break;
		case VARIANT_POSITION:
		case VARIANT_TARGETPOSITION:
		{
			lua_pushstring(L, "pos");
			lua_gettable(L, -2);
			popPosition(L, var.pos);
			break;
		}
		default:
			var.type = VARIANT_NONE;
			break;
	}

	lua_pop(L, 1); //table
	return var;
}

void LuaInterface::popPosition(lua_State* L, PositionEx& position)
{
	if(!lua_isboolean(L, -1))
	{
		position.x = getField(L, "x");
		position.y = getField(L, "y");
		position.z = getField(L, "z");
		position.stackpos = getField(L, "stackpos");
	}
	else
		position = PositionEx();

	lua_pop(L, 1); //table
}

void LuaInterface::popPosition(lua_State* L, Position& position, uint32_t& stackpos)
{
	stackpos = 0;
	if(!lua_isboolean(L, -1))
	{
		position.x = getField(L, "x");
		position.y = getField(L, "y");
		position.z = getField(L, "z");
		stackpos = getField(L, "stackpos");
	}
	else
		position = Position();

	lua_pop(L, 1); //table
}

bool LuaInterface::popBoolean(lua_State* L)
{
	lua_pop(L, 1);
	return (lua_toboolean(L, 0) != 0);
}

int64_t LuaInterface::popNumber(lua_State* L)
{
	lua_pop(L, 1);
	if(lua_isboolean(L, 0))
		return (int64_t)lua_toboolean(L, 0);

	return (int64_t)lua_tonumber(L, 0);
}

double LuaInterface::popFloatNumber(lua_State* L)
{
	lua_pop(L, 1);
	return lua_tonumber(L, 0);
}

std::string LuaInterface::popString(lua_State* L)
{
	lua_pop(L, 1);
	if(!lua_isstring(L, 0) && !lua_isnumber(L, 0))
		return std::string();

	const char* str = lua_tostring(L, 0);
	if(*str == '\0')
		return std::string();

	return str;
}

int32_t LuaInterface::popCallback(lua_State* L)
{
	return luaL_ref(L, LUA_REGISTRYINDEX);
}

Outfit_t LuaInterface::popOutfit(lua_State* L)
{
	Outfit_t outfit;
	outfit.lookAddons = getField(L, "lookAddons");

	outfit.lookFeet = getField(L, "lookFeet");
	outfit.lookLegs = getField(L, "lookLegs");
	outfit.lookBody = getField(L, "lookBody");
	outfit.lookHead = getField(L, "lookHead");

	outfit.lookTypeEx = getField(L, "lookTypeEx");
	outfit.lookType = getField(L, "lookType");

	lua_pop(L, 1); //table
	return outfit;
}

void LuaInterface::setField(lua_State* L, const char* index, int32_t val)
{
	lua_pushstring(L, index);
	lua_pushnumber(L, val);
	pushTable(L);
}

void LuaInterface::setField(lua_State* L, const char* index, const std::string& val)
{
	lua_pushstring(L, index);
	lua_pushstring(L, val.c_str());
	pushTable(L);
}

void LuaInterface::setFieldBool(lua_State* L, const char* index, bool val)
{
	lua_pushstring(L, index);
	lua_pushboolean(L, val);
	pushTable(L);
}

void LuaInterface::setFieldFloat(lua_State* L, const char* index, double val)
{
	lua_pushstring(L, index);
	lua_pushnumber(L, val);
	pushTable(L);
}

void LuaInterface::createTable(lua_State* L, const char* index)
{
	lua_pushstring(L, index);
	lua_newtable(L);
}

void LuaInterface::createTable(lua_State* L, const char* index, int32_t narr, int32_t nrec)
{
	lua_pushstring(L, index);
	lua_createtable(L, narr, nrec);
}

void LuaInterface::createTable(lua_State* L, int32_t index)
{
	lua_pushnumber(L, index);
	lua_newtable(L);
}

void LuaInterface::createTable(lua_State* L, int32_t index, int32_t narr, int32_t nrec)
{
	lua_pushnumber(L, index);
	lua_createtable(L, narr, nrec);
}

void LuaInterface::pushTable(lua_State* L)
{
	lua_settable(L, -3);
}

int64_t LuaInterface::getField(lua_State* L, const char* key)
{
	lua_pushstring(L, key);
	lua_gettable(L, -2); // get table[key]

	int64_t result = (int64_t)lua_tonumber(L, -1);
	lua_pop(L, 1); // remove number and key
	return result;
}

uint64_t LuaInterface::getFieldUnsigned(lua_State* L, const char* key)
{
	lua_pushstring(L, key);
	lua_gettable(L, -2); // get table[key]

	uint64_t result = (uint64_t)lua_tonumber(L, -1);
	lua_pop(L, 1); // remove number and key
	return result;
}

bool LuaInterface::getFieldBool(lua_State* L, const char* key)
{
	lua_pushstring(L, key);
	lua_gettable(L, -2); // get table[key]

	bool result = (lua_toboolean(L, -1) != 0);
	lua_pop(L, 1); // remove number and key
	return result;
}

std::string LuaInterface::getFieldString(lua_State* L, const char* key)
{
	lua_pushstring(L, key);
	lua_gettable(L, -2); // get table[key]

	std::string result = lua_tostring(L, -1);
	lua_pop(L, 1); // remove number and key
	return result;
}

std::string LuaInterface::getGlobalString(lua_State* L, const std::string& _identifier, const std::string& _default/* = ""*/)
{
	lua_getglobal(L, _identifier.c_str());
	if(!lua_isstring(L, -1))
	{
		lua_pop(L, 1);
		return _default;
	}

	int32_t len = (int32_t)lua_strlen(L, -1);
	std::string ret(lua_tostring(L, -1), len);

	lua_pop(L, 1);
	return ret;
}

bool LuaInterface::getGlobalBool(lua_State* L, const std::string& _identifier, bool _default/* = false*/)
{
	lua_getglobal(L, _identifier.c_str());
	if(!lua_isboolean(L, -1))
	{
		lua_pop(L, 1);
		return booleanString(LuaInterface::getGlobalString(L, _identifier, _default ? "yes" : "no"));
	}

	bool val = (lua_toboolean(L, -1) != 0);
	lua_pop(L, 1);
	return val;
}

int64_t LuaInterface::getGlobalNumber(lua_State* L, const std::string& _identifier, const int64_t _default/* = 0*/)
{
	return (int64_t)LuaInterface::getGlobalDouble(L, _identifier, _default);
}

double LuaInterface::getGlobalDouble(lua_State* L, const std::string& _identifier, const double _default/* = 0*/)
{
	lua_getglobal(L, _identifier.c_str());
	if(!lua_isnumber(L, -1))
	{
		lua_pop(L, 1);
		return _default;
	}

	double val = lua_tonumber(L, -1);
	lua_pop(L, 1);
	return val;
}

void LuaInterface::getValue(const std::string& key, lua_State* L, lua_State* _L)
{
	lua_getglobal(L, key.c_str());
	moveValue(L, _L);
}

void LuaInterface::moveValue(lua_State* from, lua_State* to)
{
	switch(lua_type(from, -1))
	{
		case LUA_TNIL:
			lua_pushnil(to);
			break;
		case LUA_TBOOLEAN:
			lua_pushboolean(to, lua_toboolean(from, -1));
			break;
		case LUA_TNUMBER:
			lua_pushnumber(to, lua_tonumber(from, -1));
			break;
		case LUA_TSTRING:
		{
			size_t len;
			const char* str = lua_tolstring(from, -1, &len);

			lua_pushlstring(to, str, len);
			break;
		}
		case LUA_TTABLE:
		{
			lua_newtable(to);
			lua_pushnil(from); // First key
			while(lua_next(from, -2))
			{
				// Move value to the other state
				moveValue(from, to); // Value is popped, key is left
				// Move key to the other state
				lua_pushvalue(from, -1); // Make a copy of the key to use for the next iteration
				moveValue(from, to); // Key is in other state.
				// We still have the key in the 'from' state ontop of the stack

				lua_insert(to, -2); // Move key above value
				pushTable(to); // Set the key
			}

			break;
		}
		default:
			break;
	}

	lua_pop(from, 1); // Pop the value we just read
}

void LuaInterface::registerFunctions()
{
	//example(...)
	//lua_register(L, "name", C_function);

	//getCreatureHealth(cid)
	lua_register(m_luaState, "getCreatureHealth", LuaInterface::luaGetCreatureHealth);

	//getCreatureMaxHealth(cid[, ignoreModifiers = false])
	lua_register(m_luaState, "getCreatureMaxHealth", LuaInterface::luaGetCreatureMaxHealth);

	//getCreatureMana(cid)
	lua_register(m_luaState, "getCreatureMana", LuaInterface::luaGetCreatureMana);

	//getCreatureMaxMana(cid[, ignoreModifiers = false])
	lua_register(m_luaState, "getCreatureMaxMana", LuaInterface::luaGetCreatureMaxMana);

	//getCreatureHideHealth(cid)
	lua_register(m_luaState, "getCreatureHideHealth", LuaInterface::luaGetCreatureHideHealth);

	//doCreatureSetHideHealth(cid, hide)
	lua_register(m_luaState, "doCreatureSetHideHealth", LuaInterface::luaDoCreatureSetHideHealth);

	//getCreatureSpeakType(cid)
	lua_register(m_luaState, "getCreatureSpeakType", LuaInterface::luaGetCreatureSpeakType);

	//doCreatureSetSpeakType(cid, type)
	lua_register(m_luaState, "doCreatureSetSpeakType", LuaInterface::luaDoCreatureSetSpeakType);

	//getCreatureLookDirection(cid)
	lua_register(m_luaState, "getCreatureLookDirection", LuaInterface::luaGetCreatureLookDirection);

	//getPlayerLevel(cid)
	lua_register(m_luaState, "getPlayerLevel", LuaInterface::luaGetPlayerLevel);

	//getPlayerExperience(cid)
	lua_register(m_luaState, "getPlayerExperience", LuaInterface::luaGetPlayerExperience);

	//getPlayerMagLevel(cid[, ignoreModifiers = false])
	lua_register(m_luaState, "getPlayerMagLevel", LuaInterface::luaGetPlayerMagLevel);

	//getPlayerSpentMana(cid)
	lua_register(m_luaState, "getPlayerSpentMana", LuaInterface::luaGetPlayerSpentMana);

	//getPlayerFood(cid)
	lua_register(m_luaState, "getPlayerFood", LuaInterface::luaGetPlayerFood);

	//getPlayerAccess(cid)
	lua_register(m_luaState, "getPlayerAccess", LuaInterface::luaGetPlayerAccess);

	//getPlayerGhostAccess(cid)
	lua_register(m_luaState, "getPlayerGhostAccess", LuaInterface::luaGetPlayerGhostAccess);

	//getPlayerSkillLevel(cid, skill[, ignoreModifiers = false])
	lua_register(m_luaState, "getPlayerSkillLevel", LuaInterface::luaGetPlayerSkillLevel);

	//getPlayerSkillTries(cid, skill)
	lua_register(m_luaState, "getPlayerSkillTries", LuaInterface::luaGetPlayerSkillTries);

	//doPlayerSetOfflineTrainingSkill(cid, skill)
	lua_register(m_luaState, "doPlayerSetOfflineTrainingSkill", LuaInterface::luaDoPlayerSetOfflineTrainingSkill);

	//getPlayerTown(cid)
	lua_register(m_luaState, "getPlayerTown", LuaInterface::luaGetPlayerTown);

	//getPlayerVocation(cid)
	lua_register(m_luaState, "getPlayerVocation", LuaInterface::luaGetPlayerVocation);

	//getPlayerIp(cid)
	lua_register(m_luaState, "getPlayerIp", LuaInterface::luaGetPlayerIp);

	//getPlayerRequiredMana(cid, magicLevel)
	lua_register(m_luaState, "getPlayerRequiredMana", LuaInterface::luaGetPlayerRequiredMana);

	//getPlayerRequiredSkillTries(cid, skillId, skillLevel)
	lua_register(m_luaState, "getPlayerRequiredSkillTries", LuaInterface::luaGetPlayerRequiredSkillTries);

	//getPlayerItemCount(cid, itemid[, subType = -1])
	lua_register(m_luaState, "getPlayerItemCount", LuaInterface::luaGetPlayerItemCount);

	//getPlayerMoney(cid)
	lua_register(m_luaState, "getPlayerMoney", LuaInterface::luaGetPlayerMoney);

	//getPlayerSoul(cid[, ignoreModifiers = false])
	lua_register(m_luaState, "getPlayerSoul", LuaInterface::luaGetPlayerSoul);

	//getPlayerFreeCap(cid)
	lua_register(m_luaState, "getPlayerFreeCap", LuaInterface::luaGetPlayerFreeCap);

	//getPlayerLight(cid)
	lua_register(m_luaState, "getPlayerLight", LuaInterface::luaGetPlayerLight);

	//getPlayerSlotItem(cid, slot)
	lua_register(m_luaState, "getPlayerSlotItem", LuaInterface::luaGetPlayerSlotItem);

	//getPlayerWeapon(cid[, ignoreAmmo = false])
	lua_register(m_luaState, "getPlayerWeapon", LuaInterface::luaGetPlayerWeapon);

	//getPlayerItemById(cid, deepSearch, itemId[, subType = -1])
	lua_register(m_luaState, "getPlayerItemById", LuaInterface::luaGetPlayerItemById);

	//getPlayerDepotItems(cid, depotid)
	lua_register(m_luaState, "getPlayerDepotItems", LuaInterface::luaGetPlayerDepotItems);

	//getPlayerGuildId(cid)
	lua_register(m_luaState, "getPlayerGuildId", LuaInterface::luaGetPlayerGuildId);

	//getPlayerGuildName(cid)
	lua_register(m_luaState, "getPlayerGuildName", LuaInterface::luaGetPlayerGuildName);

	//getPlayerGuildRankId(cid)
	lua_register(m_luaState, "getPlayerGuildRankId", LuaInterface::luaGetPlayerGuildRankId);

	//getPlayerGuildRank(cid)
	lua_register(m_luaState, "getPlayerGuildRank", LuaInterface::luaGetPlayerGuildRank);

	//getPlayerGuildNick(cid)
	lua_register(m_luaState, "getPlayerGuildNick", LuaInterface::luaGetPlayerGuildNick);

	//getPlayerGuildLevel(cid)
	lua_register(m_luaState, "getPlayerGuildLevel", LuaInterface::luaGetPlayerGuildLevel);

	//getPlayerGUID(cid)
	lua_register(m_luaState, "getPlayerGUID", LuaInterface::luaGetPlayerGUID);

	//getPlayerNameDescription(cid)
	lua_register(m_luaState, "getPlayerNameDescription", LuaInterface::luaGetPlayerNameDescription);

	//doPlayerSetNameDescription(cid, desc)
	lua_register(m_luaState, "doPlayerSetNameDescription", LuaInterface::luaDoPlayerSetNameDescription);

	//getPlayerSpecialDescription(cid)
	lua_register(m_luaState, "getPlayerSpecialDescription", LuaInterface::luaGetPlayerSpecialDescription);

	//doPlayerSetSpecialDescription(cid, desc)
	lua_register(m_luaState, "doPlayerSetSpecialDescription", LuaInterface::luaDoPlayerSetSpecialDescription);

	//getPlayerAccountId(cid)
	lua_register(m_luaState, "getPlayerAccountId", LuaInterface::luaGetPlayerAccountId);

	//getPlayerAccount(cid)
	lua_register(m_luaState, "getPlayerAccount", LuaInterface::luaGetPlayerAccount);

	//getPlayerFlagValue(cid, flag)
	lua_register(m_luaState, "getPlayerFlagValue", LuaInterface::luaGetPlayerFlagValue);

	//getPlayerCustomFlagValue(cid, flag)
	lua_register(m_luaState, "getPlayerCustomFlagValue", LuaInterface::luaGetPlayerCustomFlagValue);

	//getPlayerPromotionLevel(cid)
	lua_register(m_luaState, "getPlayerPromotionLevel", LuaInterface::luaGetPlayerPromotionLevel);

	//doPlayerSetPromotionLevel(cid, level)
	lua_register(m_luaState, "doPlayerSetPromotionLevel", LuaInterface::luaDoPlayerSetPromotionLevel);

	//getPlayerGroupId(cid)
	lua_register(m_luaState, "getPlayerGroupId", LuaInterface::luaGetPlayerGroupId);

	//doPlayerSetGroupId(cid, newGroupId)
	lua_register(m_luaState, "doPlayerSetGroupId", LuaInterface::luaDoPlayerSetGroupId);

	//doPlayerSendOutfitWindow(cid)
	lua_register(m_luaState, "doPlayerSendOutfitWindow", LuaInterface::luaDoPlayerSendOutfitWindow);

	//doPlayerLearnInstantSpell(cid, name)
	lua_register(m_luaState, "doPlayerLearnInstantSpell", LuaInterface::luaDoPlayerLearnInstantSpell);

	//doPlayerUnlearnInstantSpell(cid, name)
	lua_register(m_luaState, "doPlayerUnlearnInstantSpell", LuaInterface::luaDoPlayerUnlearnInstantSpell);

	//getPlayerLearnedInstantSpell(cid, name)
	lua_register(m_luaState, "getPlayerLearnedInstantSpell", LuaInterface::luaGetPlayerLearnedInstantSpell);

	//getPlayerInstantSpellCount(cid)
	lua_register(m_luaState, "getPlayerInstantSpellCount", LuaInterface::luaGetPlayerInstantSpellCount);

	//getPlayerInstantSpellInfo(cid, index)
	lua_register(m_luaState, "getPlayerInstantSpellInfo", LuaInterface::luaGetPlayerInstantSpellInfo);

	//getInstantSpellInfo(cid, name)
	lua_register(m_luaState, "getInstantSpellInfo", LuaInterface::luaGetInstantSpellInfo);

	//doCreatureCastSpell(uid, spell)
	lua_register(m_luaState, "doCreatureCastSpell", LuaInterface::luaDoCreatureCastSpell);

	//getCreatureStorageList(cid)
	lua_register(m_luaState, "getCreatureStorageList", LuaInterface::luaGetCreatureStorageList);

	//getCreatureStorage(uid, key)
	lua_register(m_luaState, "getCreatureStorage", LuaInterface::luaGetCreatureStorage);

	//doCreatureSetStorage(uid, key, value)
	lua_register(m_luaState, "doCreatureSetStorage", LuaInterface::luaDoCreatureSetStorage);

	//getPlayerSpectators(cid)
	lua_register(m_luaState, "getPlayerSpectators", LuaInterface::luaGetPlayerSpectators);

	//doPlayerSetSpectators(cid, data)
	lua_register(m_luaState, "doPlayerSetSpectators", LuaInterface::luaDoPlayerSetSpectators);

	//getStorageList()
	lua_register(m_luaState, "getStorageList", LuaInterface::luaGetStorageList);

	//getStorage(key)
	lua_register(m_luaState, "getStorage", LuaInterface::luaGetStorage);

	//doSetStorage(key, value)
	lua_register(m_luaState, "doSetStorage", LuaInterface::luaDoSetStorage);

	//getChannelUsers(channelId)
	lua_register(m_luaState, "getChannelUsers", LuaInterface::luaGetChannelUsers);

	//getPlayersOnline()
	lua_register(m_luaState, "getPlayersOnline", LuaInterface::luaGetPlayersOnline);

	//getTileInfo(pos)
	lua_register(m_luaState, "getTileInfo", LuaInterface::luaGetTileInfo);

	//getThingFromPosition(pos)
	lua_register(m_luaState, "getThingFromPosition", LuaInterface::luaGetThingFromPosition);

	//getThing(uid[, recursive = RECURSE_FIRST])
	lua_register(m_luaState, "getThing", LuaInterface::luaGetThing);

	//doTileQueryAdd(uid, pos[, flags])
	lua_register(m_luaState, "doTileQueryAdd", LuaInterface::luaDoTileQueryAdd);

	//doItemRaidUnref(uid)
	lua_register(m_luaState, "doItemRaidUnref", LuaInterface::luaDoItemRaidUnref);

	//getThingPosition(uid)
	lua_register(m_luaState, "getThingPosition", LuaInterface::luaGetThingPosition);

	//getTileItemById(pos, itemId[, subType = -1])
	lua_register(m_luaState, "getTileItemById", LuaInterface::luaGetTileItemById);

	//getTileItemByType(pos, type)
	lua_register(m_luaState, "getTileItemByType", LuaInterface::luaGetTileItemByType);

	//getTileThingByPos(pos)
	lua_register(m_luaState, "getTileThingByPos", LuaInterface::luaGetTileThingByPos);

	//getTopCreature(pos)
	lua_register(m_luaState, "getTopCreature", LuaInterface::luaGetTopCreature);

	//doRemoveItem(uid[, count = -1])
	lua_register(m_luaState, "doRemoveItem", LuaInterface::luaDoRemoveItem);

	//doPlayerFeed(cid, food)
	lua_register(m_luaState, "doPlayerFeed", LuaInterface::luaDoPlayerFeed);

	//doPlayerSendCancel(cid, text)
	lua_register(m_luaState, "doPlayerSendCancel", LuaInterface::luaDoPlayerSendCancel);

	//doPlayerSendDefaultCancel(cid, ReturnValue)
	lua_register(m_luaState, "doPlayerSendDefaultCancel", LuaInterface::luaDoSendDefaultCancel);

	//getSearchString(fromPosition, toPosition[, fromIsCreature = false[, toIsCreature = false]])
	lua_register(m_luaState, "getSearchString", LuaInterface::luaGetSearchString);

	//getClosestFreeTile(cid, targetpos[, extended = false[, ignoreHouse = true]])
	lua_register(m_luaState, "getClosestFreeTile", LuaInterface::luaGetClosestFreeTile);

	//doTeleportThing(cid, destination[, pushmove = true[, fullTeleport = true]])
	lua_register(m_luaState, "doTeleportThing", LuaInterface::luaDoTeleportThing);

	//doItemSetDestination(uid, destination)
	lua_register(m_luaState, "doItemSetDestination", LuaInterface::luaDoItemSetDestination);

	//doTransformItem(uid, newId[, count/subType])
	lua_register(m_luaState, "doTransformItem", LuaInterface::luaDoTransformItem);

	//doCreatureSay(uid, text[, type = SPEAK_SAY[, ghost = false[, cid = 0[, pos]]]])
	lua_register(m_luaState, "doCreatureSay", LuaInterface::luaDoCreatureSay);

	//doSendCreatureSquare(cid, color[, player])
	lua_register(m_luaState, "doSendCreatureSquare", LuaInterface::luaDoSendCreatureSquare);

	//doSendMagicEffect(pos, type[, player])
	lua_register(m_luaState, "doSendMagicEffect", LuaInterface::luaDoSendMagicEffect);

	//doSendDistanceShoot(fromPos, toPos, type[, player])
	lua_register(m_luaState, "doSendDistanceShoot", LuaInterface::luaDoSendDistanceShoot);

	//doSendAnimatedText(pos, text, color[, player])
	lua_register(m_luaState, "doSendAnimatedText", LuaInterface::luaDoSendAnimatedText);

	//doPlayerAddSkillTry(cid, skillid, n[, useMultiplier = true])
	lua_register(m_luaState, "doPlayerAddSkillTry", LuaInterface::luaDoPlayerAddSkillTry);

	//doCreatureAddHealth(cid, health[, hitEffect[, hitColor[, force]]])
	lua_register(m_luaState, "doCreatureAddHealth", LuaInterface::luaDoCreatureAddHealth);

	//doCreatureAddMana(cid, mana)
	lua_register(m_luaState, "doCreatureAddMana", LuaInterface::luaDoCreatureAddMana);

	//setCreatureMaxHealth(cid, health)
	lua_register(m_luaState, "setCreatureMaxHealth", LuaInterface::luaSetCreatureMaxHealth);

	//setCreatureMaxMana(cid, mana)
	lua_register(m_luaState, "setCreatureMaxMana", LuaInterface::luaSetCreatureMaxMana);

	//doPlayerSetMaxCapacity(cid, cap)
	lua_register(m_luaState, "doPlayerSetMaxCapacity", LuaInterface::luaDoPlayerSetMaxCapacity);

	//doPlayerAddSpentMana(cid, amount[, useMultiplier = true])
	lua_register(m_luaState, "doPlayerAddSpentMana", LuaInterface::luaDoPlayerAddSpentMana);

	//doPlayerAddSoul(cid, amount)
	lua_register(m_luaState, "doPlayerAddSoul", LuaInterface::luaDoPlayerAddSoul);

	//doPlayerAddItem(cid, itemid[, count/subtype = 1[, canDropOnMap = true[, slot = 0]]])
	//doPlayerAddItem(cid, itemid[, count = 1[, canDropOnMap = true[, subtype = 1[, slot = 0]]]])
	//Returns uid of the created item
	lua_register(m_luaState, "doPlayerAddItem", LuaInterface::luaDoPlayerAddItem);

	//doPlayerAddItemEx(cid, uid[, canDropOnMap = false[, slot = 0]])
	lua_register(m_luaState, "doPlayerAddItemEx", LuaInterface::luaDoPlayerAddItemEx);

	//doPlayerSendTextMessage(cid, MessageClasses, message[, value[, color[, position]]])
	lua_register(m_luaState, "doPlayerSendTextMessage", LuaInterface::luaDoPlayerSendTextMessage);

	//doPlayerSendChannelMessage(cid, author, message, MessageClasses, channel)
	lua_register(m_luaState, "doPlayerSendChannelMessage", LuaInterface::luaDoPlayerSendChannelMessage);

	//doCreatureChannelSay(cid, targetId, MessageClasses, message, channel[, time])
	lua_register(m_luaState, "doCreatureChannelSay", LuaInterface::luaDoCreatureChannelSay);

	//doPlayerOpenChannel(cid, channelId)
	lua_register(m_luaState, "doPlayerOpenChannel", LuaInterface::luaDoPlayerOpenChannel);

	//doPlayerSendChannels(cid[, list])
	lua_register(m_luaState, "doPlayerSendChannels", LuaInterface::luaDoPlayerSendChannels);

	//doPlayerAddMoney(cid, money)
	lua_register(m_luaState, "doPlayerAddMoney", LuaInterface::luaDoPlayerAddMoney);

	//doPlayerRemoveMoney(cid, money)
	lua_register(m_luaState, "doPlayerRemoveMoney", LuaInterface::luaDoPlayerRemoveMoney);

	//doPlayerTransferMoneyTo(cid, target, money)
	lua_register(m_luaState, "doPlayerTransferMoneyTo", LuaInterface::luaDoPlayerTransferMoneyTo);

	//doShowTextDialog(cid, itemid[, (text/canWrite)[, (canWrite/length)[, length]]])
	lua_register(m_luaState, "doShowTextDialog", LuaInterface::luaDoShowTextDialog);

	//doDecayItem(uid)
	lua_register(m_luaState, "doDecayItem", LuaInterface::luaDoDecayItem);

	//doCreateItem(itemid[, type/count], pos)
	//Returns uid of the created item, only works on tiles.
	lua_register(m_luaState, "doCreateItem", LuaInterface::luaDoCreateItem);

	//doCreateItemEx(itemid[, count/subType = -1])
	lua_register(m_luaState, "doCreateItemEx", LuaInterface::luaDoCreateItemEx);

	//doTileAddItemEx(pos, uid)
	lua_register(m_luaState, "doTileAddItemEx", LuaInterface::luaDoTileAddItemEx);

	//doAddContainerItemEx(uid, virtuid)
	lua_register(m_luaState, "doAddContainerItemEx", LuaInterface::luaDoAddContainerItemEx);

	//doRelocate(pos, posTo[, creatures = true[, unmovable = true]])
	//Moves all movable objects from pos to posTo
	lua_register(m_luaState, "doRelocate", LuaInterface::luaDoRelocate);

	//doCleanTile(pos[, forceMapLoaded = false])
	lua_register(m_luaState, "doCleanTile", LuaInterface::luaDoCleanTile);

	//doCreateTeleport(itemId, destination, position)
	lua_register(m_luaState, "doCreateTeleport", LuaInterface::luaDoCreateTeleport);

	//doCreateMonster(name, pos[, extend = false[, force = false]])
	lua_register(m_luaState, "doCreateMonster", LuaInterface::luaDoCreateMonster);

	//doCreateNpc(name, pos)
	lua_register(m_luaState, "doCreateNpc", LuaInterface::luaDoCreateNpc);

	//doSummonMonster(cid, name)
	lua_register(m_luaState, "doSummonMonster", LuaInterface::luaDoSummonMonster);

	//doConvinceCreature(cid, target)
	lua_register(m_luaState, "doConvinceCreature", LuaInterface::luaDoConvinceCreature);

	//getMonsterTargetList(cid)
	lua_register(m_luaState, "getMonsterTargetList", LuaInterface::luaGetMonsterTargetList);

	//getMonsterFriendList(cid)
	lua_register(m_luaState, "getMonsterFriendList", LuaInterface::luaGetMonsterFriendList);

	//doMonsterSetTarget(cid, target)
	lua_register(m_luaState, "doMonsterSetTarget", LuaInterface::luaDoMonsterSetTarget);

	//doMonsterChangeTarget(cid)
	lua_register(m_luaState, "doMonsterChangeTarget", LuaInterface::luaDoMonsterChangeTarget);

	//getMonsterInfo(name)
	lua_register(m_luaState, "getMonsterInfo", LuaInterface::luaGetMonsterInfo);

	//doAddCondition(cid, condition)
	lua_register(m_luaState, "doAddCondition", LuaInterface::luaDoAddCondition);

	//doRemoveCondition(cid, type[, subId = 0])
	lua_register(m_luaState, "doRemoveCondition", LuaInterface::luaDoRemoveCondition);

	//doRemoveConditions(cid[, onlyPersistent])
	lua_register(m_luaState, "doRemoveConditions", LuaInterface::luaDoRemoveConditions);

	//doRemoveCreature(cid[, forceLogout = true])
	lua_register(m_luaState, "doRemoveCreature", LuaInterface::luaDoRemoveCreature);

	//doMoveCreature(cid, direction[, flag = FLAG_NOLIMIT])
	lua_register(m_luaState, "doMoveCreature", LuaInterface::luaDoMoveCreature);

	//doSteerCreature(cid, position[, maxNodes])
	lua_register(m_luaState, "doSteerCreature", LuaInterface::luaDoSteerCreature);

	//doPlayerSetPzLocked(cid, locked)
	lua_register(m_luaState, "doPlayerSetPzLocked", LuaInterface::luaDoPlayerSetPzLocked);

	//doPlayerSetTown(cid, townid)
	lua_register(m_luaState, "doPlayerSetTown", LuaInterface::luaDoPlayerSetTown);

	//doPlayerSetVocation(cid,voc)
	lua_register(m_luaState, "doPlayerSetVocation", LuaInterface::luaDoPlayerSetVocation);

	//doPlayerRemoveItem(cid, itemid[, count[, subType = -1[, ignoreEquipped = false]]])
	lua_register(m_luaState, "doPlayerRemoveItem", LuaInterface::luaDoPlayerRemoveItem);

	//doPlayerAddExperience(cid, amount)
	lua_register(m_luaState, "doPlayerAddExperience", LuaInterface::luaDoPlayerAddExperience);

	//doPlayerSetGuildId(cid, id)
	lua_register(m_luaState, "doPlayerSetGuildId", LuaInterface::luaDoPlayerSetGuildId);

	//doPlayerSetGuildLevel(cid, level[, rank])
	lua_register(m_luaState, "doPlayerSetGuildLevel", LuaInterface::luaDoPlayerSetGuildLevel);

	//doPlayerSetGuildNick(cid, nick)
	lua_register(m_luaState, "doPlayerSetGuildNick", LuaInterface::luaDoPlayerSetGuildNick);

	//doPlayerAddOutfit(cid, looktype, addon)
	lua_register(m_luaState, "doPlayerAddOutfit", LuaInterface::luaDoPlayerAddOutfit);

	//doPlayerRemoveOutfit(cid, looktype[, addon = 0])
	lua_register(m_luaState, "doPlayerRemoveOutfit", LuaInterface::luaDoPlayerRemoveOutfit);

	//doPlayerAddOutfitId(cid, outfitId, addon)
	lua_register(m_luaState, "doPlayerAddOutfitId", LuaInterface::luaDoPlayerAddOutfitId);

	//doPlayerRemoveOutfitId(cid, outfitId[, addon = 0])
	lua_register(m_luaState, "doPlayerRemoveOutfitId", LuaInterface::luaDoPlayerRemoveOutfitId);

	//canPlayerWearOutfit(cid, looktype[, addon = 0])
	lua_register(m_luaState, "canPlayerWearOutfit", LuaInterface::luaCanPlayerWearOutfit);

	//canPlayerWearOutfitId(cid, outfitId[, addon = 0])
	lua_register(m_luaState, "canPlayerWearOutfitId", LuaInterface::luaCanPlayerWearOutfitId);

	//hasCreatureCondition(cid, condition[, subId = 0[, conditionId = (both)]])
	lua_register(m_luaState, "hasCreatureCondition", LuaInterface::luaHasCreatureCondition);

	//getCreatureConditionInfo(cid, condition[, subId = 0[, conditionId = CONDITIONID_COMBAT]])
	lua_register(m_luaState, "getCreatureConditionInfo", LuaInterface::luaGetCreatureConditionInfo);

	//doCreatureSetDropLoot(cid, doDrop)
	lua_register(m_luaState, "doCreatureSetDropLoot", LuaInterface::luaDoCreatureSetDropLoot);

	//getPlayerLossPercent(cid, lossType)
	lua_register(m_luaState, "getPlayerLossPercent", LuaInterface::luaGetPlayerLossPercent);

	//doPlayerSetLossPercent(cid, lossType, newPercent)
	lua_register(m_luaState, "doPlayerSetLossPercent", LuaInterface::luaDoPlayerSetLossPercent);

	//doPlayerSetLossSkill(cid, doLose)
	lua_register(m_luaState, "doPlayerSetLossSkill", LuaInterface::luaDoPlayerSetLossSkill);

	//getPlayerLossSkill(cid)
	lua_register(m_luaState, "getPlayerLossSkill", LuaInterface::luaGetPlayerLossSkill);

	//doPlayerSwitchSaving(cid)
	lua_register(m_luaState, "doPlayerSwitchSaving", LuaInterface::luaDoPlayerSwitchSaving);

	//doPlayerSave(cid[, shallow = false])
	lua_register(m_luaState, "doPlayerSave", LuaInterface::luaDoPlayerSave);

	//isPlayerPzLocked(cid)
	lua_register(m_luaState, "isPlayerPzLocked", LuaInterface::luaIsPlayerPzLocked);

	//isPlayerSaving(cid)
	lua_register(m_luaState, "isPlayerSaving", LuaInterface::luaIsPlayerSaving);

	//isPlayerProtected(cid)
	lua_register(m_luaState, "isPlayerProtected", LuaInterface::luaIsPlayerProtected);

	//isCreature(cid)
	lua_register(m_luaState, "isCreature", LuaInterface::luaIsCreature);

	//isMovable(uid)
	lua_register(m_luaState, "isMovable", LuaInterface::luaIsMovable);

	//getCreatureByName(name)
	lua_register(m_luaState, "getCreatureByName", LuaInterface::luaGetCreatureByName);

	//getPlayerByGUID(guid)
	lua_register(m_luaState, "getPlayerByGUID", LuaInterface::luaGetPlayerByGUID);

	//getPlayerByNameWildcard(name~[, ret = false])
	lua_register(m_luaState, "getPlayerByNameWildcard", LuaInterface::luaGetPlayerByNameWildcard);

	//getPlayerGUIDByName(name[, multiworld = false])
	lua_register(m_luaState, "getPlayerGUIDByName", LuaInterface::luaGetPlayerGUIDByName);

	//getPlayerNameByGUID(guid[, multiworld = false])
	lua_register(m_luaState, "getPlayerNameByGUID", LuaInterface::luaGetPlayerNameByGUID);

	//doPlayerChangeName(guid, oldName, newName)
	lua_register(m_luaState, "doPlayerChangeName", LuaInterface::luaDoPlayerChangeName);

	//registerCreatureEvent(uid, name)
	lua_register(m_luaState, "registerCreatureEvent", LuaInterface::luaRegisterCreatureEvent);

	//unregisterCreatureEvent(uid, name)
	lua_register(m_luaState, "unregisterCreatureEvent", LuaInterface::luaUnregisterCreatureEvent);

	//unregisterCreatureEventType(uid, type)
	lua_register(m_luaState, "unregisterCreatureEventType", LuaInterface::luaUnregisterCreatureEventType);

	//getContainerSize(uid)
	lua_register(m_luaState, "getContainerSize", LuaInterface::luaGetContainerSize);

	//getContainerCap(uid)
	lua_register(m_luaState, "getContainerCap", LuaInterface::luaGetContainerCap);

	//getContainerItem(uid, slot)
	lua_register(m_luaState, "getContainerItem", LuaInterface::luaGetContainerItem);

	//doAddContainerItem(uid, itemid[, count/subType = 1])
	lua_register(m_luaState, "doAddContainerItem", LuaInterface::luaDoAddContainerItem);

	//getHouseInfo(houseId[, full = true])
	lua_register(m_luaState, "getHouseInfo", LuaInterface::luaGetHouseInfo);

	//getHouseAccessList(houseid, listId)
	lua_register(m_luaState, "getHouseAccessList", LuaInterface::luaGetHouseAccessList);

	//getHouseByPlayerGUID(playerGUID)
	lua_register(m_luaState, "getHouseByPlayerGUID", LuaInterface::luaGetHouseByPlayerGUID);

	//getHouseFromPosition(pos)
	lua_register(m_luaState, "getHouseFromPosition", LuaInterface::luaGetHouseFromPosition);

	//setHouseAccessList(houseid, listid, listtext)
	lua_register(m_luaState, "setHouseAccessList", LuaInterface::luaSetHouseAccessList);

	//setHouseOwner(houseId, owner[, clean = true])
	lua_register(m_luaState, "setHouseOwner", LuaInterface::luaSetHouseOwner);

	//getWorldType()
	lua_register(m_luaState, "getWorldType", LuaInterface::luaGetWorldType);

	//setWorldType(type)
	lua_register(m_luaState, "setWorldType", LuaInterface::luaSetWorldType);

	//getWorldTime()
	lua_register(m_luaState, "getWorldTime", LuaInterface::luaGetWorldTime);

	//getWorldLight()
	lua_register(m_luaState, "getWorldLight", LuaInterface::luaGetWorldLight);

	//getWorldCreatures(type)
	//0 players, 1 monsters, 2 npcs, 3 all
	lua_register(m_luaState, "getWorldCreatures", LuaInterface::luaGetWorldCreatures);

	//getWorldUpTime()
	lua_register(m_luaState, "getWorldUpTime", LuaInterface::luaGetWorldUpTime);

	//getGuildId(guildName)
	lua_register(m_luaState, "getGuildId", LuaInterface::luaGetGuildId);

	//getGuildMotd(guildId)
	lua_register(m_luaState, "getGuildMotd", LuaInterface::luaGetGuildMotd);

	//getPlayerSex(cid[, full = false])
	lua_register(m_luaState, "getPlayerSex", LuaInterface::luaGetPlayerSex);

	//doPlayerSetSex(cid, newSex)
	lua_register(m_luaState, "doPlayerSetSex", LuaInterface::luaDoPlayerSetSex);

	//createCombatArea({area}[, {extArea}])
	lua_register(m_luaState, "createCombatArea", LuaInterface::luaCreateCombatArea);

	//createConditionObject(type[, ticks = 0[, buff = false[, subId = 0[, conditionId = CONDITIONID_COMBAT]]]])
	lua_register(m_luaState, "createConditionObject", LuaInterface::luaCreateConditionObject);

	//setCombatArea(combat, area)
	lua_register(m_luaState, "setCombatArea", LuaInterface::luaSetCombatArea);

	//setCombatCondition(combat, condition)
	lua_register(m_luaState, "setCombatCondition", LuaInterface::luaSetCombatCondition);

	//setCombatParam(combat, key, value)
	lua_register(m_luaState, "setCombatParam", LuaInterface::luaSetCombatParam);

	//setConditionParam(condition, key, value)
	lua_register(m_luaState, "setConditionParam", LuaInterface::luaSetConditionParam);

	//addDamageCondition(condition, rounds, time, value)
	lua_register(m_luaState, "addDamageCondition", LuaInterface::luaAddDamageCondition);

	//addOutfitCondition(condition, outfit)
	lua_register(m_luaState, "addOutfitCondition", LuaInterface::luaAddOutfitCondition);

	//setCombatCallBack(combat, key, function_name)
	lua_register(m_luaState, "setCombatCallback", LuaInterface::luaSetCombatCallBack);

	//setCombatFormula(combat, type, mina, minb, maxa, maxb[, minl, maxl[, minm, maxm[, minc[, maxc]]]])
	lua_register(m_luaState, "setCombatFormula", LuaInterface::luaSetCombatFormula);

	//setConditionFormula(combat, mina, minb, maxa, maxb)
	lua_register(m_luaState, "setConditionFormula", LuaInterface::luaSetConditionFormula);

	//doCombat(cid, combat, param)
	lua_register(m_luaState, "doCombat", LuaInterface::luaDoCombat);

	//createCombatObject()
	lua_register(m_luaState, "createCombatObject", LuaInterface::luaCreateCombatObject);

	//doCombatAreaHealth(cid, type, pos, area, min, max, effect)
	lua_register(m_luaState, "doCombatAreaHealth", LuaInterface::luaDoCombatAreaHealth);

	//doTargetCombatHealth(cid, target, type, min, max, effect)
	lua_register(m_luaState, "doTargetCombatHealth", LuaInterface::luaDoTargetCombatHealth);

	//doCombatAreaMana(cid, pos, area, min, max, effect)
	lua_register(m_luaState, "doCombatAreaMana", LuaInterface::luaDoCombatAreaMana);

	//doTargetCombatMana(cid, target, min, max, effect)
	lua_register(m_luaState, "doTargetCombatMana", LuaInterface::luaDoTargetCombatMana);

	//doCombatAreaCondition(cid, pos, area, condition, effect)
	lua_register(m_luaState, "doCombatAreaCondition", LuaInterface::luaDoCombatAreaCondition);

	//doTargetCombatCondition(cid, target, condition, effect)
	lua_register(m_luaState, "doTargetCombatCondition", LuaInterface::luaDoTargetCombatCondition);

	//doCombatAreaDispel(cid, pos, area, type, effect)
	lua_register(m_luaState, "doCombatAreaDispel", LuaInterface::luaDoCombatAreaDispel);

	//doTargetCombatDispel(cid, target, type, effect)
	lua_register(m_luaState, "doTargetCombatDispel", LuaInterface::luaDoTargetCombatDispel);

	//doChallengeCreature(cid, target)
	lua_register(m_luaState, "doChallengeCreature", LuaInterface::luaDoChallengeCreature);

	//numberToVariant(number)
	lua_register(m_luaState, "numberToVariant", LuaInterface::luaNumberToVariant);

	//stringToVariant(string)
	lua_register(m_luaState, "stringToVariant", LuaInterface::luaStringToVariant);

	//positionToVariant(pos)
	lua_register(m_luaState, "positionToVariant", LuaInterface::luaPositionToVariant);

	//targetPositionToVariant(pos)
	lua_register(m_luaState, "targetPositionToVariant", LuaInterface::luaTargetPositionToVariant);

	//variantToNumber(var)
	lua_register(m_luaState, "variantToNumber", LuaInterface::luaVariantToNumber);

	//variantToString(var)
	lua_register(m_luaState, "variantToString", LuaInterface::luaVariantToString);

	//variantToPosition(var)
	lua_register(m_luaState, "variantToPosition", LuaInterface::luaVariantToPosition);

	//doChangeSpeed(cid, delta)
	lua_register(m_luaState, "doChangeSpeed", LuaInterface::luaDoChangeSpeed);

	//doCreatureChangeOutfit(cid, outfit)
	lua_register(m_luaState, "doCreatureChangeOutfit", LuaInterface::luaDoCreatureChangeOutfit);

	//doSetMonsterOutfit(cid, name[, time = -1])
	lua_register(m_luaState, "doSetMonsterOutfit", LuaInterface::luaSetMonsterOutfit);

	//doSetItemOutfit(cid, item[, time = -1])
	lua_register(m_luaState, "doSetItemOutfit", LuaInterface::luaSetItemOutfit);

	//doSetCreatureOutfit(cid, outfit[, time = -1])
	lua_register(m_luaState, "doSetCreatureOutfit", LuaInterface::luaSetCreatureOutfit);

	//getCreatureOutfit(cid)
	lua_register(m_luaState, "getCreatureOutfit", LuaInterface::luaGetCreatureOutfit);

	//getCreatureLastPosition(cid)
	lua_register(m_luaState, "getCreatureLastPosition", LuaInterface::luaGetCreatureLastPosition);

	//getCreatureName(cid)
	lua_register(m_luaState, "getCreatureName", LuaInterface::luaGetCreatureName);

	//getCreatureSpeed(cid)
	lua_register(m_luaState, "getCreatureSpeed", LuaInterface::luaGetCreatureSpeed);

	//getCreatureBaseSpeed(cid)
	lua_register(m_luaState, "getCreatureBaseSpeed", LuaInterface::luaGetCreatureBaseSpeed);

	//getCreatureTarget(cid)
	lua_register(m_luaState, "getCreatureTarget", LuaInterface::luaGetCreatureTarget);

	//isSightClear(fromPos, toPos, floorCheck)
	lua_register(m_luaState, "isSightClear", LuaInterface::luaIsSightClear);

	//addEvent(callback, delay, ...)
	lua_register(m_luaState, "addEvent", LuaInterface::luaAddEvent);

	//stopEvent(eventid)
	lua_register(m_luaState, "stopEvent", LuaInterface::luaStopEvent);

	//getPlayersByAccountId(accId)
	lua_register(m_luaState, "getPlayersByAccountId", LuaInterface::luaGetPlayersByAccountId);

	//getAccountIdByName(name)
	lua_register(m_luaState, "getAccountIdByName", LuaInterface::luaGetAccountIdByName);

	//getAccountByName(name)
	lua_register(m_luaState, "getAccountByName", LuaInterface::luaGetAccountByName);

	//getAccountIdByAccount(accName)
	lua_register(m_luaState, "getAccountIdByAccount", LuaInterface::luaGetAccountIdByAccount);

	//getAccountByAccountId(accId)
	lua_register(m_luaState, "getAccountByAccountId", LuaInterface::luaGetAccountByAccountId);

	//getAccountFlagValue(name/id)
	lua_register(m_luaState, "getAccountFlagValue", LuaInterface::luaGetAccountFlagValue);

	//getAccountCustomFlagValue(name/id)
	lua_register(m_luaState, "getAccountCustomFlagValue", LuaInterface::luaGetAccountCustomFlagValue);

	//getIpByName(name)
	lua_register(m_luaState, "getIpByName", LuaInterface::luaGetIpByName);

	//getPlayersByIp(ip[, mask = 0xFFFFFFFF])
	lua_register(m_luaState, "getPlayersByIp", LuaInterface::luaGetPlayersByIp);

	//doPlayerPopupFYI(cid, message)
	lua_register(m_luaState, "doPlayerPopupFYI", LuaInterface::luaDoPlayerPopupFYI);

	//doPlayerSendTutorial(cid, id)
	lua_register(m_luaState, "doPlayerSendTutorial", LuaInterface::luaDoPlayerSendTutorial);

	//doPlayerSendMailByName(name, item[, town[, actor]])
	lua_register(m_luaState, "doPlayerSendMailByName", LuaInterface::luaDoPlayerSendMailByName);

	//doPlayerAddMapMark(cid, pos, type[, description])
	lua_register(m_luaState, "doPlayerAddMapMark", LuaInterface::luaDoPlayerAddMapMark);

	//doPlayerAddPremiumDays(cid, days)
	lua_register(m_luaState, "doPlayerAddPremiumDays", LuaInterface::luaDoPlayerAddPremiumDays);

	//getPlayerPremiumDays(cid)
	lua_register(m_luaState, "getPlayerPremiumDays", LuaInterface::luaGetPlayerPremiumDays);

	//doCreatureSetLookDirection(cid, dir)
	lua_register(m_luaState, "doCreatureSetLookDirection", LuaInterface::luaDoCreatureSetLookDir);

	//getCreatureGuildEmblem(cid[, target])
	lua_register(m_luaState, "getCreatureGuildEmblem", LuaInterface::luaGetCreatureGuildEmblem);

	//doCreatureSetGuildEmblem(cid, emblem)
	lua_register(m_luaState, "doCreatureSetGuildEmblem", LuaInterface::luaDoCreatureSetGuildEmblem);

	//getCreaturePartyShield(cid[, target])
	lua_register(m_luaState, "getCreaturePartyShield", LuaInterface::luaGetCreaturePartyShield);

	//doCreatureSetPartyShield(cid, shield)
	lua_register(m_luaState, "doCreatureSetPartyShield", LuaInterface::luaDoCreatureSetPartyShield);

	//getCreatureSkullType(cid[, target])
	lua_register(m_luaState, "getCreatureSkullType", LuaInterface::luaGetCreatureSkullType);

	//doCreatureSetSkullType(cid, skull)
	lua_register(m_luaState, "doCreatureSetSkullType", LuaInterface::luaDoCreatureSetSkullType);

	//getPlayerSkullEnd(cid)
	lua_register(m_luaState, "getPlayerSkullEnd", LuaInterface::luaGetPlayerSkullEnd);

	//doPlayerSetSkullEnd(cid, time, type)
	lua_register(m_luaState, "doPlayerSetSkullEnd", LuaInterface::luaDoPlayerSetSkullEnd);

	//getPlayerBlessing(cid, blessing)
	lua_register(m_luaState, "getPlayerBlessing", LuaInterface::luaGetPlayerBlessing);

	//doPlayerAddBlessing(cid, blessing)
	lua_register(m_luaState, "doPlayerAddBlessing", LuaInterface::luaDoPlayerAddBlessing);

	//getPlayerPVPBlessing(cid)
	lua_register(m_luaState, "getPlayerPVPBlessing", LuaInterface::luaGetPlayerPVPBlessing);

	//doPlayerSetPVPBlessing(cid[, value])
	lua_register(m_luaState, "doPlayerSetPVPBlessing", LuaInterface::luaDoPlayerSetPVPBlessing);

	//getPlayerStamina(cid)
	lua_register(m_luaState, "getPlayerStamina", LuaInterface::luaGetPlayerStamina);

	//doPlayerSetStamina(cid, minutes)
	lua_register(m_luaState, "doPlayerSetStamina", LuaInterface::luaDoPlayerSetStamina);

	//getPlayerBalance(cid)
	lua_register(m_luaState, "getPlayerBalance", LuaInterface::luaGetPlayerBalance);

	//doPlayerSetBalance(cid, balance)
	lua_register(m_luaState, "doPlayerSetBalance", LuaInterface::luaDoPlayerSetBalance);

	//getCreatureNoMove(cid)
	lua_register(m_luaState, "getCreatureNoMove", LuaInterface::luaGetCreatureNoMove);

	//doCreatureSetNoMove(cid, block)
	lua_register(m_luaState, "doCreatureSetNoMove", LuaInterface::luaDoCreatureSetNoMove);

	//getPlayerIdleTime(cid)
	lua_register(m_luaState, "getPlayerIdleTime", LuaInterface::luaGetPlayerIdleTime);

	//doPlayerSetIdleTime(cid, amount)
	lua_register(m_luaState, "doPlayerSetIdleTime", LuaInterface::luaDoPlayerSetIdleTime);

	//getPlayerLastLoad(cid)
	lua_register(m_luaState, "getPlayerLastLoad", LuaInterface::luaGetPlayerLastLoad);

	//getPlayerLastLogin(cid)
	lua_register(m_luaState, "getPlayerLastLogin", LuaInterface::luaGetPlayerLastLogin);

	//getPlayerAccountManager(cid)
	lua_register(m_luaState, "getPlayerAccountManager", LuaInterface::luaGetPlayerAccountManager);

	//getPlayerTradeState(cid)
	lua_register(m_luaState, "getPlayerTradeState", LuaInterface::luaGetPlayerTradeState);

	//getPlayerOperatingSystem(cid)
	lua_register(m_luaState, "getPlayerOperatingSystem", LuaInterface::luaGetPlayerOperatingSystem);

	//getPlayerClientVersion(cid)
	lua_register(m_luaState, "getPlayerClientVersion", LuaInterface::luaGetPlayerClientVersion);

	//getPlayerModes(cid)
	lua_register(m_luaState, "getPlayerModes", LuaInterface::luaGetPlayerModes);

	//getPlayerRates(cid)
	lua_register(m_luaState, "getPlayerRates", LuaInterface::luaGetPlayerRates);

	//doPlayerSetRate(cid, type, value)
	lua_register(m_luaState, "doPlayerSetRate", LuaInterface::luaDoPlayerSetRate);

	//getPlayerPartner(cid)
	lua_register(m_luaState, "getPlayerPartner", LuaInterface::luaGetPlayerPartner);

	//doPlayerSetPartner(cid, guid)
	lua_register(m_luaState, "doPlayerSetPartner", LuaInterface::luaDoPlayerSetPartner);

	//doPlayerFollowCreature(cid, target)
	lua_register(m_luaState, "doPlayerFollowCreature", LuaInterface::luaDoPlayerFollowCreature);

	//getPlayerParty(cid)
	lua_register(m_luaState, "getPlayerParty", LuaInterface::luaGetPlayerParty);

	//doPlayerJoinParty(cid, lid)
	lua_register(m_luaState, "doPlayerJoinParty", LuaInterface::luaDoPlayerJoinParty);

	//doPlayerLeaveParty(cid[, forced = false])
	lua_register(m_luaState, "doPlayerLeaveParty", LuaInterface::luaDoPlayerLeaveParty);

	//getPartyMembers(lid)
	lua_register(m_luaState, "getPartyMembers", LuaInterface::luaGetPartyMembers);

	//getCreatureMaster(cid)
	lua_register(m_luaState, "getCreatureMaster", LuaInterface::luaGetCreatureMaster);

	//getCreatureSummons(cid)
	lua_register(m_luaState, "getCreatureSummons", LuaInterface::luaGetCreatureSummons);

	//getTownId(townName)
	lua_register(m_luaState, "getTownId", LuaInterface::luaGetTownId);

	//getTownName(townId)
	lua_register(m_luaState, "getTownName", LuaInterface::luaGetTownName);

	//getTownTemplePosition(townId)
	lua_register(m_luaState, "getTownTemplePosition", LuaInterface::luaGetTownTemplePosition);

	//getTownHouses(townId)
	lua_register(m_luaState, "getTownHouses", LuaInterface::luaGetTownHouses);

	//getSpectators(centerPos, rangex, rangey[, multifloor = false])
	lua_register(m_luaState, "getSpectators", LuaInterface::luaGetSpectators);

	//getVocationInfo(id)
	lua_register(m_luaState, "getVocationInfo", LuaInterface::luaGetVocationInfo);

	//getGroupInfo(id[, premium = false])
	lua_register(m_luaState, "getGroupInfo", LuaInterface::luaGetGroupInfo);

	//getVocationList()
	lua_register(m_luaState, "getVocationList", LuaInterface::luaGetVocationList);

	//getGroupList()
	lua_register(m_luaState, "getGroupList", LuaInterface::luaGetGroupList);

	//getChannelList()
	lua_register(m_luaState, "getChannelList", LuaInterface::luaGetChannelList);

	//getTownList()
	lua_register(m_luaState, "getTownList", LuaInterface::luaGetTownList);

	//getWaypointList()
	lua_register(m_luaState, "getWaypointList", LuaInterface::luaGetWaypointList);

	//getTalkActionList()
	lua_register(m_luaState, "getTalkActionList", LuaInterface::luaGetTalkActionList);

	//getExperienceStageList()
	lua_register(m_luaState, "getExperienceStageList", LuaInterface::luaGetExperienceStageList);

	//getItemIdByName(name)
	lua_register(m_luaState, "getItemIdByName", LuaInterface::luaGetItemIdByName);

	//getItemInfo(itemid)
	lua_register(m_luaState, "getItemInfo", LuaInterface::luaGetItemInfo);

	//getItemAttribute(uid, key)
	lua_register(m_luaState, "getItemAttribute", LuaInterface::luaGetItemAttribute);

	//doItemSetAttribute(uid, key, value)
	lua_register(m_luaState, "doItemSetAttribute", LuaInterface::luaDoItemSetAttribute);

	//doItemEraseAttribute(uid, key)
	lua_register(m_luaState, "doItemEraseAttribute", LuaInterface::luaDoItemEraseAttribute);

	//getItemWeight(uid[, precise = true])
	lua_register(m_luaState, "getItemWeight", LuaInterface::luaGetItemWeight);

	//getItemParent(uid)
	lua_register(m_luaState, "getItemParent", LuaInterface::luaGetItemParent);

	//hasItemProperty(uid, prop)
	lua_register(m_luaState, "hasItemProperty", LuaInterface::luaHasItemProperty);

	//hasPlayerClient(cid)
	lua_register(m_luaState, "hasPlayerClient", LuaInterface::luaHasPlayerClient);

	//hasMonsterRaid(cid)
	lua_register(m_luaState, "hasMonsterRaid", LuaInterface::luaHasMonsterRaid);

	//isIpBanished(ip[, mask])
	lua_register(m_luaState, "isIpBanished", LuaInterface::luaIsIpBanished);

	//isPlayerBanished(name/guid, type)
	lua_register(m_luaState, "isPlayerBanished", LuaInterface::luaIsPlayerBanished);

	//isAccountBanished(accountId[, playerId])
	lua_register(m_luaState, "isAccountBanished", LuaInterface::luaIsAccountBanished);

	//doAddIpBanishment(...)
	lua_register(m_luaState, "doAddIpBanishment", LuaInterface::luaDoAddIpBanishment);

	//doAddPlayerBanishment(...)
	lua_register(m_luaState, "doAddPlayerBanishment", LuaInterface::luaDoAddPlayerBanishment);

	//doAddAccountBanishment(...)
	lua_register(m_luaState, "doAddAccountBanishment", LuaInterface::luaDoAddAccountBanishment);

	//doAddAccountWarnings(...)
	lua_register(m_luaState, "doAddAccountWarnings", LuaInterface::luaDoAddAccountWarnings);

	//getAccountWarnings(accountId)
	lua_register(m_luaState, "getAccountWarnings", LuaInterface::luaGetAccountWarnings);

	//doAddNotation(...)
	lua_register(m_luaState, "doAddNotation", LuaInterface::luaDoAddNotation);

	//doAddStatement(...)
	lua_register(m_luaState, "doAddStatement", LuaInterface::luaDoAddStatement);

	//doRemoveIpBanishment(ip[, mask])
	lua_register(m_luaState, "doRemoveIpBanishment", LuaInterface::luaDoRemoveIpBanishment);

	//doRemovePlayerBanishment(name/guid, type)
	lua_register(m_luaState, "doRemovePlayerBanishment", LuaInterface::luaDoRemovePlayerBanishment);

	//doRemoveAccountBanishment(accountId[, playerId])
	lua_register(m_luaState, "doRemoveAccountBanishment", LuaInterface::luaDoRemoveAccountBanishment);

	//doRemoveNotations(accountId[, playerId])
	lua_register(m_luaState, "doRemoveNotations", LuaInterface::luaDoRemoveNotations);

	//getNotationsCount(accountId[, playerId])
	lua_register(m_luaState, "getNotationsCount", LuaInterface::luaGetNotationsCount);

	//getBanData(value[, type[, param]])
	lua_register(m_luaState, "getBanData", LuaInterface::luaGetBanData);

	//getBanList(type[, value[, param]])
	lua_register(m_luaState, "getBanList", LuaInterface::luaGetBanList);

	//getExperienceStage(level)
	lua_register(m_luaState, "getExperienceStage", LuaInterface::luaGetExperienceStage);

	//getDataDir()
	lua_register(m_luaState, "getDataDir", LuaInterface::luaGetDataDir);

	//getLogsDir()
	lua_register(m_luaState, "getLogsDir", LuaInterface::luaGetLogsDir);

	//getConfigFile()
	lua_register(m_luaState, "getConfigFile", LuaInterface::luaGetConfigFile);

	//isPlayerUsingOtclient(cid)
	lua_register(m_luaState, "isPlayerUsingOtclient", LuaInterface::luaIsPlayerUsingOtclient);

	//doSendPlayerExtendedOpcode(cid, opcode, buffer)
	lua_register(m_luaState, "doSendPlayerExtendedOpcode", LuaInterface::luaDoSendPlayerExtendedOpcode);

	//getConfigValue(key)
	lua_register(m_luaState, "getConfigValue", LuaInterface::luaGetConfigValue);

	//getModList()
	lua_register(m_luaState, "getModList", LuaInterface::luaGetModList);

	//getHighscoreString(skillId)
	lua_register(m_luaState, "getHighscoreString", LuaInterface::luaGetHighscoreString);

	//getWaypointPosition(name)
	lua_register(m_luaState, "getWaypointPosition", LuaInterface::luaGetWaypointPosition);

	//doWaypointAddTemporial(name, pos)
	lua_register(m_luaState, "doWaypointAddTemporial", LuaInterface::luaDoWaypointAddTemporial);

	//getGameState()
	lua_register(m_luaState, "getGameState", LuaInterface::luaGetGameState);

	//doSetGameState(id)
	lua_register(m_luaState, "doSetGameState", LuaInterface::luaDoSetGameState);

	//doExecuteRaid(name)
	lua_register(m_luaState, "doExecuteRaid", LuaInterface::luaDoExecuteRaid);

	//doCreatureExecuteTalkAction(cid, text[, ignoreAccess = false[, channelId = CHANNEL_DEFAULT]])
	lua_register(m_luaState, "doCreatureExecuteTalkAction", LuaInterface::luaDoCreatureExecuteTalkAction);

	//doReloadInfo(id[, cid])
	lua_register(m_luaState, "doReloadInfo", LuaInterface::luaDoReloadInfo);

	//doSaveServer([flags = 13])
	lua_register(m_luaState, "doSaveServer", LuaInterface::luaDoSaveServer);

	//doSaveHouse({list})
	lua_register(m_luaState, "doSaveHouse", LuaInterface::luaDoSaveHouse);

	//doCleanHouse(houseId)
	lua_register(m_luaState, "doCleanHouse", LuaInterface::luaDoCleanHouse);

	//doCleanMap()
	lua_register(m_luaState, "doCleanMap", LuaInterface::luaDoCleanMap);

	//doRefreshMap()
	lua_register(m_luaState, "doRefreshMap", LuaInterface::luaDoRefreshMap);

	//doPlayerSetWalkthrough(cid, uid, walkthrough)
	lua_register(m_luaState, "doPlayerSetWalkthrough", LuaInterface::luaDoPlayerSetWalkthrough);

	//doGuildAddEnemy(guild, enemy, war, type)
	lua_register(m_luaState, "doGuildAddEnemy", LuaInterface::luaDoGuildAddEnemy);

	//doGuildRemoveEnemy(guild, enemy)
	lua_register(m_luaState, "doGuildRemoveEnemy", LuaInterface::luaDoGuildRemoveEnemy);

	//doUpdateHouseAuctions()
	lua_register(m_luaState, "doUpdateHouseAuctions", LuaInterface::luaDoUpdateHouseAuctions);

	//loadmodlib(lib)
	lua_register(m_luaState, "loadmodlib", LuaInterface::luaL_loadmodlib);

	//domodlib(lib)
	lua_register(m_luaState, "domodlib", LuaInterface::luaL_domodlib);

	//dodirectory(dir[, recursively = false])
	lua_register(m_luaState, "dodirectory", LuaInterface::luaL_dodirectory);

	//errors(var)
	lua_register(m_luaState, "errors", LuaInterface::luaL_errors);

	//os table
	luaL_register(m_luaState, "os", LuaInterface::luaSystemTable);

	//db table
	luaL_register(m_luaState, "db", LuaInterface::luaDatabaseTable);

	//result table
	luaL_register(m_luaState, "result", LuaInterface::luaResultTable);

	//bit table
	luaL_register(m_luaState, "bit", LuaInterface::luaBitTable);

	//std table
	luaL_register(m_luaState, "std", LuaInterface::luaStdTable);
}

const luaL_Reg LuaInterface::luaSystemTable[] =
{
	//os.mtime()
	{"mtime", LuaInterface::luaSystemTime},

	{NULL, NULL}
};

const luaL_Reg LuaInterface::luaDatabaseTable[] =
{
	//db.query(query)
	{"query", LuaInterface::luaDatabaseExecute},

	//db.storeQuery(query)
	{"storeQuery", LuaInterface::luaDatabaseStoreQuery},

	//db.escapeString(str)
	{"escapeString", LuaInterface::luaDatabaseEscapeString},

	//db.escapeBlob(s, length)
	{"escapeBlob", LuaInterface::luaDatabaseEscapeBlob},

	//db.lastInsertId()
	{"lastInsertId", LuaInterface::luaDatabaseLastInsertId},

	//db.stringComparer()
	{"stringComparer", LuaInterface::luaDatabaseStringComparer},

	//db.updateLimiter()
	{"updateLimiter", LuaInterface::luaDatabaseUpdateLimiter},

	//db.connected()
	{"connected", LuaInterface::luaDatabaseConnected},

	//db.tableExists(name)
	{"tableExists", LuaInterface::luaDatabaseTableExists},

	//db.transBegin()
	{"transBegin", LuaInterface::luaDatabaseTransBegin},

	//db.transRollback()
	{"transRollback", LuaInterface::luaDatabaseTransRollback},

	//db.transCommit()
	{"transCommit", LuaInterface::luaDatabaseTransCommit},

	{NULL, NULL}
};

const luaL_Reg LuaInterface::luaResultTable[] =
{
	//result.getDataInt(resId, s)
	{"getDataInt", LuaInterface::luaResultGetDataInt},

	//result.getDataLong(resId, s)
	{"getDataLong", LuaInterface::luaResultGetDataLong},

	//result.getDataString(resId, s)
	{"getDataString", LuaInterface::luaResultGetDataString},

	//result.getDataStream(resId, s, length)
	{"getDataStream", LuaInterface::luaResultGetDataStream},

	//result.next(resId)
	{"next", LuaInterface::luaResultNext},

	//result.free(resId)
	{"free", LuaInterface::luaResultFree},

	{NULL, NULL}
};

const luaL_Reg LuaInterface::luaBitTable[] =
{
	//{"cast", LuaInterface::luaBitCast},
	{"bnot", LuaInterface::luaBitNot},
	{"band", LuaInterface::luaBitAnd},
	{"bor", LuaInterface::luaBitOr},
	{"bxor", LuaInterface::luaBitXor},
	{"lshift", LuaInterface::luaBitLeftShift},
	{"rshift", LuaInterface::luaBitRightShift},
	//{"arshift", LuaInterface::luaBitArithmeticalRightShift},

	//{"ucast", LuaInterface::luaBitUCast},
	{"ubnot", LuaInterface::luaBitUNot},
	{"uband", LuaInterface::luaBitUAnd},
	{"ubor", LuaInterface::luaBitUOr},
	{"ubxor", LuaInterface::luaBitUXor},
	{"ulshift", LuaInterface::luaBitULeftShift},
	{"urshift", LuaInterface::luaBitURightShift},
	//{"uarshift", LuaInterface::luaBitUArithmeticalRightShift},

	{NULL, NULL}
};

const luaL_Reg LuaInterface::luaStdTable[] =
{
	{"cout", LuaInterface::luaStdCout},
	{"clog", LuaInterface::luaStdClog},
	{"cerr", LuaInterface::luaStdCerr},

	{"md5", LuaInterface::luaStdMD5},
	{"sha1", LuaInterface::luaStdSHA1},
	{"sha256", LuaInterface::luaStdSHA256},
	{"sha512", LuaInterface::luaStdSHA512},

	{"checkName", LuaInterface::luaStdCheckName},
	{NULL, NULL}
};

int32_t LuaInterface::internalGetPlayerInfo(lua_State* L, PlayerInfo_t info)
{
	ScriptEnviroment* env = getEnv();
	const Player* player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		std::stringstream s;
		s << getError(LUA_ERROR_PLAYER_NOT_FOUND) << " when requesting player info #" << info;
		errorEx(s.str());

		lua_pushboolean(L, false);
		return 1;
	}

	int64_t value = 0;
	Position pos;
	switch(info)
	{
		case PlayerInfoNameDescription:
			lua_pushstring(L, player->getNameDescription().c_str());
			return 1;
		case PlayerInfoSpecialDescription:
			lua_pushstring(L, player->getSpecialDescription().c_str());
			return 1;
		case PlayerInfoAccess:
			value = player->getAccess();
			break;
		case PlayerInfoGhostAccess:
			value = player->getGhostAccess();
			break;
		case PlayerInfoLevel:
			value = player->getLevel();
			break;
		case PlayerInfoExperience:
			value = player->getExperience();
			break;
		case PlayerInfoManaSpent:
			value = player->getSpentMana();
			break;
		case PlayerInfoTown:
			value = player->getTown();
			break;
		case PlayerInfoPromotionLevel:
			value = player->getPromotionLevel();
			break;
		case PlayerInfoGUID:
			value = player->getGUID();
			break;
		case PlayerInfoAccountId:
			value = player->getAccount();
			break;
		case PlayerInfoAccount:
			lua_pushstring(L, player->getAccountName().c_str());
			return 1;
		case PlayerInfoPremiumDays:
			value = player->getPremiumDays();
			break;
		case PlayerInfoFood:
		{
			if(Condition* condition = player->getCondition(CONDITION_REGENERATION, CONDITIONID_DEFAULT))
				value = condition->getTicks() / 1000;

			break;
		}
		case PlayerInfoVocation:
			value = player->getVocationId();
			break;
		case PlayerInfoMoney:
			value = g_game.getMoney(player);
			break;
		case PlayerInfoFreeCap:
			value = (int64_t)player->getFreeCapacity();
			break;
		case PlayerInfoGuildId:
			value = player->getGuildId();
			break;
		case PlayerInfoGuildName:
			lua_pushstring(L, player->getGuildName().c_str());
			return 1;
		case PlayerInfoGuildRankId:
			value = player->getRankId();
			break;
		case PlayerInfoGuildRank:
			lua_pushstring(L, player->getRankName().c_str());
			return 1;
		case PlayerInfoGuildLevel:
			value = player->getGuildLevel();
			break;
		case PlayerInfoGuildNick:
			lua_pushstring(L, player->getGuildNick().c_str());
			return 1;
		case PlayerInfoGroupId:
			value = player->getGroupId();
			break;
		case PlayerInfoBalance:
			if(g_config.getBool(ConfigManager::BANK_SYSTEM))
				lua_pushnumber(L, player->balance);
			else
				lua_pushnumber(L, 0);

			return 1;
		case PlayerInfoStamina:
			value = player->getStaminaMinutes();
			break;
		case PlayerInfoLossSkill:
			lua_pushboolean(L, player->getLossSkill());
			return 1;
		case PlayerInfoMarriage:
			value = player->marriage;
			break;
		case PlayerInfoPzLock:
			lua_pushboolean(L, player->isPzLocked());
			return 1;
		case PlayerInfoSaving:
			lua_pushboolean(L, player->isSaving());
			return 1;
		case PlayerInfoProtected:
			lua_pushboolean(L, player->isProtected());
			return 1;
		case PlayerInfoIp:
			value = player->getIP();
			break;
		case PlayerInfoSkullEnd:
			value = player->getSkullEnd();
			break;
		case PlayerInfoOutfitWindow:
			player->sendOutfitWindow();
			lua_pushboolean(L, true);
			return 1;
		case PlayerInfoIdleTime:
			value = player->getIdleTime();
			break;
		case PlayerInfoClient:
			lua_pushboolean(L, player->hasClient());
			return 1;
		case PlayerInfoLastLoad:
			value = player->getLastLoad();
			break;
		case PlayerInfoLastLogin:
			value = player->getLastLogin();
			break;
		case PlayerInfoAccountManager:
			value = player->accountManager;
			break;
		case PlayerInfoTradeState:
			value = player->tradeState;
			break;
		case PlayerInfoOperatingSystem:
			value = player->getOperatingSystem();
			break;
		case PlayerInfoClientVersion:
			value = player->getClientVersion();
			break;
		default:
			errorEx("Unknown player info #" + info);
			value = 0;
			break;
	}

	lua_pushnumber(L, value);
	return 1;
}

//getPlayer[Info](uid)
int32_t LuaInterface::luaGetPlayerNameDescription(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoNameDescription);
}

int32_t LuaInterface::luaGetPlayerSpecialDescription(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoSpecialDescription);
}

int32_t LuaInterface::luaGetPlayerFood(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoFood);
}

int32_t LuaInterface::luaGetPlayerAccess(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoAccess);
}

int32_t LuaInterface::luaGetPlayerGhostAccess(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoGhostAccess);
}

int32_t LuaInterface::luaGetPlayerLevel(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoLevel);
}

int32_t LuaInterface::luaGetPlayerExperience(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoExperience);
}

int32_t LuaInterface::luaGetPlayerSpentMana(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoManaSpent);
}

int32_t LuaInterface::luaGetPlayerVocation(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoVocation);
}

int32_t LuaInterface::luaGetPlayerMoney(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoMoney);
}

int32_t LuaInterface::luaGetPlayerFreeCap(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoFreeCap);
}

int32_t LuaInterface::luaGetPlayerGuildId(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoGuildId);
}

int32_t LuaInterface::luaGetPlayerGuildName(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoGuildName);
}

int32_t LuaInterface::luaGetPlayerGuildRankId(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoGuildRankId);
}

int32_t LuaInterface::luaGetPlayerGuildRank(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoGuildRank);
}

int32_t LuaInterface::luaGetPlayerGuildLevel(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoGuildLevel);
}

int32_t LuaInterface::luaGetPlayerGuildNick(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoGuildNick);
}

int32_t LuaInterface::luaGetPlayerTown(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoTown);
}

int32_t LuaInterface::luaGetPlayerPromotionLevel(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoPromotionLevel);
}

int32_t LuaInterface::luaGetPlayerGroupId(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoGroupId);
}

int32_t LuaInterface::luaGetPlayerGUID(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoGUID);
}

int32_t LuaInterface::luaGetPlayerAccountId(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoAccountId);
}

int32_t LuaInterface::luaGetPlayerAccount(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoAccount);
}

int32_t LuaInterface::luaGetPlayerPremiumDays(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoPremiumDays);
}

int32_t LuaInterface::luaGetPlayerBalance(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoBalance);
}

int32_t LuaInterface::luaGetPlayerStamina(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoStamina);
}

int32_t LuaInterface::luaGetPlayerLossSkill(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoLossSkill);
}

int32_t LuaInterface::luaGetPlayerPartner(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoMarriage);
}

int32_t LuaInterface::luaIsPlayerPzLocked(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoPzLock);
}

int32_t LuaInterface::luaIsPlayerSaving(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoSaving);
}

int32_t LuaInterface::luaIsPlayerProtected(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoProtected);
}

int32_t LuaInterface::luaGetPlayerIp(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoIp);
}

int32_t LuaInterface::luaGetPlayerSkullEnd(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoSkullEnd);
}

int32_t LuaInterface::luaDoPlayerSendOutfitWindow(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoOutfitWindow);
}

int32_t LuaInterface::luaGetPlayerIdleTime(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoIdleTime);
}

int32_t LuaInterface::luaHasPlayerClient(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoClient);
}

int32_t LuaInterface::luaGetPlayerLastLoad(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoLastLoad);
}

int32_t LuaInterface::luaGetPlayerLastLogin(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoLastLogin);
}

int32_t LuaInterface::luaGetPlayerAccountManager(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoAccountManager);
}

int32_t LuaInterface::luaGetPlayerTradeState(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoTradeState);
}

int32_t LuaInterface::luaGetPlayerOperatingSystem(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoOperatingSystem);
}

int32_t LuaInterface::luaGetPlayerClientVersion(lua_State* L)
{
	return internalGetPlayerInfo(L, PlayerInfoClientVersion);
}
//

int32_t LuaInterface::luaGetPlayerSex(lua_State* L)
{
	//getPlayerSex(cid[, full = false])
	bool full = false;
	if(lua_gettop(L) > 1)
		full = popBoolean(L);

	ScriptEnviroment* env = getEnv();
	Player* player = env->getPlayerByUID((uint32_t)popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}
	else
		lua_pushnumber(L, player->getSex(full));

	return 1;
}

int32_t LuaInterface::luaDoPlayerSetNameDescription(lua_State* L)
{
	//doPlayerSetNameDescription(cid, description)
	std::string description = popString(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->nameDescription += description;
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSetSpecialDescription(lua_State* L)
{
	//doPlayerSetSpecialDescription(cid, description)
	std::string description = popString(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->setSpecialDescription(description);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetPlayerMagLevel(lua_State* L)
{
	//getPlayerMagLevel(cid[, ignoreModifiers = false])
	bool ignoreModifiers = false;
	if(lua_gettop(L) > 1)
		ignoreModifiers = popBoolean(L);

	ScriptEnviroment* env = getEnv();
	if(const Player* player = env->getPlayerByUID(popNumber(L)))
		lua_pushnumber(L, ignoreModifiers ? player->magLevel : player->getMagicLevel());
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetPlayerRequiredMana(lua_State* L)
{
	//getPlayerRequiredMana(cid, magicLevel)
	uint32_t magLevel = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
		lua_pushnumber(L, player->vocation->getReqMana(magLevel));
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetPlayerRequiredSkillTries(lua_State* L)
{
	//getPlayerRequiredSkillTries(cid, skill, level)
	int32_t level = popNumber(L), skill = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
		lua_pushnumber(L, player->vocation->getReqSkillTries(skill, level));
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetPlayerFlagValue(lua_State* L)
{
	//getPlayerFlagValue(cid, flag)
	uint32_t index = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		if(index < PlayerFlag_LastFlag)
			lua_pushboolean(L, player->hasFlag((PlayerFlags)index));
		else
		{
			errorEx("No valid flag index - " + asString<uint32_t>(index));
			lua_pushboolean(L, false);
		}
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetPlayerCustomFlagValue(lua_State* L)
{
	//getPlayerCustomFlagValue(cid, flag)
	uint32_t index = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		if(index < PlayerCustomFlag_LastFlag)
			lua_pushboolean(L, player->hasCustomFlag((PlayerCustomFlags)index));
		else
		{
			errorEx("No valid flag index - " + asString<uint32_t>(index));
			lua_pushboolean(L, false);
		}
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerLearnInstantSpell(lua_State* L)
{
	//doPlayerLearnInstantSpell(cid, name)
	std::string spellName = popString(L);

	ScriptEnviroment* env = getEnv();
	Player* player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	InstantSpell* spell = g_spells->getInstantSpellByName(spellName);
	if(!spell)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	player->learnInstantSpell(spell->getName());
	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaDoPlayerUnlearnInstantSpell(lua_State* L)
{
	//doPlayerUnlearnInstantSpell(cid, name)
	std::string spellName = popString(L);

	ScriptEnviroment* env = getEnv();
	Player* player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	InstantSpell* spell = g_spells->getInstantSpellByName(spellName);
	if(!spell)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	player->unlearnInstantSpell(spell->getName());
	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaGetPlayerLearnedInstantSpell(lua_State* L)
{
	//getPlayerLearnedInstantSpell(cid, name)
	std::string spellName = popString(L);

	ScriptEnviroment* env = getEnv();
	Player* player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	InstantSpell* spell = g_spells->getInstantSpellByName(spellName);
	if(!spell)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	lua_pushboolean(L, player->hasLearnedInstantSpell(spellName));
	return 1;
}

int32_t LuaInterface::luaGetPlayerInstantSpellCount(lua_State* L)
{
	//getPlayerInstantSpellCount(cid)
	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
		lua_pushnumber(L, g_spells->getInstantSpellCount(player));
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetPlayerInstantSpellInfo(lua_State* L)
{
	//getPlayerInstantSpellInfo(cid, index)
	uint32_t index = popNumber(L);

	ScriptEnviroment* env = getEnv();
	Player* player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	InstantSpell* spell = g_spells->getInstantSpellByIndex(player, index);
	if(!spell)
	{
		errorEx(getError(LUA_ERROR_SPELL_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	lua_newtable(L);
	setField(L, "name", spell->getName());
	setField(L, "words", spell->getWords());
	setField(L, "level", spell->getLevel());
	setField(L, "mlevel", spell->getMagicLevel());
	setField(L, "mana", spell->getManaCost(player));
	setField(L, "manapercent", spell->getManaPercent());
	return 1;
}

int32_t LuaInterface::luaGetInstantSpellInfo(lua_State* L)
{
	//getInstantSpellInfo(name)
	InstantSpell* spell = g_spells->getInstantSpellByName(popString(L));
	if(!spell)
	{
		errorEx(getError(LUA_ERROR_SPELL_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	lua_newtable(L);
	setField(L, "name", spell->getName());
	setField(L, "words", spell->getWords());
	setField(L, "level", spell->getLevel());
	setField(L, "mlevel", spell->getMagicLevel());
	setField(L, "mana", spell->getManaCost(NULL));
	setField(L, "manapercent", spell->getManaPercent());
	return 1;
}

int32_t LuaInterface::luaDoCreatureCastSpell(lua_State* L)
{
	//doCreatureCastSpell (uid, spell)
	std::string spellname = popString(L);

	ScriptEnviroment *env = getEnv();
	Creature *creature = env->getCreatureByUID(popNumber(L));
	if(!creature)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean (L, false);
		return 1;
	}

	Spell *spell = g_spells->getInstantSpellByName(spellname);
	if(spell)
	{
		if(spell->castSpell(creature))
		{
			lua_pushboolean(L, true);
			return 1;
		}
	}

	errorEx(getError(LUA_ERROR_SPELL_NOT_FOUND));
	lua_pushboolean(L, false);
	return 1;
}

int32_t LuaInterface::luaDoRemoveItem(lua_State* L)
{
	//doRemoveItem(uid[, count = -1])
	int32_t count = -1;
	if(lua_gettop(L) > 1)
		count = popNumber(L);

	ScriptEnviroment* env = getEnv();
	Item* item = env->getItemByUID(popNumber(L));
	if(!item)
	{
		errorEx(getError(LUA_ERROR_ITEM_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	if(g_game.internalRemoveItem(NULL, item, count) != RET_NOERROR)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaDoPlayerRemoveItem(lua_State* L)
{
	//doPlayerRemoveItem(cid, itemid, count[, subType = -1[, ignoreEquipped = false]])
	int32_t params = lua_gettop(L), subType = -1;
	bool ignoreEquipped = false;
	if(params > 4)
		ignoreEquipped = popBoolean(L);

	if(params > 3)
		subType = popNumber(L);

	uint32_t count = popNumber(L);
	uint16_t itemId = (uint16_t)popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
		lua_pushboolean(L, g_game.removeItemOfType(player, itemId, count, subType, ignoreEquipped));
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerFeed(lua_State* L)
{
	//doPlayerFeed(cid, food)
	int32_t food = (int32_t)popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->addDefaultRegeneration((food * 1000) * 3);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSendCancel(lua_State* L)
{
	//doPlayerSendCancel(cid, text)
	std::string text = popString(L);
	ScriptEnviroment* env = getEnv();
	if(const Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->sendCancel(text);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoSendDefaultCancel(lua_State* L)
{
	//doPlayerSendDefaultCancel(cid, ReturnValue)
	ReturnValue ret = (ReturnValue)popNumber(L);
	ScriptEnviroment* env = getEnv();
	if(const Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->sendCancelMessage(ret);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetSearchString(lua_State* L)
{
	//getSearchString(fromPosition, toPosition[, fromIsCreature = false[, toIsCreature = false]])
	PositionEx toPos, fromPos;
	bool toIsCreature = false, fromIsCreature = false;

	int32_t params = lua_gettop(L);
	if(params > 3)
		toIsCreature = popBoolean(L);

	if(params > 2)
		fromIsCreature = popBoolean(L);

	popPosition(L, toPos);
	popPosition(L, fromPos);
	if(!toPos.x || !toPos.y || !fromPos.x || !fromPos.y)
	{
		errorEx("Wrong position(s) specified");
		lua_pushboolean(L, false);
	}
	else
		lua_pushstring(L, g_game.getSearchString(fromPos, toPos, fromIsCreature, toIsCreature).c_str());

	return 1;
}

int32_t LuaInterface::luaGetClosestFreeTile(lua_State* L)
{
	//getClosestFreeTile(cid, targetPos[, extended = false[, ignoreHouse = true]])
	uint32_t params = lua_gettop(L);
	bool ignoreHouse = true, extended = false;
	if(params > 3)
		ignoreHouse = popBoolean(L);

	if(params > 2)
		extended = popBoolean(L);

	PositionEx pos;
	popPosition(L, pos);

	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		Position newPos = g_game.getClosestFreeTile(creature, pos, extended, ignoreHouse);
		if(newPos.x != 0)
			pushPosition(L, newPos, 0);
		else
			lua_pushboolean(L, false);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoTeleportThing(lua_State* L)
{
	//doTeleportThing(cid, destination[, pushMove = true[, fullTeleport = true]])
	bool fullTeleport = true, pushMove = true;
	int32_t params = lua_gettop(L);
	if(params > 3)
		fullTeleport = popBoolean(L);

	if(params > 2)
		pushMove = popBoolean(L);

	PositionEx pos;
	popPosition(L, pos);

	ScriptEnviroment* env = getEnv();
	if(Thing* tmp = env->getThingByUID(popNumber(L)))
		lua_pushboolean(L, g_game.internalTeleport(tmp, pos, !pushMove, FLAG_NOLIMIT, fullTeleport) == RET_NOERROR);
	else
	{
		errorEx(getError(LUA_ERROR_THING_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoItemSetDestination(lua_State* L)
{
	//doItemSetDestination(uid, destination)
	PositionEx destination;
	popPosition(L, destination);

	ScriptEnviroment* env = getEnv();
	Item* item = env->getItemByUID(popNumber(L));
	if(!item)
	{
		errorEx(getError(LUA_ERROR_ITEM_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	if(Teleport* teleport = item->getTeleport())
	{
		teleport->setDestination(destination);
		lua_pushboolean(L, true);
		return 1;
	}

	errorEx("Target item is not a teleport.");
	lua_pushboolean(L, false);
	return 1;
}

int32_t LuaInterface::luaDoTransformItem(lua_State* L)
{
	//doTransformItem(uid, newId[, count/subType])
	int32_t count = -1;
	if(lua_gettop(L) > 2)
		count = popNumber(L);

	uint32_t newId = popNumber(L), uid = popNumber(L);
	ScriptEnviroment* env = getEnv();

	Item* item = env->getItemByUID(uid);
	if(!item)
	{
		errorEx(getError(LUA_ERROR_ITEM_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	const ItemType& it = Item::items[newId];
	if(it.stackable && count > 100)
		count = 100;

	Item* newItem = g_game.transformItem(item, newId, count);
	if(newItem && newItem != item)
	{
		env->removeThing(uid);
		env->insertThing(uid, newItem);

		if(newItem->getUniqueId() != 0)
			env->addUniqueThing(newItem);
	}

	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaDoCreatureSay(lua_State* L)
{
	//doCreatureSay(uid, text[, type = SPEAK_SAY[, ghost = false[, cid = 0[, pos]]]])
	uint32_t params = lua_gettop(L), cid = 0, uid = 0;
	PositionEx pos;
	if(params > 5)
		popPosition(L, pos);

	if(params > 4)
		cid = popNumber(L);

	bool ghost = false;
	if(params > 3)
		ghost = popBoolean(L);

	MessageClasses type = MSG_SPEAK_SAY;
	if(params > 2)
		type = (MessageClasses)popNumber(L);

	std::string text = popString(L);

	uid = popNumber(L);
	if(params > 5 && (!pos.x || !pos.y))
	{
		errorEx("Invalid position specified");
		lua_pushboolean(L, false);
		return 1;
	}

	ScriptEnviroment* env = getEnv();
	Creature* creature = env->getCreatureByUID(uid);
	if(!creature)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	SpectatorVec list;
	if(cid)
	{
		Creature* target = env->getCreatureByUID(cid);
		if(!target)
		{
			errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
			lua_pushboolean(L, false);
			return 1;
		}

		list.push_back(target);
	}

	if(params > 5)
		lua_pushboolean(L, g_game.internalCreatureSay(creature, type, text, ghost, &list, &pos));
	else
		lua_pushboolean(L, g_game.internalCreatureSay(creature, type, text, ghost, &list));

	return 1;
}

int32_t LuaInterface::luaDoCreatureChannelSay(lua_State* L)
{
	//doCreatureChannelSay(target, uid, message, type, channel)
	ScriptEnviroment* env = getEnv();
	uint16_t channelId = popNumber(L);
	std::string text = popString(L);
	uint32_t speakClass = popNumber(L), targetId = popNumber(L);

	Player* player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	Creature* creature = env->getCreatureByUID(targetId);
	if(!creature)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	player->sendCreatureChannelSay(creature, (MessageClasses)speakClass, text, channelId);
	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaDoSendMagicEffect(lua_State* L)
{
	//doSendMagicEffect(pos, type[, player])
	ScriptEnviroment* env = getEnv();
	SpectatorVec list;
	if(lua_gettop(L) > 2)
	{
		if(Creature* creature = env->getCreatureByUID(popNumber(L)))
			list.push_back(creature);
	}

	uint32_t type = popNumber(L);
	PositionEx pos;

	popPosition(L, pos);
	if(pos.x == 0xFFFF)
		pos = env->getRealPos();

	if(!list.empty())
		g_game.addMagicEffect(list, pos, type);
	else
		g_game.addMagicEffect(pos, type);

	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaDoSendDistanceShoot(lua_State* L)
{
	//doSendDistanceShoot(fromPos, toPos, type[, player])
	ScriptEnviroment* env = getEnv();
	SpectatorVec list;
	if(lua_gettop(L) > 3)
	{
		if(Creature* creature = env->getCreatureByUID(popNumber(L)))
			list.push_back(creature);
	}

	uint32_t type = popNumber(L);
	PositionEx toPos, fromPos;

	popPosition(L, toPos);
	popPosition(L, fromPos);
	if(fromPos.x == 0xFFFF)
		fromPos = env->getRealPos();

	if(toPos.x == 0xFFFF)
		toPos = env->getRealPos();

	if(!list.empty())
		g_game.addDistanceEffect(list, fromPos, toPos, type);
	else
		g_game.addDistanceEffect(fromPos, toPos, type);

	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaDoPlayerAddSkillTry(lua_State* L)
{
	//doPlayerAddSkillTry(uid, skillid, n[, useMultiplier = true])
	bool multiplier = true;
	if(lua_gettop(L) > 3)
		multiplier = popBoolean(L);

	uint64_t n = popNumber(L);
	uint16_t skillid = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->addSkillAdvance((skills_t)skillid, n, multiplier);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetCreatureSpeakType(lua_State* L)
{
	//getCreatureSpeakType(uid)
	ScriptEnviroment* env = getEnv();
	if(const Creature* creature = env->getCreatureByUID(popNumber(L)))
		lua_pushnumber(L, (MessageClasses)creature->getSpeakType());
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoCreatureSetSpeakType(lua_State* L)
{
	//doCreatureSetSpeakType(uid, type)
	MessageClasses type = (MessageClasses)popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		if(!((type >= MSG_SPEAK_FIRST && type <= MSG_SPEAK_LAST) ||
			(type >= MSG_SPEAK_MONSTER_FIRST && type <= MSG_SPEAK_MONSTER_LAST)))
		{
			errorEx("Invalid speak type");
			lua_pushboolean(L, false);
			return 1;
		}

		creature->setSpeakType(type);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetCreatureHideHealth(lua_State* L)
{
	//getCreatureHideHealth(cid)
	ScriptEnviroment* env = getEnv();

	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
		lua_pushboolean(L, creature->getHideHealth());
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoCreatureSetHideHealth(lua_State* L)
{
	//doCreatureSetHideHealth(cid, hide)
	bool hide = popBoolean(L);

	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		creature->setHideHealth(hide);
		g_game.addCreatureHealth(creature);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoCreatureAddHealth(lua_State* L)
{
	//doCreatureAddHealth(uid, health[, hitEffect[, hitColor[, force]]])
	int32_t params = lua_gettop(L);
	bool force = false;
	if(params > 4)
		force = popBoolean(L);

	Color_t hitColor = COLOR_UNKNOWN;
	if(params > 3)
		hitColor = (Color_t)popNumber(L);

	MagicEffect_t hitEffect = MAGIC_EFFECT_UNKNOWN;
	if(params > 2)
		hitEffect = (MagicEffect_t)popNumber(L);

	int32_t healthChange = popNumber(L);
	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		if(healthChange) //do not post with 0 value
			g_game.combatChangeHealth(healthChange < 1 ? COMBAT_UNDEFINEDDAMAGE : COMBAT_HEALING,
				NULL, creature, healthChange, hitEffect, hitColor, force);

		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoCreatureAddMana(lua_State* L)
{
	//doCreatureAddMana(uid, mana[, aggressive])
	bool aggressive = true;
	if(lua_gettop(L) > 2)
		aggressive = popBoolean(L);

	int32_t manaChange = popNumber(L);
	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		if(aggressive)
			g_game.combatChangeMana(NULL, creature, manaChange);
		else
			creature->changeMana(manaChange);

		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerAddSpentMana(lua_State* L)
{
	//doPlayerAddSpentMana(cid, amount[, useMultiplier = true])
	bool multiplier = true;
	if(lua_gettop(L) > 2)
		multiplier = popBoolean(L);

	uint32_t amount = popNumber(L);
	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->addManaSpent(amount, multiplier);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerAddItem(lua_State* L)
{
	//doPlayerAddItem(cid, itemid[, count/subtype = 1[, canDropOnMap = true[, slot = 0]]])
	//doPlayerAddItem(cid, itemid[, count = 1[, canDropOnMap = true[, subtype = 1[, slot = 0]]]])
	int32_t params = lua_gettop(L), subType = 1, slot = SLOT_WHEREEVER;
	if(params > 5)
		slot = popNumber(L);

	if(params > 4)
	{
		if(params > 5)
			subType = popNumber(L);
		else
			slot = popNumber(L);
	}

	bool canDropOnMap = true;
	if(params > 3)
		canDropOnMap = popBoolean(L);

	uint32_t count = 1;
	if(params > 2)
		count = popNumber(L);

	uint32_t itemId = popNumber(L);
	if(slot > SLOT_AMMO)
	{
		errorEx("Invalid slot");
		lua_pushboolean(L, false);
		return 1;
	}

	ScriptEnviroment* env = getEnv();
	Player* player = env->getPlayerByUID((uint32_t)popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	const ItemType& it = Item::items[itemId];
	int32_t itemCount = 1;
	if(params > 4)
		itemCount = std::max((uint32_t)1, count);
	else if(it.hasSubType())
	{
		if(it.stackable)
			itemCount = (int32_t)std::ceil((float)count / 100);

		subType = count;
	}

	uint32_t ret = 0;
	Item* newItem = NULL;
	while(itemCount > 0)
	{
		int32_t stackCount = std::min(100, subType);
		if(!(newItem = Item::CreateItem(itemId, stackCount)))
		{
			errorEx(getError(LUA_ERROR_ITEM_NOT_FOUND));
			lua_pushboolean(L, false);
			return ++ret;
		}

		if(it.stackable)
			subType -= stackCount;

		Item* stackItem = NULL;
		if(g_game.internalPlayerAddItem(NULL, player, newItem, canDropOnMap, (slots_t)slot, &stackItem) != RET_NOERROR)
		{
			delete newItem;
			lua_pushboolean(L, false);
			return ++ret;
		}

		++ret;
		if(newItem->getParent())
			lua_pushnumber(L, env->addThing(newItem));
		else if(stackItem)
			lua_pushnumber(L, env->addThing(stackItem));
		else
			lua_pushnil(L);

		--itemCount;
	}

	if(ret)
		return ret;

	lua_pushnil(L);
	return 1;
}

int32_t LuaInterface::luaDoPlayerAddItemEx(lua_State* L)
{
	//doPlayerAddItemEx(cid, uid[, canDropOnMap = false[, slot = 0]])
	int32_t params = lua_gettop(L), slot = SLOT_WHEREEVER;
	if(params > 3)
		slot = popNumber(L);

	bool canDropOnMap = false;
	if(params > 2)
		canDropOnMap = popBoolean(L);

	uint32_t uid = (uint32_t)popNumber(L);
	if(slot > SLOT_AMMO)
	{
		errorEx("Invalid slot");
		lua_pushboolean(L, false);
		return 1;
	}

	ScriptEnviroment* env = getEnv();
	Player* player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	Item* item = env->getItemByUID(uid);
	if(!item)
	{
		errorEx(getError(LUA_ERROR_ITEM_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	if(item->getParent() == VirtualCylinder::virtualCylinder)
	{
		env->removeTempItem(env, item);
		ReturnValue ret = g_game.internalPlayerAddItem(NULL, player, item, canDropOnMap, (slots_t)slot);
		if(ret != RET_NOERROR)
			env->addTempItem(env, item);

		lua_pushnumber(L, ret);
	}
	else
		lua_pushboolean(L, false);

	return 1;
}

int32_t LuaInterface::luaDoTileAddItemEx(lua_State* L)
{
	//doTileAddItemEx(pos, uid)
	uint32_t uid = (uint32_t)popNumber(L);
	PositionEx pos;
	popPosition(L, pos);

	ScriptEnviroment* env = getEnv();
	Item* item = env->getItemByUID(uid);
	if(!item)
	{
		errorEx(getError(LUA_ERROR_ITEM_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	Tile* tile = g_game.getTile(pos);
	if(!tile)
	{
		if(item->isGroundTile())
		{
			if(item->getParent() == VirtualCylinder::virtualCylinder)
			{
				tile = IOMap::createTile(item, NULL, pos.x, pos.y, pos.z);
				g_game.setTile(tile);

				env->removeTempItem(env, item);
				lua_pushnumber(L, RET_NOERROR);
			}
			else
				lua_pushboolean(L, false);
		}
		else
		{
			errorEx(getError(LUA_ERROR_TILE_NOT_FOUND));
			lua_pushboolean(L, false);
		}

		return 1;
	}

	if(item->getParent() == VirtualCylinder::virtualCylinder)
	{
		ReturnValue ret = g_game.internalAddItem(NULL, tile, item);
		if(ret == RET_NOERROR)
			env->removeTempItem(env, item);

		lua_pushnumber(L, ret);
	}
	else
		lua_pushboolean(L, false);

	return 1;
}

int32_t LuaInterface::luaDoRelocate(lua_State* L)
{
	//doRelocate(pos, posTo[, creatures = true[, unmovable = true]])
	//Moves all[ movable] objects from pos to posTo
	//Uses protected methods for optimal speed
	bool unmovable = true, creatures = true;
	int32_t params = lua_gettop(L);
	if(params > 3)
		unmovable = popBoolean(L);

	if(params > 2)
		creatures = popBoolean(L);

	PositionEx toPos;
	popPosition(L, toPos);

	PositionEx fromPos;
	popPosition(L, fromPos);

	Tile* fromTile = g_game.getTile(fromPos);
	if(!fromTile)
	{
		errorEx(getError(LUA_ERROR_TILE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	Tile* toTile = g_game.getTile(toPos);
	if(!toTile)
	{
		errorEx(getError(LUA_ERROR_TILE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	if(fromTile == toTile)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	TileItemVector *toItems = toTile->getItemList(),
		*fromItems = fromTile->getItemList();
	if(fromItems && toItems)
	{
		int32_t itemLimit = g_config.getNumber(toTile->hasFlag(TILESTATE_PROTECTIONZONE)
			? ConfigManager::PROTECTION_TILE_LIMIT : ConfigManager::TILE_LIMIT), count = 0;
		for(ItemVector::iterator it = fromItems->getBeginDownItem(); it != fromItems->getEndDownItem(); )
		{
			if(itemLimit && (int32_t)toItems->size() > itemLimit)
				break;

			const ItemType& iType = Item::items[(*it)->getID()];
			if(!iType.isGroundTile() && !iType.alwaysOnTop && !iType.isMagicField() && (unmovable || iType.movable))
			{
				if(Item* item = (*it))
				{
					it = fromItems->erase(it);
					fromItems->removeDownItem();
					fromTile->updateTileFlags(item, true);

					g_moveEvents->onItemMove(NULL, item, fromTile, false);
					g_moveEvents->onRemoveTileItem(fromTile, item);

					item->setParent(toTile);
					++count;

					toItems->insert(toItems->getBeginDownItem(), item);
					toItems->addDownItem();
					toTile->updateTileFlags(item, false);

					g_moveEvents->onAddTileItem(toTile, item);
					g_moveEvents->onItemMove(NULL, item, toTile, true);
				}
				else
					++it;
			}
			else
				++it;
		}

		fromTile->updateThingCount(-count);
		toTile->updateThingCount(count);

		fromTile->onUpdateTile();
		toTile->onUpdateTile();
		if(g_config.getBool(ConfigManager::STORE_TRASH)
			&& fromTile->hasFlag(TILESTATE_TRASHED))
		{
			g_game.addTrash(toPos);
			toTile->setFlag(TILESTATE_TRASHED);
		}
	}

	if(creatures)
	{
		CreatureVector* creatureVector = fromTile->getCreatures();
		Creature* creature = NULL;
		while(creatureVector && !creatureVector->empty())
		{
			if((creature = (*creatureVector->begin())))
				g_game.internalMoveCreature(NULL, creature, fromTile, toTile, FLAG_NOLIMIT);
		}
	}

	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaDoCleanTile(lua_State* L)
{
	//doCleanTile(pos, forceMapLoaded = false)
	//Remove all items from tile, ignore creatures
	bool forceMapLoaded = false;
	if(lua_gettop(L) > 1)
		forceMapLoaded = popBoolean(L);

	PositionEx pos;
	popPosition(L, pos);

	Tile* tile = g_game.getTile(pos);
	if(!tile)
	{
		errorEx(getError(LUA_ERROR_TILE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	Thing* thing = NULL;
	Item* item = NULL;

	for(int32_t i = tile->getThingCount() - 1; i >= 1; --i) //ignore ground
	{
		if(!(thing = tile->__getThing(i)) || !(item = thing->getItem()))
			continue;

		if(!item->isLoadedFromMap() || forceMapLoaded)
			g_game.internalRemoveItem(NULL, item);
	}

	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaDoPlayerSendTextMessage(lua_State* L)
{
	//doPlayerSendTextMessage(cid, MessageClasses, message[, value[, color[, position]]])
	int32_t args = lua_gettop(L), value = 0, color = COLOR_WHITE;
	PositionEx position;
	if(args > 5)
		popPosition(L, position);

	if(args > 4)
		color = popNumber(L);

	if(args > 3)
		value = popNumber(L);

	std::string text = popString(L);
	uint32_t messageClass = popNumber(L);

	ScriptEnviroment* env = getEnv();
	Player* player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	if(args > 3)
	{
		if(!position.x || !position.y)
			position = player->getPosition();

		MessageDetails* details = new MessageDetails(value, (Color_t)color);
		player->sendStatsMessage((MessageClasses)messageClass, text, position, details);
		delete details;
	}
	else
		player->sendTextMessage((MessageClasses)messageClass, text);

	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaDoPlayerSendChannelMessage(lua_State* L)
{
	//doPlayerSendChannelMessage(cid, author, message, MessageClasses, channel)
	uint16_t channelId = popNumber(L);
	uint32_t speakClass = popNumber(L);
	std::string text = popString(L), name = popString(L);

	ScriptEnviroment* env = getEnv();
	Player* player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	player->sendChannelMessage(name, text, (MessageClasses)speakClass, channelId);
	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaDoPlayerOpenChannel(lua_State* L)
{
	//doPlayerOpenChannel(cid, channelId)
	uint16_t channelId = popNumber(L);
	uint32_t cid = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(env->getPlayerByUID(cid))
	{
		lua_pushboolean(L, g_game.playerOpenChannel(cid, channelId));
		return 1;
	}

	errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
	lua_pushboolean(L, false);
	return 1;
}

int32_t LuaInterface::luaDoPlayerSendChannels(lua_State* L)
{
	//doPlayerSendChannels(cid[, list])
	ChannelsList channels;
	uint32_t params = lua_gettop(L);
	if(params > 1)
	{
		if(!lua_istable(L, -1))
		{
			errorEx("Channel list is not a table");
			lua_pushboolean(L, false);
			return 1;
		}

		lua_pushnil(L);
		while(lua_next(L, -2))
		{
			channels.push_back(std::make_pair((uint16_t)lua_tonumber(L, -2), lua_tostring(L, -1)));
			lua_pop(L, 1);
		}

		lua_pop(L, 1);
	}

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		if(params < 2)
			channels = g_chat.getChannelList(player);

		player->sendChannelsDialog(channels);
		player->setSentChat(params < 2);
		lua_pushboolean(L, true);
		return 1;
	}

	errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
	lua_pushboolean(L, false);
	return 1;
}

int32_t LuaInterface::luaDoSendCreatureSquare(lua_State* L)
{
	//doSendCreatureSquare(cid, color[, player])
	ScriptEnviroment* env = getEnv();
	SpectatorVec list;
	if(lua_gettop(L) > 2)
	{
		if(Creature* creature = env->getCreatureByUID(popNumber(L)))
			list.push_back(creature);
	}

	uint8_t color = popNumber(L);
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		if(!list.empty())
			g_game.addCreatureSquare(list, creature, color);
		else
			g_game.addCreatureSquare(creature, color);

		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoSendAnimatedText(lua_State* L)
{
	//doSendAnimatedText(pos, text, color[, player])
	ScriptEnviroment* env = getEnv();
	SpectatorVec list;
	if(lua_gettop(L) > 3)
	{
		if(Creature* creature = env->getCreatureByUID(popNumber(L)))
			list.push_back(creature);
	}

	uint8_t color = popNumber(L);
	std::string text = popString(L);

	PositionEx pos;
	popPosition(L, pos);
	if(pos.x == 0xFFFF)
		pos = env->getRealPos();

	if(!list.empty())
		g_game.addAnimatedText(list, pos, color, text);
	else
		g_game.addAnimatedText(pos, color, text);

	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaGetPlayerSkillLevel(lua_State* L)
{
	//getPlayerSkillLevel(cid, skill[, ignoreModifiers = false])
	bool ignoreModifiers = false;
	if(lua_gettop(L) > 2)
		ignoreModifiers = popBoolean(L);

	uint32_t skill = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(const Player* player = env->getPlayerByUID(popNumber(L)))
	{
		if(skill <= SKILL_LAST)
			lua_pushnumber(L, ignoreModifiers ? player->skills[skill][SKILL_LEVEL] :
				player->skills[skill][SKILL_LEVEL] + player->getVarSkill((skills_t)skill));
		else
			lua_pushboolean(L, false);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetPlayerSkillTries(lua_State* L)
{
	//getPlayerSkillTries(cid, skill)
	uint32_t skill = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(const Player* player = env->getPlayerByUID(popNumber(L)))
	{
		if(skill <= SKILL_LAST)
			lua_pushnumber(L, player->skills[skill][SKILL_TRIES]);
		else
			lua_pushboolean(L, false);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSetOfflineTrainingSkill(lua_State* L)
{
	//doPlayerSetOfflineTrainingSkill(cid, skillid)
	uint32_t skillid = (uint32_t)popNumber(L);
	uint32_t cid = popNumber(L);

	ScriptEnviroment* env = getEnv();

	Player* player = env->getPlayerByUID(cid);
	if(player)
	{
		player->setOfflineTrainingSkill(skillid);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoCreatureSetDropLoot(lua_State* L)
{
	//doCreatureSetDropLoot(cid, doDrop)
	bool doDrop = popBoolean(L);

	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		creature->setDropLoot(doDrop ? LOOT_DROP_FULL : LOOT_DROP_NONE);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetPlayerLossPercent(lua_State* L)
{
	//getPlayerLossPercent(cid, lossType)
	uint8_t lossType = (uint8_t)popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(const Player* player = env->getPlayerByUID(popNumber(L)))
	{
		if(lossType <= LOSS_LAST)
		{
			uint32_t value = player->getLossPercent((lossTypes_t)lossType);
			lua_pushnumber(L, value);
		}
		else
			lua_pushboolean(L, false);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSetLossPercent(lua_State* L)
{
	//doPlayerSetLossPercent(cid, lossType, newPercent)
	uint32_t newPercent = popNumber(L);
	uint8_t lossType = (uint8_t)popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		if(lossType <= LOSS_LAST)
		{
			player->setLossPercent((lossTypes_t)lossType, newPercent);
			lua_pushboolean(L, true);
		}
		else
			lua_pushboolean(L, false);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSetLossSkill(lua_State* L)
{
	//doPlayerSetLossSkill(cid, doLose)
	bool doLose = popBoolean(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->setLossSkill(doLose);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoShowTextDialog(lua_State* L)
{
	//doShowTextDialog(cid, itemid[, (text/canWrite)[, (canWrite/length)[, length]]])
	int32_t length = -1, params = lua_gettop(L);
	if(params > 4)
		length = std::abs(popNumber(L));

	bool canWrite = false;
	if(params > 3)
	{
		if(lua_isboolean(L, -1))
			canWrite = popBoolean(L);
		else
			length = popNumber(L);
	}

	std::string text;
	if(params > 2)
	{
		if(lua_isboolean(L, -1))
			canWrite = popBoolean(L);
		else
			text = popString(L);
	}

	uint32_t itemId = popNumber(L);
	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		Item* item = Item::CreateItem(itemId);
		if(length < 0)
			length = item->getMaxWriteLength();

		player->transferContainer.__addThing(NULL, item);
		if(text.size())
		{
			item->setText(text);
			length = std::max((int32_t)text.size(), length);
		}

		player->setWriteItem(item, length);
		player->transferContainer.setParent(player);

		player->sendTextWindow(item, length, canWrite);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoDecayItem(lua_State* L)
{
	//doDecayItem(uid)
	//Note: to stop decay set decayTo = 0 in items.xml
	ScriptEnviroment* env = getEnv();
	if(Item* item = env->getItemByUID(popNumber(L)))
	{
		g_game.startDecay(item);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_ITEM_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetThingFromPosition(lua_State* L)
{
	//getThingFromPosition(pos)
	// Note:
	//	stackpos = 255- top thing (movable item or creature)
	//	stackpos = 254- magic field
	//	stackpos = 253- top creature
	PositionEx pos;
	popPosition(L, pos);

	ScriptEnviroment* env = getEnv();
	Thing* thing = NULL;
	if(Tile* tile = g_game.getMap()->getTile(pos))
	{
		if(pos.stackpos == 255)
		{
			if(!(thing = tile->getTopCreature()))
			{
				Item* item = tile->getTopDownItem();
				if(item && item->isMovable())
					thing = item;
			}
		}
		else if(pos.stackpos == 254)
			thing = tile->getFieldItem();
		else if(pos.stackpos == 253)
			thing = tile->getTopCreature();
		else
			thing = tile->__getThing(pos.stackpos);

		if(thing)
			pushThing(L, thing, env->addThing(thing));
		else
			pushThing(L, NULL, 0);

		return 1;
	}

	errorEx(getError(LUA_ERROR_TILE_NOT_FOUND));
	pushThing(L, NULL, 0);
	return 1;
}

int32_t LuaInterface::luaGetTileItemById(lua_State* L)
{
	//getTileItemById(pos, itemId[, subType = -1])
	ScriptEnviroment* env = getEnv();

	int32_t subType = -1;
	if(lua_gettop(L) > 2)
		subType = (int32_t)popNumber(L);

	int32_t itemId = (int32_t)popNumber(L);
	PositionEx pos;
	popPosition(L, pos);

	Tile* tile = g_game.getTile(pos);
	if(!tile)
	{
		pushThing(L, NULL, 0);
		return 1;
	}

	Item* item = g_game.findItemOfType(tile, itemId, false, subType);
	if(!item)
	{
		pushThing(L, NULL, 0);
		return 1;
	}

	pushThing(L, item, env->addThing(item));
	return 1;
}

int32_t LuaInterface::luaGetTileItemByType(lua_State* L)
{
	//getTileItemByType(pos, type)
	uint32_t rType = (uint32_t)popNumber(L);
	if(rType >= ITEM_TYPE_LAST)
	{
		errorEx("Not a valid item type");
		pushThing(L, NULL, 0);
		return 1;
	}

	PositionEx pos;
	popPosition(L, pos);

	Tile* tile = g_game.getTile(pos);
	if(!tile)
	{
		pushThing(L, NULL, 0);
		return 1;
	}

	bool found = true;
	switch((ItemTypes_t)rType)
	{
		case ITEM_TYPE_TELEPORT:
		{
			if(!tile->hasFlag(TILESTATE_TELEPORT))
				found = false;

			break;
		}
		case ITEM_TYPE_MAGICFIELD:
		{
			if(!tile->hasFlag(TILESTATE_MAGICFIELD))
				found = false;

			break;
		}
		case ITEM_TYPE_MAILBOX:
		{
			if(!tile->hasFlag(TILESTATE_MAILBOX))
				found = false;

			break;
		}
		case ITEM_TYPE_TRASHHOLDER:
		{
			if(!tile->hasFlag(TILESTATE_TRASHHOLDER))
				found = false;

			break;
		}
		case ITEM_TYPE_BED:
		{
			if(!tile->hasFlag(TILESTATE_BED))
				found = false;

			break;
		}
		case ITEM_TYPE_DEPOT:
		{
			if(!tile->hasFlag(TILESTATE_DEPOT))
				found = false;

			break;
		}
		default:
			break;
	}

	if(!found)
	{
		pushThing(L, NULL, 0);
		return 1;
	}

	ScriptEnviroment* env = getEnv();
	if(TileItemVector* items = tile->getItemList())
	{
		for(ItemVector::iterator it = items->begin(); it != items->end(); ++it)
		{
			if(Item::items[(*it)->getID()].type != (ItemTypes_t)rType)
				continue;

			pushThing(L, *it, env->addThing(*it));
			return 1;
		}
	}

	pushThing(L, NULL, 0);
	return 1;
}

int32_t LuaInterface::luaGetTileThingByPos(lua_State* L)
{
	//getTileThingByPos(pos)
	PositionEx pos;
	popPosition(L, pos);
	ScriptEnviroment* env = getEnv();

	Tile* tile = g_game.getTile(pos.x, pos.y, pos.z);
	if(!tile)
	{
		if(pos.stackpos == -1)
		{
			lua_pushnumber(L, -1);
			return 1;
		}
		else
		{
			pushThing(L, NULL, 0);
			return 1;
		}
	}

	if(pos.stackpos == -1)
	{
		lua_pushnumber(L, tile->getThingCount());
		return 1;
	}

	Thing* thing = tile->__getThing(pos.stackpos);
	if(!thing)
	{
		pushThing(L, NULL, 0);
		return 1;
	}

	pushThing(L, thing, env->addThing(thing));
	return 1;
}

int32_t LuaInterface::luaGetTopCreature(lua_State* L)
{
	//getTopCreature(pos)
	PositionEx pos;
	popPosition(L, pos);

	ScriptEnviroment* env = getEnv();
	Tile* tile = g_game.getTile(pos);
	if(!tile)
	{
		pushThing(L, NULL, 0);
		return 1;
	}

	Thing* thing = tile->getTopCreature();
	if(!thing || !thing->getCreature())
	{
		pushThing(L, NULL, 0);
		return 1;
	}

	pushThing(L, thing, env->addThing(thing));
	return 1;
}

int32_t LuaInterface::luaDoCreateItem(lua_State* L)
{
	//doCreateItem(itemid[, type/count = 1], pos)
	//Returns uid of the created item, only works on tiles.
	PositionEx pos;
	popPosition(L, pos);

	uint32_t count = 1;
	if(lua_gettop(L) > 1)
		count = popNumber(L);

	const ItemType& it = Item::items[popNumber(L)];
	ScriptEnviroment* env = getEnv();

	Tile* tile = g_game.getTile(pos);
	if(!tile)
	{
		if(it.group == ITEM_GROUP_GROUND)
		{
			Item* item = Item::CreateItem(it.id);
			tile = IOMap::createTile(item, NULL, pos.x, pos.y, pos.z);

			g_game.setTile(tile);
			lua_pushnumber(L, env->addThing(item));
		}
		else
		{
			errorEx(getError(LUA_ERROR_TILE_NOT_FOUND));
			lua_pushboolean(L, false);
		}

		return 1;
	}

	int32_t itemCount = 1, subType = 1;
	if(it.hasSubType())
	{
		if(it.stackable)
			itemCount = (int32_t)std::ceil(count / 100.);

		subType = count;
	}
	else
		itemCount = std::max(1U, count);

	uint32_t ret = 0;
	Item* newItem = NULL;
	while(itemCount > 0)
	{
		int32_t stackCount = std::min(100, subType);
		if(!(newItem = Item::CreateItem(it.id, stackCount)))
		{
			errorEx(getError(LUA_ERROR_ITEM_NOT_FOUND));
			lua_pushboolean(L, false);
			return ++ret;
		}

		if(it.stackable)
			subType -= stackCount;

		uint32_t dummy = 0;
		Item* stackItem = NULL;
		if(g_game.internalAddItem(NULL, tile, newItem, INDEX_WHEREEVER, FLAG_NOLIMIT, false, dummy, &stackItem) != RET_NOERROR)
		{
			delete newItem;
			lua_pushboolean(L, false);
			return ++ret;
		}

		++ret;
		if(newItem->getParent())
			lua_pushnumber(L, env->addThing(newItem));
		else if(stackItem)
			lua_pushnumber(L, env->addThing(stackItem));
		else
			lua_pushnil(L);

		--itemCount;
	}

	if(ret)
		return ret;

	lua_pushnil(L);
	return 1;
}

int32_t LuaInterface::luaDoCreateItemEx(lua_State* L)
{
	//doCreateItemEx(itemid[, count/subType])
	uint32_t count = 0;
	if(lua_gettop(L) > 1)
		count = popNumber(L);

	ScriptEnviroment* env = getEnv();
	const ItemType& it = Item::items[(uint32_t)popNumber(L)];
	if(it.stackable && count > 100)
		count = 100;

	Item* newItem = Item::CreateItem(it.id, count);
	if(!newItem)
	{
		errorEx(getError(LUA_ERROR_ITEM_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	newItem->setParent(VirtualCylinder::virtualCylinder);
	env->addTempItem(env, newItem);

	lua_pushnumber(L, env->addThing(newItem));
	return 1;
}

int32_t LuaInterface::luaDoCreateTeleport(lua_State* L)
{
	//doCreateTeleport(itemid, destination, position)
	PositionEx position;
	popPosition(L, position);
	PositionEx destination;
	popPosition(L, destination);

	uint32_t itemId = (uint32_t)popNumber(L);
	ScriptEnviroment* env = getEnv();

	Tile* tile = g_game.getMap()->getTile(position);
	if(!tile)
	{
		errorEx(getError(LUA_ERROR_TILE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	Item* newItem = Item::CreateItem(itemId);
	Teleport* newTeleport = newItem->getTeleport();
	if(!newTeleport)
	{
		delete newItem;
		errorEx("Item " + asString(itemId) + " is not a teleport.");
		lua_pushboolean(L, false);
		return 1;
	}

	uint32_t dummy = 0;
	Item* stackItem = NULL;
	if(g_game.internalAddItem(NULL, tile, newItem, INDEX_WHEREEVER, FLAG_NOLIMIT, false, dummy, &stackItem) != RET_NOERROR)
	{
		delete newItem;
		lua_pushboolean(L, false);
		return 1;
	}

	newTeleport->setDestination(destination);
	if(newItem->getParent())
		lua_pushnumber(L, env->addThing(newItem));
	else if(stackItem)
		lua_pushnumber(L, env->addThing(stackItem));
	else
		lua_pushnil(L);

	return 1;
}

int32_t LuaInterface::luaGetCreatureStorageList(lua_State* L)
{
	//getCreatureStorageList(cid)
	ScriptEnviroment* env = getEnv();

	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		StorageMap::const_iterator it = creature->getStorageBegin();
		lua_newtable(L);
		for(uint32_t i = 1; it != creature->getStorageEnd(); ++i, ++it)
		{
			lua_pushnumber(L, i);
			lua_pushstring(L, it->first.c_str());
			pushTable(L);
		}
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetCreatureStorage(lua_State* L)
{
	//getCreatureStorage(cid, key)
	std::string key = popString(L);
	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		std::string strValue;
		if(!creature->getStorage(key, strValue))
		{
			lua_pushnumber(L, -1);
			lua_pushnil(L);
			return 2;
		}

		int32_t intValue = atoi(strValue.c_str());
		if(intValue || strValue == "0")
			lua_pushnumber(L, intValue);
		else
			lua_pushstring(L, strValue.c_str());
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoCreatureSetStorage(lua_State* L)
{
	//doCreatureSetStorage(cid, key[, value])
	std::string value;
	bool tmp = true;
	if(lua_gettop(L) > 2)
	{
		if(!lua_isnil(L, -1))
		{
			value = popString(L);
			tmp = false;
		}
		else
			lua_pop(L, 1);
	}

	std::string key = popString(L);
	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		if(!tmp)
			tmp = creature->setStorage(key, value);
		else
			creature->eraseStorage(key);

		lua_pushboolean(L, tmp);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetPlayerSpectators(lua_State* L)
{
	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		lua_newtable(L);
		setFieldBool(L, "broadcast", player->client->isBroadcasting());
		setField(L, "password", player->client->getPassword());
		setFieldBool(L, "auth", player->client->isAuth());

		createTable(L, "names");
		StringVec t = player->client->list();

		StringVec::const_iterator it = t.begin();
		for(uint32_t i = 1; it != t.end(); ++it, ++i)
		{
			lua_pushnumber(L, i);
			lua_pushstring(L, (*it).c_str());
			pushTable(L);
		}

		pushTable(L);
		createTable(L, "mutes");
		t = player->client->muteList();

		it = t.begin();
		for(uint32_t i = 1; it != t.end(); ++it, ++i)
		{
			lua_pushnumber(L, i);
			lua_pushstring(L, (*it).c_str());
			pushTable(L);
		}

		pushTable(L);
		createTable(L, "bans");
		std::map<std::string, uint32_t> _t = player->client->banList();

		std::map<std::string, uint32_t>::const_iterator _it = _t.begin();
		for(uint32_t i = 1; _it != _t.end(); ++_it, ++i)
		{
			lua_pushnumber(L, i);
			lua_pushstring(L, _it->first.c_str());
			pushTable(L);
		}

		pushTable(L);
		createTable(L, "kick");
		pushTable(L);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSetSpectators(lua_State* L)
{
	std::string password = getFieldString(L, "password");
	bool broadcast = getFieldBool(L, "broadcast"),
		auth = getFieldBool(L, "auth");

	StringVec m, b, k;
	lua_pushstring(L, "mutes");
	lua_gettable(L, -2);

	lua_pushnil(L);
	while(lua_next(L, -2))
	{
		m.push_back(asLowerCaseString(lua_tostring(L, -1)));
		lua_pop(L, 1);
	}

	lua_pop(L, 1);
	lua_pushstring(L, "bans");
	lua_gettable(L, -2);

	lua_pushnil(L);
	while(lua_next(L, -2))
	{
		b.push_back(asLowerCaseString(lua_tostring(L, -1)));
		lua_pop(L, 1);
	}

	lua_pop(L, 1);
	lua_pushstring(L, "kick");
	lua_gettable(L, -2);

	lua_pushnil(L);
	while(lua_next(L, -2))
	{
		k.push_back(asLowerCaseString(lua_tostring(L, -1)));
		lua_pop(L, 1);
	}

	lua_pop(L, 2);
	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		if(player->client->getPassword() != password && !password.empty())
			player->client->clear(false);

		player->client->setPassword(password);
		if(!broadcast && player->client->isBroadcasting())
			player->client->clear(false);

		player->client->kick(k);
		player->client->mute(m);
		player->client->ban(b);

		player->client->setBroadcast(broadcast);
		player->client->setAuth(auth);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetTileInfo(lua_State* L)
{
	//getTileInfo(pos)
	PositionEx pos;
	popPosition(L, pos);
	if(Tile* tile = g_game.getMap()->getTile(pos))
	{
		ScriptEnviroment* env = getEnv();
		pushThing(L, tile->ground, env->addThing(tile->ground));

		setFieldBool(L, "protection", tile->hasFlag(TILESTATE_PROTECTIONZONE));
		setFieldBool(L, "optional", tile->hasFlag(TILESTATE_OPTIONALZONE));
		setFieldBool(L, "hardcore", tile->hasFlag(TILESTATE_HARDCOREZONE));
		setFieldBool(L, "noLogout", tile->hasFlag(TILESTATE_NOLOGOUT));
		setFieldBool(L, "refresh", tile->hasFlag(TILESTATE_REFRESH));
		setFieldBool(L, "trashed", tile->hasFlag(TILESTATE_TRASHED));
		setFieldBool(L, "magicField", tile->hasFlag(TILESTATE_MAGICFIELD));
		setFieldBool(L, "trashHolder", tile->hasFlag(TILESTATE_TRASHHOLDER));
		setFieldBool(L, "mailbox", tile->hasFlag(TILESTATE_MAILBOX));
		setFieldBool(L, "depot", tile->hasFlag(TILESTATE_DEPOT));
		setFieldBool(L, "bed", tile->hasFlag(TILESTATE_BED));

		createTable(L, "floorChange");
		for(int32_t i = CHANGE_FIRST; i <= CHANGE_LAST; ++i)
		{
			lua_pushnumber(L, i);
			lua_pushboolean(L, tile->floorChange((FloorChange_t)i));
			pushTable(L);
		}

		pushTable(L);
		setFieldBool(L, "teleport", tile->hasFlag(TILESTATE_TELEPORT));

		setField(L, "things", tile->getThingCount());
		setField(L, "creatures", tile->getCreatureCount());
		setField(L, "items", tile->getItemCount());
		setField(L, "topItems", tile->getTopItemCount());
		setField(L, "downItems", tile->getDownItemCount());
		if(House* house = tile->getHouse())
			setField(L, "house", house->getId());
	}
	else
	{
		errorEx(getError(LUA_ERROR_TILE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetHouseFromPosition(lua_State* L)
{
	//getHouseFromPosition(pos)
	PositionEx pos;
	popPosition(L, pos);

	Tile* tile = g_game.getMap()->getTile(pos);
	if(!tile)
	{
		errorEx(getError(LUA_ERROR_TILE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	HouseTile* houseTile = tile->getHouseTile();
	if(!houseTile)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	House* house = houseTile->getHouse();
	if(!house)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	lua_pushnumber(L, house->getId());
	return 1;
}

int32_t LuaInterface::luaDoCreateMonster(lua_State* L)
{
	//doCreateMonster(name, pos[, extend = false[, force = false]])
	bool force = false, extend = false;
	int32_t params = lua_gettop(L);
	if(params > 3)
		force = popBoolean(L);

	if(params > 2)
		extend = popBoolean(L);

	PositionEx pos;
	popPosition(L, pos);

	std::string name = popString(L);
	Monster* monster = Monster::createMonster(name.c_str());
	if(!monster)
	{
		errorEx("Monster with name '" + name + "' not found");
		lua_pushboolean(L, false);
		return 1;
	}

	if(!g_game.placeCreature(monster, pos, extend, force))
	{
		delete monster;
		errorEx("Cannot create monster: " + name);

		lua_pushboolean(L, false);
		return 1;
	}

	ScriptEnviroment* env = getEnv();
	lua_pushnumber(L, env->addThing((Thing*)monster));
	return 1;
}

int32_t LuaInterface::luaDoCreateNpc(lua_State* L)
{
	//doCreateNpc(name, pos)
	PositionEx pos;
	popPosition(L, pos);
	std::string name = popString(L);

	Npc* npc = Npc::createNpc(name.c_str());
	if(!npc)
	{
		errorEx("Npc with name '" + name + "' not found");
		lua_pushboolean(L, false);
		return 1;
	}

	if(!g_game.placeCreature(npc, pos))
	{
		delete npc;
		errorEx("Cannot create npc: " + name);

		lua_pushboolean(L, true); //for scripting compatibility
		return 1;
	}

	ScriptEnviroment* env = getEnv();
	lua_pushnumber(L, env->addThing((Thing*)npc));
	return 1;
}

int32_t LuaInterface::luaDoRemoveCreature(lua_State* L)
{
	//doRemoveCreature(cid[, forceLogout = true])
	bool forceLogout = true;
	if(lua_gettop(L) > 1)
		forceLogout = popBoolean(L);

	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		if(Player* player = creature->getPlayer())
			player->kick(true, forceLogout); //Players will get kicked without restrictions
		else
			g_game.removeCreature(creature); //Monsters/NPCs will get removed

		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerAddMoney(lua_State* L)
{
	//doPlayerAddMoney(cid, money)
	uint64_t money = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		g_game.addMoney(player, money);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerRemoveMoney(lua_State* L)
{
	//doPlayerRemoveMoney(cid,money)
	uint64_t money = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
		lua_pushboolean(L, g_game.removeMoney(player, money));
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerTransferMoneyTo(lua_State* L)
{
	//doPlayerTransferMoneyTo(cid, target, money)
	uint64_t money = popNumber(L);
	std::string target = popString(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
		lua_pushboolean(L, player->transferMoneyTo(target, money));
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSetPzLocked(lua_State* L)
{
	//doPlayerSetPzLocked(cid, locked)
	bool locked = popBoolean(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		if(player->isPzLocked() != locked)
		{
			player->setPzLocked(locked);
			player->sendIcons();
		}

		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSetTown(lua_State* L)
{
	//doPlayerSetTown(cid, townid)
	uint32_t townid = (uint32_t)popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		if(Town* town = Towns::getInstance()->getTown(townid))
		{
			player->setMasterPosition(town->getPosition());
			player->setTown(townid);
			lua_pushboolean(L, true);
		}
		else
			lua_pushboolean(L, false);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSetVocation(lua_State* L)
{
	//doPlayerSetVocation(cid, voc)
	uint32_t voc = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->setVocation(voc);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSetSex(lua_State* L)
{
	//doPlayerSetSex(cid, sex)
	uint32_t newSex = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->setSex(newSex);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerAddSoul(lua_State* L)
{
	//doPlayerAddSoul(cid, amount)
	int32_t amount = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->changeSoul(amount);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetPlayerItemCount(lua_State* L)
{
	//getPlayerItemCount(cid, itemid[, subType = -1])
	int32_t subType = -1;
	if(lua_gettop(L) > 2)
		subType = popNumber(L);

	uint32_t itemId = popNumber(L);
	ScriptEnviroment* env = getEnv();
	if(const Player* player = env->getPlayerByUID(popNumber(L)))
		lua_pushnumber(L, player->__getItemTypeCount(itemId, subType));
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetHouseInfo(lua_State* L)
{
	//getHouseInfo(houseId[, full = true])
	bool full = true;
	if(lua_gettop(L) > 1)
		full = popBoolean(L);

	House* house = Houses::getInstance()->getHouse(popNumber(L));
	if(!house)
	{
		errorEx(getError(LUA_ERROR_HOUSE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	lua_newtable(L);
	setField(L, "id", house->getId());
	setField(L, "name", house->getName().c_str());
	setField(L, "owner", house->getOwner());

	lua_pushstring(L, "entry");
	pushPosition(L, house->getEntry(), 0);
	pushTable(L);

	setField(L, "rent", house->getRent());
	setField(L, "price", house->getPrice());
	setField(L, "town", house->getTownId());
	setField(L, "paidUntil", house->getPaidUntil());
	setField(L, "warnings", house->getRentWarnings());
	setField(L, "lastWarning", house->getLastWarning());

	setFieldBool(L, "guildHall", house->isGuild());
	setField(L, "size", house->getSize());

	if(full)
	{
		createTable(L, "doors");

		HouseDoorList::iterator dit = house->getHouseDoorBegin();
		for(uint32_t i = 1; dit != house->getHouseDoorEnd(); ++dit, ++i)
		{
			lua_pushnumber(L, i);
			pushPosition(L, (*dit)->getPosition(), 0);
			pushTable(L);
		}

		pushTable(L);
		createTable(L, "beds");

		HouseBedList::iterator bit = house->getHouseBedsBegin();
		for(uint32_t i = 1; bit != house->getHouseBedsEnd(); ++bit, ++i)
		{
			lua_pushnumber(L, i);
			pushPosition(L, (*bit)->getPosition(), 0);
			pushTable(L);
		}

		pushTable(L);
		createTable(L, "tiles");

		HouseTileList::iterator tit = house->getHouseTileBegin();
		for(uint32_t i = 1; tit != house->getHouseTileEnd(); ++tit, ++i)
		{
			lua_pushnumber(L, i);
			pushPosition(L, (*tit)->getPosition(), 0);
			pushTable(L);
		}

		pushTable(L);
	}

	return 1;
}

int32_t LuaInterface::luaGetHouseAccessList(lua_State* L)
{
	//getHouseAccessList(houseid, listid)
	uint32_t listid = popNumber(L);
	if(House* house = Houses::getInstance()->getHouse(popNumber(L)))
	{
		std::string list;
		if(house->getAccessList(listid, list))
			lua_pushstring(L, list.c_str());
		else
			lua_pushnil(L);
	}
	else
	{
		errorEx(getError(LUA_ERROR_HOUSE_NOT_FOUND));
		lua_pushnil(L);
	}

	return 1;
}

int32_t LuaInterface::luaGetHouseByPlayerGUID(lua_State* L)
{
	//getHouseByPlayerGUID(guid)
	if(House* house = Houses::getInstance()->getHouseByPlayerId(popNumber(L)))
		lua_pushnumber(L, house->getId());
	else
		lua_pushnil(L);
	return 1;
}

int32_t LuaInterface::luaSetHouseAccessList(lua_State* L)
{
	//setHouseAccessList(houseid, listid, listtext)
	std::string list = popString(L);
	uint32_t listid = popNumber(L);

	if(House* house = Houses::getInstance()->getHouse(popNumber(L)))
	{
		house->setAccessList(listid, list);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_HOUSE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaSetHouseOwner(lua_State* L)
{
	//setHouseOwner(houseId, owner[, clean = true])
	bool clean = true;
	if(lua_gettop(L) > 2)
		clean = popBoolean(L);

	uint32_t owner = popNumber(L);
	if(House* house = Houses::getInstance()->getHouse(popNumber(L)))
		lua_pushboolean(L, house->setOwnerEx(owner, clean));
	else
	{
		errorEx(getError(LUA_ERROR_HOUSE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetWorldType(lua_State* L)
{
	lua_pushnumber(L, (uint32_t)g_game.getWorldType());
	return 1;
}

int32_t LuaInterface::luaSetWorldType(lua_State* L)
{
	//setWorldType(type)
	WorldType_t type = (WorldType_t)popNumber(L);
	if(type >= WORLDTYPE_FIRST && type <= WORLDTYPE_LAST)
	{
		g_game.setWorldType(type);
		lua_pushboolean(L, true);
	}
	else
		lua_pushboolean(L, false);

	return 1;
}

int32_t LuaInterface::luaGetWorldTime(lua_State* L)
{
	//getWorldTime()
	lua_pushnumber(L, g_game.getLightHour());
	return 1;
}

int32_t LuaInterface::luaGetWorldLight(lua_State* L)
{
	//getWorldLight()
	LightInfo lightInfo;
	g_game.getWorldLightInfo(lightInfo);

	lua_pushnumber(L, lightInfo.level);
	lua_pushnumber(L, lightInfo.color);
	return 2;
}

int32_t LuaInterface::luaGetWorldCreatures(lua_State* L)
{
	//getWorldCreatures(type)
	//0 players, 1 monsters, 2 npcs, 3 all
	uint32_t type = popNumber(L), value;
	switch(type)
	{
		case 0:
			value = g_game.getPlayersOnline();
			break;
		case 1:
			value = g_game.getMonstersOnline();
			break;
		case 2:
			value = g_game.getNpcsOnline();
			break;
		case 3:
			value = g_game.getCreaturesOnline();
			break;
		default:
			lua_pushboolean(L, false);
			return 1;
	}

	lua_pushnumber(L, value);
	return 1;
}

int32_t LuaInterface::luaGetWorldUpTime(lua_State* L)
{
	//getWorldUpTime()
	uint32_t uptime = 0;
	if(Status* status = Status::getInstance())
		uptime = status->getUptime();

	lua_pushnumber(L, uptime);
	return 1;
}

int32_t LuaInterface::luaGetPlayerLight(lua_State* L)
{
	//getPlayerLight(cid)
	ScriptEnviroment* env = getEnv();
	if(const Player* player = env->getPlayerByUID(popNumber(L)))
	{
		LightInfo lightInfo;
		player->getCreatureLight(lightInfo);

		lua_pushnumber(L, lightInfo.level);
		lua_pushnumber(L, lightInfo.color);
		return 2;
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}
}

int32_t LuaInterface::luaGetPlayerSoul(lua_State* L)
{
	//getPlayerSoul(cid[, ignoreModifiers = false])
	bool ignoreModifiers = false;
	if(lua_gettop(L) > 1)
		ignoreModifiers = popBoolean(L);

	ScriptEnviroment* env = getEnv();
	if(const Player* player = env->getPlayerByUID(popNumber(L)))
		lua_pushnumber(L, ignoreModifiers ? player->soul : player->getSoul());
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerAddExperience(lua_State* L)
{
	//doPlayerAddExperience(cid, amount)
	int64_t amount = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		if(amount > 0)
			player->addExperience(amount);
		else if(amount < 0)
			player->removeExperience(std::abs(amount));
		else
		{
			lua_pushboolean(L, false);
			return 1;
		}

		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetPlayerSlotItem(lua_State* L)
{
	//getPlayerSlotItem(cid, slot)
	uint32_t slot = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(const Player* player = env->getPlayerByUID(popNumber(L)))
	{
		if(Thing* thing = player->__getThing(slot))
			pushThing(L, thing, env->addThing(thing));
		else
			pushThing(L, NULL, 0);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		pushThing(L, NULL, 0);
	}

	return 1;
}

int32_t LuaInterface::luaGetPlayerWeapon(lua_State* L)
{
	//getPlayerWeapon(cid[, ignoreAmmo = false])
	bool ignoreAmmo = false;
	if(lua_gettop(L) > 1)
		ignoreAmmo = popBoolean(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		if(Item* weapon = player->getWeapon(ignoreAmmo))
			pushThing(L, weapon, env->addThing(weapon));
		else
			pushThing(L, NULL, 0);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushnil(L);
	}

	return 1;
}

int32_t LuaInterface::luaGetPlayerItemById(lua_State* L)
{
	//getPlayerItemById(cid, deepSearch, itemId[, subType = -1])
	ScriptEnviroment* env = getEnv();

	int32_t subType = -1;
	if(lua_gettop(L) > 3)
		subType = (int32_t)popNumber(L);

	int32_t itemId = (int32_t)popNumber(L);
	bool deepSearch = popBoolean(L);

	Player* player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		pushThing(L, NULL, 0);
		return 1;
	}

	Item* item = g_game.findItemOfType(player, itemId, deepSearch, subType);
	if(!item)
	{
		pushThing(L, NULL, 0);
		return 1;
	}

	pushThing(L, item, env->addThing(item));
	return 1;
}

int32_t LuaInterface::luaGetThing(lua_State* L)
{
	//getThing(uid[, recursive = RECURSE_FIRST])
	Recursive_t recursive = RECURSE_FIRST;
	if(lua_gettop(L) > 1)
		recursive = (Recursive_t)popNumber(L);

	uint32_t uid = popNumber(L);
	ScriptEnviroment* env = getEnv();
	if(Thing* thing = env->getThingByUID(uid))
		pushThing(L, thing, uid, recursive);
	else
	{
		errorEx(getError(LUA_ERROR_THING_NOT_FOUND));
		pushThing(L, NULL, 0);
	}

	return 1;
}

int32_t LuaInterface::luaDoTileQueryAdd(lua_State* L)
{
	//doTileQueryAdd(uid, pos[, flags])
	uint32_t flags = 0, params = lua_gettop(L);
	if(params > 2)
		flags = popNumber(L);

	PositionEx pos;
	popPosition(L, pos);
	uint32_t uid = popNumber(L);

	ScriptEnviroment* env = getEnv();
	Tile* tile = g_game.getTile(pos);
	if(!tile)
	{
		errorEx(getError(LUA_ERROR_TILE_NOT_FOUND));
		lua_pushnumber(L, (uint32_t)RET_NOTPOSSIBLE);
		return 1;
	}

	Thing* thing = env->getThingByUID(uid);
	if(!thing)
	{
		errorEx(getError(LUA_ERROR_THING_NOT_FOUND));
		lua_pushnumber(L, (uint32_t)RET_NOTPOSSIBLE);
		return 1;
	}

	lua_pushnumber(L, (uint32_t)tile->__queryAdd(0, thing, 1, flags));
	return 1;
}

int32_t LuaInterface::luaDoItemRaidUnref(lua_State* L)
{
	//doItemRaidUnref(uid)
	ScriptEnviroment* env = getEnv();
	if(Item* item = env->getItemByUID(popNumber(L)))
	{
		if(Raid* raid = item->getRaid())
		{
			raid->unRef();
			item->setRaid(NULL);
			lua_pushboolean(L, true);
		}
		else
			lua_pushboolean(L, false);
	}
	else
	{
		errorEx(getError(LUA_ERROR_ITEM_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetThingPosition(lua_State* L)
{
	//getThingPosition(uid)
	ScriptEnviroment* env = getEnv();
	if(Thing* thing = env->getThingByUID(popNumber(L)))
	{
		Position pos = thing->getPosition();
		uint32_t stackpos = 0;
		if(Tile* tile = thing->getTile())
			stackpos = tile->__getIndexOfThing(thing);

		pushPosition(L, pos, stackpos);
	}
	else
	{
		errorEx(getError(LUA_ERROR_THING_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaCreateCombatObject(lua_State* L)
{
	//createCombatObject()
	ScriptEnviroment* env = getEnv();
	if(env->getScriptId() != EVENT_ID_LOADING)
	{
		errorEx("This function can only be used while loading the script");
		lua_pushboolean(L, false);
		return 1;
	}

	Combat* combat = new Combat;
	if(!combat)
	{
		errorEx(getError(LUA_ERROR_COMBAT_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	lua_pushnumber(L, env->addCombatObject(combat));
	return 1;
}

bool LuaInterface::getArea(lua_State* L, std::list<uint32_t>& list, uint32_t& rows)
{
	rows = 0;
	if(!lua_istable(L, -1))
	{
		errorEx("Object on the stack is not a table");
		return false;
	}

	lua_pushnil(L);
	while(lua_next(L, -2))
	{
		lua_pushnil(L);
		while(lua_next(L, -2))
		{
			list.push_back((uint32_t)lua_tonumber(L, -1));
			lua_pop(L, 1);
		}

		lua_pop(L, 1);
		++rows;
	}

	lua_pop(L, 1);
	return (rows != 0);
}

int32_t LuaInterface::luaCreateCombatArea(lua_State* L)
{
	//createCombatArea({area}[, {extArea}])
	ScriptEnviroment* env = getEnv();
	if(env->getScriptId() != EVENT_ID_LOADING)
	{
		errorEx("This function can only be used while loading the script");
		lua_pushboolean(L, false);
		return 1;
	}

	CombatArea* area = new CombatArea;
	if(lua_gettop(L) > 1) //has extra parameter with diagonal area information
	{
		uint32_t rowsExtArea = 0;
		std::list<uint32_t> listExtArea;
		if(getArea(L, listExtArea, rowsExtArea))
			area->setupExtArea(listExtArea, rowsExtArea);
	}

	uint32_t rowsArea = 0;
	std::list<uint32_t> listArea;
	if(getArea(L, listArea, rowsArea))
	{
		area->setupArea(listArea, rowsArea);
		lua_pushnumber(L, env->addCombatArea(area));
	}
	else
		lua_pushboolean(L, false);

	return 1;
}

int32_t LuaInterface::luaCreateConditionObject(lua_State* L)
{
	//createConditionObject(type[, ticks = 0[, buff = false[, subId = 0[, conditionId = CONDITIONID_COMBAT]]]])
	int32_t conditionId = CONDITIONID_COMBAT;
	uint32_t params = lua_gettop(L), subId = 0;
	if(params > 4)
		conditionId = popNumber(L);

	if(params > 3)
		subId = popNumber(L);

	bool buff = false;
	if(params > 2)
		buff = popBoolean(L);

	int32_t ticks = 0;
	if(params > 1)
		ticks = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Condition* condition = Condition::createCondition((ConditionId_t)conditionId, (ConditionType_t)popNumber(L), ticks, 0, buff, subId))
	{
		if(env->getScriptId() != EVENT_ID_LOADING)
			lua_pushnumber(L, env->addTempConditionObject(condition));
		else
			lua_pushnumber(L, env->addConditionObject(condition));
	}
	else
	{
		errorEx(getError(LUA_ERROR_CONDITION_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaSetCombatArea(lua_State* L)
{
	//setCombatArea(combat, area)
	uint32_t areaId = popNumber(L);
	ScriptEnviroment* env = getEnv();
	if(env->getScriptId() != EVENT_ID_LOADING)
	{
		errorEx("This function can only be used while loading the script");
		lua_pushboolean(L, false);
		return 1;
	}

	Combat* combat = env->getCombatObject(popNumber(L));
	if(!combat)
	{
		errorEx(getError(LUA_ERROR_COMBAT_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	const CombatArea* area = env->getCombatArea(areaId);
	if(!area)
	{
		errorEx(getError(LUA_ERROR_AREA_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	combat->setArea(new CombatArea(*area));
	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaSetCombatCondition(lua_State* L)
{
	//setCombatCondition(combat, condition[, loaded])
	bool loaded = true;
	if(lua_gettop(L) > 2)
		loaded = popBoolean(L);

	uint32_t conditionId = popNumber(L);
	ScriptEnviroment* env = getEnv();

	Combat* combat = env->getCombatObject(popNumber(L));
	if(!combat)
	{
		errorEx(getError(LUA_ERROR_COMBAT_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	const Condition* condition = env->getConditionObject(conditionId, loaded);
	if(!condition)
	{
		errorEx(getError(LUA_ERROR_CONDITION_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	combat->setCondition(condition->clone());
	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaSetCombatParam(lua_State* L)
{
	//setCombatParam(combat, key, value)
	uint32_t value = popNumber(L);
	CombatParam_t key = (CombatParam_t)popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(env->getScriptId() != EVENT_ID_LOADING)
	{
		errorEx("This function can only be used while loading the script");
		lua_pushboolean(L, false);
		return 1;
	}

	Combat* combat = env->getCombatObject(popNumber(L));
	if(!combat)
	{
		errorEx(getError(LUA_ERROR_COMBAT_NOT_FOUND));
		lua_pushboolean(L, false);
	}
	else
	{
		combat->setParam(key, value);
		lua_pushboolean(L, true);
	}

	return 1;
}

int32_t LuaInterface::luaSetConditionParam(lua_State* L)
{
	//setConditionParam(condition, key, value[, loaded])
	bool loaded = true;
	if(lua_gettop(L) > 3)
		loaded = popBoolean(L);

	int32_t value = popNumber(L);
	ScriptEnviroment* env = getEnv();

	ConditionParam_t key = (ConditionParam_t)popNumber(L);
	if(Condition* condition = env->getConditionObject(popNumber(L), loaded))
	{
		condition->setParam(key, value);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CONDITION_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaAddDamageCondition(lua_State* L)
{
	//addDamageCondition(condition, rounds, time, value[, loaded])
	bool loaded = true;
	if(lua_gettop(L) > 4)
		loaded = popBoolean(L);

	int32_t value = popNumber(L), time = popNumber(L), rounds = popNumber(L);
	ScriptEnviroment* env = getEnv();
	if(ConditionDamage* condition = dynamic_cast<ConditionDamage*>(env->getConditionObject(popNumber(L), loaded)))
	{
		condition->addDamage(rounds, time, value);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CONDITION_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaAddOutfitCondition(lua_State* L)
{
	//addOutfitCondition(condition, outfit[, loaded])
	bool loaded = true;
	if(lua_gettop(L) > 2)
		loaded = popBoolean(L);

	Outfit_t outfit = popOutfit(L);
	ScriptEnviroment* env = getEnv();
	if(ConditionOutfit* condition = dynamic_cast<ConditionOutfit*>(env->getConditionObject(popNumber(L), loaded)))
	{
		condition->addOutfit(outfit);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CONDITION_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaSetCombatCallBack(lua_State* L)
{
	//setCombatCallBack(combat, key, functionName)
	std::string function = popString(L);
	CallBackParam_t key = (CallBackParam_t)popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(env->getScriptId() != EVENT_ID_LOADING)
	{
		errorEx("This function can only be used while loading the script");
		lua_pushboolean(L, false);
		return 1;
	}

	Combat* combat = env->getCombatObject(popNumber(L));
	if(!combat)
	{
		errorEx(getError(LUA_ERROR_COMBAT_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	LuaInterface* interface = env->getInterface();
	combat->setCallback(key);

	CallBack* callback = combat->getCallback(key);
	if(!callback)
	{
		errorEx(asString<uint32_t>(key) + " is not a valid callback key");
		lua_pushboolean(L, false);
		return 1;
	}

	if(!callback->loadCallBack(interface, function))
	{
		errorEx("Cannot load callback");
		lua_pushboolean(L, false);
	}
	else
		lua_pushboolean(L, true);

	return 1;
}

int32_t LuaInterface::luaSetCombatFormula(lua_State* L)
{
	//setCombatFormula(combat, type, mina, minb, maxa, maxb[, minl, maxl[, minm, maxm[, minc[, maxc]]]])
	ScriptEnviroment* env = getEnv();
	if(env->getScriptId() != EVENT_ID_LOADING)
	{
		errorEx("This function can only be used while loading the script");
		lua_pushboolean(L, false);
		return 1;
	}

	int32_t params = lua_gettop(L), minc = 0, maxc = 0;
	if(params > 11)
		maxc = popNumber(L);

	if(params > 10)
		minc = popNumber(L);

	double minm = g_config.getDouble(ConfigManager::FORMULA_MAGIC), maxm = minm,
		minl = g_config.getDouble(ConfigManager::FORMULA_LEVEL), maxl = minl;
	if(params > 8)
	{
		maxm = popFloatNumber(L);
		minm = popFloatNumber(L);
	}

	if(params > 6)
	{
		maxl = popFloatNumber(L);
		minl = popFloatNumber(L);
	}

	double maxb = popFloatNumber(L), maxa = popFloatNumber(L),
		minb = popFloatNumber(L), mina = popFloatNumber(L);
	formulaType_t type = (formulaType_t)popNumber(L);
	if(Combat* combat = env->getCombatObject(popNumber(L)))
	{
		combat->setPlayerCombatValues(type, mina, minb, maxa, maxb, minl, maxl, minm, maxm, minc, maxc);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_COMBAT_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaSetConditionFormula(lua_State* L)
{
	//setConditionFormula(condition, mina, minb, maxa, maxb[, loaded])
	bool loaded = true;
	if(lua_gettop(L) > 5)
		loaded = popBoolean(L);

	double maxb = popFloatNumber(L), maxa = popFloatNumber(L),
		minb = popFloatNumber(L), mina = popFloatNumber(L);
	ScriptEnviroment* env = getEnv();
	if(ConditionSpeed* condition = dynamic_cast<ConditionSpeed*>(env->getConditionObject(popNumber(L), loaded)))
	{
		condition->setFormulaVars(mina, minb, maxa, maxb);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CONDITION_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoCombat(lua_State* L)
{
	//doCombat(cid, combat, param)
	ScriptEnviroment* env = getEnv();

	LuaVariant var = popVariant(L);
	uint32_t combatId = popNumber(L), cid = popNumber(L);

	Creature* creature = NULL;
	if(cid != 0)
	{
		creature = env->getCreatureByUID(cid);
		if(!creature)
		{
			errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
			lua_pushboolean(L, false);
			return 1;
		}
	}

	const Combat* combat = env->getCombatObject(combatId);
	if(!combat)
	{
		errorEx(getError(LUA_ERROR_COMBAT_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	if(var.type == VARIANT_NONE)
	{
		errorEx(getError(LUA_ERROR_VARIANT_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	switch(var.type)
	{
		case VARIANT_NUMBER:
		{
			Creature* target = g_game.getCreatureByID(var.number);
			if(!target || !creature || !creature->canSeeCreature(target))
			{
				lua_pushboolean(L, false);
				return 1;
			}

			if(combat->hasArea())
				combat->doCombat(creature, target->getPosition());
			else
				combat->doCombat(creature, target);

			break;
		}

		case VARIANT_POSITION:
		{
			combat->doCombat(creature, var.pos);
			break;
		}

		case VARIANT_TARGETPOSITION:
		{
			if(!combat->hasArea())
			{
				combat->postCombatEffects(creature, var.pos);
				g_game.addMagicEffect(var.pos, MAGIC_EFFECT_POFF);
			}
			else
				combat->doCombat(creature, var.pos);

			break;
		}

		case VARIANT_STRING:
		{
			Player* target = g_game.getPlayerByName(var.text);
			if(!target || !creature || !creature->canSeeCreature(target))
			{
				lua_pushboolean(L, false);
				return 1;
			}

			combat->doCombat(creature, target);
			break;
		}

		default:
		{
			errorEx(getError(LUA_ERROR_VARIANT_UNKNOWN));
			lua_pushboolean(L, false);
			break;
		}
	}

	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaDoCombatAreaHealth(lua_State* L)
{
	//doCombatAreaHealth(cid, type, pos, area, min, max, effect[, aggressive])
	bool aggressive = true;
	if(lua_gettop(L) > 7) // shouldn't it be enough if we check only is conditionType == CONDITION_HEALING?
		aggressive = popBoolean(L);

	MagicEffect_t effect = (MagicEffect_t)popNumber(L);
	int32_t maxChange = (int32_t)popNumber(L), minChange = (int32_t)popNumber(L);
	uint32_t areaId = popNumber(L);

	PositionEx pos;
	popPosition(L, pos);

	CombatType_t combatType = (CombatType_t)popNumber(L);
	uint32_t cid = popNumber(L);

	ScriptEnviroment* env = getEnv();
	Creature* creature = NULL;
	if(cid)
	{
		if(!(creature = env->getCreatureByUID(cid)))
		{
			errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
			lua_pushboolean(L, false);
			return 1;
		}
	}

	const CombatArea* area = env->getCombatArea(areaId);
	if(area || !areaId)
	{
		CombatParams params;
		params.combatType = combatType;
		params.effects.impact = effect;
		params.isAggressive = aggressive;

		Combat::doCombatHealth(creature, pos, area, minChange, maxChange, params);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_AREA_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoTargetCombatHealth(lua_State* L)
{
	//doTargetCombatHealth(cid, target, type, min, max, effect[, aggressive])
	bool aggressive = true;
	if(lua_gettop(L) > 6) // shouldn't it be enough if we check only is conditionType == CONDITION_HEALING?
		aggressive = popBoolean(L);

	MagicEffect_t effect = (MagicEffect_t)popNumber(L);
	int32_t maxChange = (int32_t)popNumber(L), minChange = (int32_t)popNumber(L);

	CombatType_t combatType = (CombatType_t)popNumber(L);
	uint32_t targetCid = popNumber(L), cid = popNumber(L);

	ScriptEnviroment* env = getEnv();
	Creature* creature = NULL;
	if(cid)
	{
		if(!(creature = env->getCreatureByUID(cid)))
		{
			errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
			lua_pushboolean(L, false);
			return 1;
		}
	}

	Creature* target = env->getCreatureByUID(targetCid);
	if(target)
	{
		CombatParams params;
		params.combatType = combatType;
		params.effects.impact = effect;
		params.isAggressive = aggressive;

		Combat::doCombatHealth(creature, target, minChange, maxChange, params);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoCombatAreaMana(lua_State* L)
{
	//doCombatAreaMana(cid, pos, area, min, max, effect[, aggressive])
	bool aggressive = true;
	if(lua_gettop(L) > 6)
		aggressive = popBoolean(L);

	MagicEffect_t effect = (MagicEffect_t)popNumber(L);
	int32_t maxChange = (int32_t)popNumber(L), minChange = (int32_t)popNumber(L);
	uint32_t areaId = popNumber(L);

	PositionEx pos;
	popPosition(L, pos);
	uint32_t cid = popNumber(L);

	ScriptEnviroment* env = getEnv();
	Creature* creature = NULL;
	if(cid)
	{
		if(!(creature = env->getCreatureByUID(cid)))
		{
			errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
			lua_pushboolean(L, false);
			return 1;
		}
	}

	const CombatArea* area = env->getCombatArea(areaId);
	if(area || !areaId)
	{
		CombatParams params;
		params.effects.impact = effect;
		params.isAggressive = aggressive;

		Combat::doCombatMana(creature, pos, area, minChange, maxChange, params);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_AREA_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoTargetCombatMana(lua_State* L)
{
	//doTargetCombatMana(cid, target, min, max, effect[, aggressive])
	bool aggressive = true;
	if(lua_gettop(L) > 5)
		aggressive = popBoolean(L);

	MagicEffect_t effect = (MagicEffect_t)popNumber(L);
	int32_t maxChange = (int32_t)popNumber(L), minChange = (int32_t)popNumber(L);
	uint32_t targetCid = popNumber(L), cid = popNumber(L);

	ScriptEnviroment* env = getEnv();
	Creature* creature = NULL;
	if(cid)
	{
		if(!(creature = env->getCreatureByUID(cid)))
		{
			errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
			lua_pushboolean(L, false);
			return 1;
		}
	}

	if(Creature* target = env->getCreatureByUID(targetCid))
	{
		CombatParams params;
		params.effects.impact = effect;
		params.isAggressive = aggressive;

		Combat::doCombatMana(creature, target, minChange, maxChange, params);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoCombatAreaCondition(lua_State* L)
{
	//doCombatAreaCondition(cid, pos, area, condition, effect[, loaded])
	bool loaded = true;
	if(lua_gettop(L) > 5)
		loaded = popBoolean(L);

	MagicEffect_t effect = (MagicEffect_t)popNumber(L);
	uint32_t conditionId = popNumber(L), areaId = popNumber(L);

	PositionEx pos;
	popPosition(L, pos);
	uint32_t cid = popNumber(L);

	ScriptEnviroment* env = getEnv();
	Creature* creature = NULL;
	if(cid)
	{
		if(!(creature = env->getCreatureByUID(cid)))
		{
			errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
			lua_pushboolean(L, false);
			return 1;
		}
	}

	if(const Condition* condition = env->getConditionObject(conditionId, loaded))
	{
		const CombatArea* area = env->getCombatArea(areaId);
		if(area || !areaId)
		{
			CombatParams params;
			params.effects.impact = effect;
			params.conditionList.push_back(condition);

			Combat::doCombatCondition(creature, pos, area, params);
			lua_pushboolean(L, true);
		}
		else
		{
			errorEx(getError(LUA_ERROR_AREA_NOT_FOUND));
			lua_pushboolean(L, false);
		}
	}
	else
	{
		errorEx(getError(LUA_ERROR_CONDITION_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoTargetCombatCondition(lua_State* L)
{
	//doTargetCombatCondition(cid, target, condition, effect[, loaded])
	bool loaded = true;
	if(lua_gettop(L) > 4)
		loaded = popBoolean(L);

	MagicEffect_t effect = (MagicEffect_t)popNumber(L);
	uint32_t conditionId = popNumber(L), targetCid = popNumber(L), cid = popNumber(L);

	ScriptEnviroment* env = getEnv();
	Creature* creature = NULL;
	if(cid)
	{
		if(!(creature = env->getCreatureByUID(cid)))
		{
			errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
			lua_pushboolean(L, false);
			return 1;
		}
	}

	if(Creature* target = env->getCreatureByUID(targetCid))
	{
		if(const Condition* condition = env->getConditionObject(conditionId, loaded))
		{
			CombatParams params;
			params.effects.impact = effect;
			params.conditionList.push_back(condition);

			Combat::doCombatCondition(creature, target, params);
			lua_pushboolean(L, true);
		}
		else
		{
			errorEx(getError(LUA_ERROR_CONDITION_NOT_FOUND));
			lua_pushboolean(L, false);
		}
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoCombatAreaDispel(lua_State* L)
{
	//doCombatAreaDispel(cid, pos, area, type, effect)
	MagicEffect_t effect = (MagicEffect_t)popNumber(L);
	ConditionType_t dispelType = (ConditionType_t)popNumber(L);
	uint32_t areaId = popNumber(L);

	PositionEx pos;
	popPosition(L, pos);
	uint32_t cid = popNumber(L);

	ScriptEnviroment* env = getEnv();
	Creature* creature = NULL;
	if(cid)
	{
		if(!(creature = env->getCreatureByUID(cid)))
		{
			errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
			lua_pushboolean(L, false);
			return 1;
		}
	}

	const CombatArea* area = env->getCombatArea(areaId);
	if(area || !areaId)
	{
		CombatParams params;
		params.effects.impact = effect;
		params.dispelType = dispelType;

		Combat::doCombatDispel(creature, pos, area, params);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_AREA_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoTargetCombatDispel(lua_State* L)
{
	//doTargetCombatDispel(cid, target, type, effect)
	MagicEffect_t effect = (MagicEffect_t)popNumber(L);
	ConditionType_t dispelType = (ConditionType_t)popNumber(L);
	uint32_t targetCid = popNumber(L), cid = popNumber(L);

	ScriptEnviroment* env = getEnv();
	Creature* creature = NULL;
	if(cid)
	{
		if(!(creature = env->getCreatureByUID(cid)))
		{
			errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
			lua_pushboolean(L, false);
			return 1;
		}
	}

	if(Creature* target = env->getCreatureByUID(targetCid))
	{
		CombatParams params;
		params.effects.impact = effect;
		params.dispelType = dispelType;

		Combat::doCombatDispel(creature, target, params);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoChallengeCreature(lua_State* L)
{
	//doChallengeCreature(cid, target)
	ScriptEnviroment* env = getEnv();
	uint32_t targetCid = popNumber(L);

	Creature* creature = env->getCreatureByUID(popNumber(L));
	if(!creature)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	Creature* target = env->getCreatureByUID(targetCid);
	if(!target)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	target->challengeCreature(creature);
	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaDoSummonMonster(lua_State* L)
{
	//doSummonMonster(cid, name)
	std::string name = popString(L);

	ScriptEnviroment* env = getEnv();
	Creature* creature = env->getCreatureByUID(popNumber(L));
	if(!creature)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	lua_pushnumber(L, g_game.placeSummon(creature, name));
	return 1;
}

int32_t LuaInterface::luaDoConvinceCreature(lua_State* L)
{
	//doConvinceCreature(cid, target)
	uint32_t cid = popNumber(L);

	ScriptEnviroment* env = getEnv();
	Creature* creature = env->getCreatureByUID(popNumber(L));
	if(!creature)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	Creature* target = env->getCreatureByUID(cid);
	if(!target)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	target->convinceCreature(creature);
	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaGetMonsterTargetList(lua_State* L)
{
	//getMonsterTargetList(cid)
	ScriptEnviroment* env = getEnv();
	Creature* creature = env->getCreatureByUID(popNumber(L));
	if(!creature)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	Monster* monster = creature->getMonster();
	if(!monster)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	const CreatureList& targetList = monster->getTargetList();
	CreatureList::const_iterator it = targetList.begin();

	lua_newtable(L);
	for(uint32_t i = 1; it != targetList.end(); ++it, ++i)
	{
		if(monster->isTarget(*it))
		{
			lua_pushnumber(L, i);
			lua_pushnumber(L, env->addThing(*it));
			pushTable(L);
		}
	}

	return 1;
}

int32_t LuaInterface::luaGetMonsterFriendList(lua_State* L)
{
	//getMonsterFriendList(cid)
	ScriptEnviroment* env = getEnv();
	Creature* creature = env->getCreatureByUID(popNumber(L));
	if(!creature)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	Monster* monster = creature->getMonster();
	if(!monster)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	Creature* friendCreature;
	const CreatureList& friendList = monster->getFriendList();
	CreatureList::const_iterator it = friendList.begin();

	lua_newtable(L);
	for(uint32_t i = 1; it != friendList.end(); ++it, ++i)
	{
		friendCreature = (*it);
		if(!friendCreature->isRemoved() && friendCreature->getPosition().z == monster->getPosition().z)
		{
			lua_pushnumber(L, i);
			lua_pushnumber(L, env->addThing(*it));
			pushTable(L);
		}
	}

	return 1;
}

int32_t LuaInterface::luaDoMonsterSetTarget(lua_State* L)
{
	//doMonsterSetTarget(cid, target)
	uint32_t targetId = popNumber(L);
	ScriptEnviroment* env = getEnv();

	Creature* creature = env->getCreatureByUID(popNumber(L));
	if(!creature)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	Monster* monster = creature->getMonster();
	if(!monster)
	{
		errorEx(getError(LUA_ERROR_MONSTER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	Creature* target = env->getCreatureByUID(targetId);
	if(!target)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	if(!monster->isSummon())
		lua_pushboolean(L, monster->selectTarget(target));
	else
		lua_pushboolean(L, false);

	return 1;
}

int32_t LuaInterface::luaDoMonsterChangeTarget(lua_State* L)
{
	//doMonsterChangeTarget(cid)
	ScriptEnviroment* env = getEnv();
	Creature* creature = env->getCreatureByUID(popNumber(L));
	if(!creature)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	Monster* monster = creature->getMonster();
	if(!monster)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	if(!monster->isSummon())
		monster->searchTarget(TARGETSEARCH_RANDOM);

	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaGetMonsterInfo(lua_State* L)
{
	//getMonsterInfo(name)
	const MonsterType* mType = g_monsters.getMonsterType(popString(L));
	if(!mType)
	{
		errorEx(getError(LUA_ERROR_MONSTER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	lua_newtable(L);
	setField(L, "name", mType->name.c_str());
	setField(L, "description", mType->nameDescription.c_str());
	setField(L, "file", mType->file.c_str());
	setField(L, "experience", mType->experience);
	setField(L, "health", mType->health);
	setField(L, "healthMax", mType->healthMax);
	setField(L, "manaCost", mType->manaCost);
	setField(L, "defense", mType->defense);
	setField(L, "armor", mType->armor);
	setField(L, "baseSpeed", mType->baseSpeed);
	setField(L, "lookCorpse", mType->lookCorpse);
	setField(L, "corpseUnique", mType->corpseUnique);
	setField(L, "corpseAction", mType->corpseAction);
	setField(L, "race", mType->race);
	setField(L, "skull", mType->skull);
	setField(L, "partyShield", mType->partyShield);
	setField(L, "guildEmblem", mType->guildEmblem);
	setFieldBool(L, "summonable", mType->isSummonable);
	setFieldBool(L, "illusionable", mType->isIllusionable);
	setFieldBool(L, "convinceable", mType->isConvinceable);
	setFieldBool(L, "attackable", mType->isAttackable);
	setFieldBool(L, "hostile", mType->isHostile);

	lua_pushstring(L, "outfit"); // name the table created by pushOutfit
	pushOutfit(L, mType->outfit);
	pushTable(L);
	createTable(L, "defenses");

	SpellList::const_iterator it = mType->spellDefenseList.begin();
	for(uint32_t i = 1; it != mType->spellDefenseList.end(); ++it, ++i)
	{
		createTable(L, i);
		setField(L, "speed", it->speed);
		setField(L, "chance", it->chance);
		setField(L, "range", it->range);

		setField(L, "minCombatValue", it->minCombatValue);
		setField(L, "maxCombatValue", it->maxCombatValue);
		setFieldBool(L, "isMelee", it->isMelee);
		pushTable(L);
	}

	pushTable(L);
	createTable(L, "attacks");

	it = mType->spellAttackList.begin();
	for(uint32_t i = 1; it != mType->spellAttackList.end(); ++it, ++i)
	{
		createTable(L, i);
		setField(L, "speed", it->speed);
		setField(L, "chance", it->chance);
		setField(L, "range", it->range);

		setField(L, "minCombatValue", it->minCombatValue);
		setField(L, "maxCombatValue", it->maxCombatValue);
		setFieldBool(L, "isMelee", it->isMelee);
		pushTable(L);
	}

	pushTable(L);
	createTable(L, "loot");

	LootItems::const_iterator lit = mType->lootItems.begin();
	for(uint32_t i = 1; lit != mType->lootItems.end(); ++lit, ++i)
	{
		createTable(L, i);
		if(lit->ids.size() > 1)
		{
			createTable(L, "ids");
			std::vector<uint16_t>::const_iterator iit = lit->ids.begin();
			for(uint32_t j = 1; iit != lit->ids.end(); ++iit, ++j)
			{
				lua_pushnumber(L, j);
				lua_pushnumber(L, (*iit));
				pushTable(L);
			}

			pushTable(L);
		}
		else
			setField(L, "id", lit->ids[0]);

		setField(L, "count", lit->count);
		setField(L, "chance", lit->chance);
		setField(L, "subType", lit->subType);
		setField(L, "actionId", lit->actionId);
		setField(L, "uniqueId", lit->uniqueId);
		setField(L, "text", lit->text);

		if(lit->childLoot.size() > 0)
		{
			createTable(L, "child");
			LootItems::const_iterator cit = lit->childLoot.begin();
			for(uint32_t j = 1; cit != lit->childLoot.end(); ++cit, ++j)
			{
				createTable(L, j);
				if(cit->ids.size() > 1)
				{
					createTable(L, "ids");
					std::vector<uint16_t>::const_iterator iit = cit->ids.begin();
					for(uint32_t k = 1; iit != cit->ids.end(); ++iit, ++k)
					{
						lua_pushnumber(L, k);
						lua_pushnumber(L, (*iit));
						pushTable(L);
					}

					pushTable(L);
				}
				else
					setField(L, "id", cit->ids[0]);

				setField(L, "count", cit->count);
				setField(L, "chance", cit->chance);
				setField(L, "subType", cit->subType);
				setField(L, "actionId", cit->actionId);
				setField(L, "uniqueId", cit->uniqueId);
				setField(L, "text", cit->text);

				pushTable(L);
			}

			pushTable(L);
		}

		pushTable(L);
	}

	pushTable(L);
	createTable(L, "summons");

	SummonList::const_iterator sit = mType->summonList.begin();
	for(uint32_t i = 1; sit != mType->summonList.end(); ++sit, ++i)
	{
		createTable(L, i);
		setField(L, "name", sit->name);
		setField(L, "chance", sit->chance);

		setField(L, "interval", sit->interval);
		setField(L, "amount", sit->amount);
		pushTable(L);
	}

	pushTable(L);
	return 1;
}

int32_t LuaInterface::luaGetTalkActionList(lua_State* L)
{
	//getTalkactionList()
	lua_newtable(L);

	TalkActionsMap::const_iterator it = g_talkActions->getFirstTalk();
	for(uint32_t i = 1; it != g_talkActions->getLastTalk(); ++it, ++i)
	{
		createTable(L, i);
		setField(L, "words", it->first);
		setField(L, "access", it->second->getAccess());

		createTable(L, "groups");
		IntegerVec::const_iterator git = it->second->getGroupsBegin();
		for(uint32_t j = 1; git != it->second->getGroupsEnd(); ++git, ++j)
		{
			lua_pushnumber(L, j);
			lua_pushnumber(L, *git);
			pushTable(L);
		}

		pushTable(L);
		setFieldBool(L, "log", it->second->isLogged());
		setFieldBool(L, "logged", it->second->isLogged());
		setFieldBool(L, "hide", it->second->isHidden());
		setFieldBool(L, "hidden", it->second->isHidden());

		setField(L, "functionName", it->second->getFunctionName());
		setField(L, "channel", it->second->getChannel());
		pushTable(L);
	}

	return 1;
}

int32_t LuaInterface::luaGetExperienceStageList(lua_State* L)
{
	//getExperienceStageList()
	if(!g_config.getBool(ConfigManager::EXPERIENCE_STAGES))
	{
		lua_pushboolean(L, false);
		return true;
	}

	StageList::const_iterator it = g_game.getFirstStage();
	lua_newtable(L);
	for(uint32_t i = 1; it != g_game.getLastStage(); ++it, ++i)
	{
		createTable(L, i);
		setField(L, "level", it->first);
		setFieldFloat(L, "multiplier", it->second);
		pushTable(L);
	}

	return 1;
}

int32_t LuaInterface::luaDoAddCondition(lua_State* L)
{
	//doAddCondition(cid, condition[, loaded])
	bool loaded = true;
	if(lua_gettop(L) > 2)
		loaded = popBoolean(L);

	uint32_t conditionId = popNumber(L);
	ScriptEnviroment* env = getEnv();

	Creature* creature = env->getCreatureByUID(popNumber(L));
	if(!creature)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	Condition* condition = env->getConditionObject(conditionId, loaded);
	if(!condition)
	{
		errorEx(getError(LUA_ERROR_CONDITION_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	creature->addCondition(condition->clone());
	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaDoRemoveCondition(lua_State* L)
{
	//doRemoveCondition(cid, type[, subId = 0[, conditionId = CONDITIONID_COMBAT]])
	int32_t conditionId = CONDITIONID_COMBAT;
	uint32_t params = lua_gettop(L), subId = 0;
	if(params > 3)
		conditionId = popNumber(L);

	if(params > 2)
		subId = popNumber(L);

	ConditionType_t conditionType = (ConditionType_t)popNumber(L);
	ScriptEnviroment* env = getEnv();

	Creature* creature = env->getCreatureByUID(popNumber(L));
	if(!creature)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	Condition* condition = NULL;
	while((condition = creature->getCondition(conditionType, (ConditionId_t)conditionId, subId)))
		creature->removeCondition(condition);

	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaDoRemoveConditions(lua_State* L)
{
	//doRemoveConditions(cid[, onlyPersistent])
	bool onlyPersistent = true;
	if(lua_gettop(L) > 1)
		onlyPersistent = popBoolean(L);

	ScriptEnviroment* env = getEnv();
	Creature* creature = env->getCreatureByUID(popNumber(L));
	if(!creature)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	creature->removeConditions(CONDITIONEND_ABORT, onlyPersistent);
	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaNumberToVariant(lua_State* L)
{
	//numberToVariant(number)
	LuaVariant var;
	var.type = VARIANT_NUMBER;
	var.number = popNumber(L);

	LuaInterface::pushVariant(L, var);
	return 1;
}

int32_t LuaInterface::luaStringToVariant(lua_State* L)
{
	//stringToVariant(string)
	LuaVariant var;
	var.type = VARIANT_STRING;
	var.text = popString(L);

	LuaInterface::pushVariant(L, var);
	return 1;
}

int32_t LuaInterface::luaPositionToVariant(lua_State* L)
{
	//positionToVariant(pos)
	LuaVariant var;
	var.type = VARIANT_POSITION;
	popPosition(L, var.pos);

	LuaInterface::pushVariant(L, var);
	return 1;
}

int32_t LuaInterface::luaTargetPositionToVariant(lua_State* L)
{
	//targetPositionToVariant(pos)
	LuaVariant var;
	var.type = VARIANT_TARGETPOSITION;
	popPosition(L, var.pos);

	LuaInterface::pushVariant(L, var);
	return 1;
}

int32_t LuaInterface::luaVariantToNumber(lua_State* L)
{
	//variantToNumber(var)
	LuaVariant var = popVariant(L);

	uint32_t number = 0;
	if(var.type == VARIANT_NUMBER)
		number = var.number;

	lua_pushnumber(L, number);
	return 1;
}

int32_t LuaInterface::luaVariantToString(lua_State* L)
{
	//variantToString(var)
	LuaVariant var = popVariant(L);

	std::string text = "";
	if(var.type == VARIANT_STRING)
		text = var.text;

	lua_pushstring(L, text.c_str());
	return 1;
}

int32_t LuaInterface::luaVariantToPosition(lua_State* L)
{
	//luaVariantToPosition(var)
	LuaVariant var = popVariant(L);

	PositionEx pos(0, 0, 0, 0);
	if(var.type == VARIANT_POSITION || var.type == VARIANT_TARGETPOSITION)
		pos = var.pos;

	pushPosition(L, pos, pos.stackpos);
	return 1;
}

int32_t LuaInterface::luaDoChangeSpeed(lua_State* L)
{
	//doChangeSpeed(cid, delta)
	int32_t delta = (int32_t)popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		g_game.changeSpeed(creature, delta);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaSetCreatureOutfit(lua_State* L)
{
	//doSetCreatureOutfit(cid, outfit[, time = -1])
	int32_t time = -1;
	if(lua_gettop(L) > 2)
		time = (int32_t)popNumber(L);

	Outfit_t outfit = popOutfit(L);
	ScriptEnviroment* env = getEnv();

	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
		lua_pushboolean(L, Spell::CreateIllusion(creature, outfit, time) == RET_NOERROR);
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetCreatureOutfit(lua_State* L)
{
	//getCreatureOutfit(cid)
	ScriptEnviroment* env = getEnv();
	if(const Creature* creature = env->getCreatureByUID(popNumber(L)))
		pushOutfit(L, creature->getCurrentOutfit());
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaSetMonsterOutfit(lua_State* L)
{
	//doSetMonsterOutfit(cid, name[, time = -1])
	int32_t time = -1;
	if(lua_gettop(L) > 2)
		time = (int32_t)popNumber(L);

	std::string name = popString(L);
	ScriptEnviroment* env = getEnv();

	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
		lua_pushboolean(L, Spell::CreateIllusion(creature, name, time) == RET_NOERROR);
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaSetItemOutfit(lua_State* L)
{
	//doSetItemOutfit(cid, item[, time = -1])
	int32_t time = -1;
	if(lua_gettop(L) > 2)
		time = (int32_t)popNumber(L);

	uint32_t item = (uint32_t)popNumber(L);
	ScriptEnviroment* env = getEnv();

	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
		lua_pushboolean(L, Spell::CreateIllusion(creature, item, time) == RET_NOERROR);
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetStorageList(lua_State* L)
{
	//getStorageList()
	ScriptEnviroment* env = getEnv();

	StorageMap::const_iterator it = env->getStorageBegin();
	lua_newtable(L);
	for(uint32_t i = 1; it != env->getStorageEnd(); ++i, ++it)
	{
		lua_pushnumber(L, i);
		lua_pushstring(L, it->first.c_str());
		pushTable(L);
	}

	return 1;
}

int32_t LuaInterface::luaGetStorage(lua_State* L)
{
	//getStorage(key)
	ScriptEnviroment* env = getEnv();
	std::string strValue;
	if(!env->getStorage(popString(L), strValue))
	{
		lua_pushnumber(L, -1);
		lua_pushnil(L);
		return 2;
	}

	int32_t intValue = atoi(strValue.c_str());
	if(intValue || strValue == "0")
		lua_pushnumber(L, intValue);
	else
		lua_pushstring(L, strValue.c_str());

	return 1;
}

int32_t LuaInterface::luaDoSetStorage(lua_State* L)
{
	//doSetStorage(key, value)
	std::string value;
	bool tmp = false;
	if(lua_isnil(L, -1))
	{
		tmp = true;
		lua_pop(L, 1);
	}
	else
		value = popString(L);

	ScriptEnviroment* env = getEnv();
	if(!tmp)
		env->setStorage(popString(L), value);
	else
		env->eraseStorage(popString(L));

	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaGetPlayerDepotItems(lua_State* L)
{
	//getPlayerDepotItems(cid, depotid)
	uint32_t depotid = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		if(const Depot* depot = player->getDepot(depotid, true))
			lua_pushnumber(L, depot->getItemHoldingCount());
		else
			lua_pushboolean(L, false);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSetGuildId(lua_State* L)
{
	//doPlayerSetGuildId(cid, id)
	uint32_t id = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		if(player->getGuildId())
		{
			player->leaveGuild();

			if(!id)
				lua_pushboolean(L, true);
			else if(IOGuild::getInstance()->guildExists(id))
				lua_pushboolean(L, IOGuild::getInstance()->joinGuild(player, id));
			else
				lua_pushboolean(L, false);
		}
		else if(id && IOGuild::getInstance()->guildExists(id))
			lua_pushboolean(L, IOGuild::getInstance()->joinGuild(player, id));
		else
			lua_pushboolean(L, false);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSetGuildLevel(lua_State* L)
{
	//doPlayerSetGuildLevel(cid, level[, rank])
	uint32_t rank = 0;
	if(lua_gettop(L) > 2)
		rank = popNumber(L);

	GuildLevel_t level = (GuildLevel_t)popNumber(L);
	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
		lua_pushboolean(L, player->setGuildLevel(level, rank));
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSetGuildNick(lua_State* L)
{
	//doPlayerSetGuildNick(cid, nick)
	std::string nick = popString(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->setGuildNick(nick);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetGuildId(lua_State* L)
{
	//getGuildId(guildName)
	uint32_t guildId;
	if(IOGuild::getInstance()->getGuildId(guildId, popString(L)))
		lua_pushnumber(L, guildId);
	else
		lua_pushboolean(L, false);

	return 1;
}

int32_t LuaInterface::luaGetGuildMotd(lua_State* L)
{
	//getGuildMotd(guildId)
	uint32_t guildId = popNumber(L);
	if(IOGuild::getInstance()->guildExists(guildId))
		lua_pushstring(L, IOGuild::getInstance()->getMotd(guildId).c_str());
	else
		lua_pushboolean(L, false);

	return 1;
}

int32_t LuaInterface::luaDoMoveCreature(lua_State* L)
{
	//doMoveCreature(cid, direction[, flag = FLAG_NOLIMIT])
	uint32_t flags = FLAG_NOLIMIT;
	if(lua_gettop(L) > 2)
		flags = popNumber(L);

	int32_t direction = popNumber(L);
	if(direction < NORTH || direction > NORTHEAST)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
		lua_pushnumber(L, g_game.internalMoveCreature(creature, (Direction)direction, flags));
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoSteerCreature(lua_State* L)
{
	//doSteerCreature(cid, position[, maxNodes])
	uint16_t maxNodes = 100;
	if(lua_gettop(L) > 2)
		maxNodes = popNumber(L);

	PositionEx pos;
	popPosition(L, pos);

	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
		lua_pushboolean(L, g_game.steerCreature(creature, pos, maxNodes));
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaIsCreature(lua_State* L)
{
	//isCreature(cid)
	ScriptEnviroment* env = getEnv();
	lua_pushboolean(L, env->getCreatureByUID(popNumber(L)) ? true : false);
	return 1;
}

int32_t LuaInterface::luaIsMovable(lua_State* L)
{
	//isMovable(uid)
	ScriptEnviroment* env = getEnv();
	Thing* thing = env->getThingByUID(popNumber(L));
	if(thing && thing->isPushable())
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);

	return 1;
}

int32_t LuaInterface::luaGetCreatureByName(lua_State* L)
{
	//getCreatureByName(name)
	ScriptEnviroment* env = getEnv();
	if(Creature* creature = g_game.getCreatureByName(popString(L)))
		lua_pushnumber(L, env->addThing(creature));
	else
		lua_pushnil(L);

	return 1;
}

int32_t LuaInterface::luaGetPlayerByGUID(lua_State* L)
{
	//getPlayerByGUID(guid)
	ScriptEnviroment* env = getEnv();
	if(Player* player = g_game.getPlayerByGuid(popNumber(L)))
		lua_pushnumber(L, env->addThing(player));
	else
		lua_pushnil(L);

	return 1;
}

int32_t LuaInterface::luaGetPlayerByNameWildcard(lua_State* L)
{
	//getPlayerByNameWildcard(name~[, ret = false])
	Player* player = NULL;
	bool pushValue = false;
	if(lua_gettop(L) > 1)
		pushValue = popBoolean(L);

	ScriptEnviroment* env = getEnv();
	ReturnValue ret = g_game.getPlayerByNameWildcard(popString(L), player);
	if(ret == RET_NOERROR)
		lua_pushnumber(L, env->addThing(player));
	else if(pushValue)
		lua_pushnumber(L, ret);
	else
		lua_pushnil(L);

	return 1;
}

int32_t LuaInterface::luaGetPlayerGUIDByName(lua_State* L)
{
	//getPlayerGUIDByName(name[, multiworld = false])
	bool multiworld = false;
	if(lua_gettop(L) > 1)
		multiworld = popBoolean(L);

	std::string name = popString(L);
	uint32_t guid;
	if(Player* player = g_game.getPlayerByName(name.c_str()))
		lua_pushnumber(L, player->getGUID());
	else if(IOLoginData::getInstance()->getGuidByName(guid, name, multiworld))
		lua_pushnumber(L, guid);
	else
		lua_pushnil(L);

	return 1;
}

int32_t LuaInterface::luaGetPlayerNameByGUID(lua_State* L)
{
	//getPlayerNameByGUID(guid[, multiworld = false])
	bool multiworld = false;
	if(lua_gettop(L) > 1)
		multiworld = popBoolean(L);

	uint32_t guid = popNumber(L);
	std::string name;
	if(!IOLoginData::getInstance()->getNameByGuid(guid, name, multiworld))
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushnil(L);
		return 1;
	}

	lua_pushstring(L, name.c_str());
	return 1;
}

int32_t LuaInterface::luaDoPlayerChangeName(lua_State* L)
{
	//doPlayerChangeName(guid, oldName, newName)
	std::string newName = popString(L), oldName = popString(L);
	uint32_t guid = popNumber(L);
	if(IOLoginData::getInstance()->changeName(guid, newName, oldName))
	{
		if(House* house = Houses::getInstance()->getHouseByPlayerId(guid))
			house->updateDoorDescription(newName);

		lua_pushboolean(L, true);
	}
	else
		lua_pushboolean(L, false);

	return 1;
}

int32_t LuaInterface::luaGetPlayersByAccountId(lua_State* L)
{
	//getPlayersByAccountId(accId)
	PlayerVector players = g_game.getPlayersByAccount(popNumber(L));

	ScriptEnviroment* env = getEnv();
	PlayerVector::iterator it = players.begin();

	lua_newtable(L);
	for(uint32_t i = 1; it != players.end(); ++it, ++i)
	{
		lua_pushnumber(L, i);
		lua_pushnumber(L, env->addThing(*it));
		pushTable(L);
	}

	return 1;
}

int32_t LuaInterface::luaGetIpByName(lua_State* L)
{
	//getIpByName(name)
	std::string name = popString(L);
	if(Player* player = g_game.getPlayerByName(name))
		lua_pushnumber(L, player->getIP());
	else if(IOLoginData::getInstance()->playerExists(name))
		lua_pushnumber(L, IOLoginData::getInstance()->getLastIPByName(name));
	else
		lua_pushnil(L);

	return 1;
}

int32_t LuaInterface::luaGetPlayersByIp(lua_State* L)
{
	//getPlayersByIp(ip[, mask])
	uint32_t mask = 0xFFFFFFFF;
	if(lua_gettop(L) > 1)
		mask = (uint32_t)popNumber(L);

	PlayerVector players = g_game.getPlayersByIP(popNumber(L), mask);

	ScriptEnviroment* env = getEnv();
	PlayerVector::iterator it = players.begin();

	lua_newtable(L);
	for(uint32_t i = 1; it != players.end(); ++it, ++i)
	{
		lua_pushnumber(L, i);
		lua_pushnumber(L, env->addThing(*it));
		pushTable(L);
	}

	return 1;
}

int32_t LuaInterface::luaGetAccountIdByName(lua_State* L)
{
	//getAccountIdByName(name)
	std::string name = popString(L);

	if(Player* player = g_game.getPlayerByName(name))
		lua_pushnumber(L, player->getAccount());
	else
		lua_pushnumber(L, IOLoginData::getInstance()->getAccountIdByName(name));

	return 1;
}

int32_t LuaInterface::luaGetAccountByName(lua_State* L)
{
	//getAccountByName(name)
	std::string name = popString(L);

	if(Player* player = g_game.getPlayerByName(name))
		lua_pushstring(L, player->getAccountName().c_str());
	else
	{
		std::string tmp;
		IOLoginData::getInstance()->getAccountName(IOLoginData::getInstance()->getAccountIdByName(name), tmp);
		lua_pushstring(L, tmp.c_str());
	}

	return 1;
}

int32_t LuaInterface::luaGetAccountIdByAccount(lua_State* L)
{
	//getAccountIdByAccount(accName)
	uint32_t value = 0;
	IOLoginData::getInstance()->getAccountId(popString(L), value);
	lua_pushnumber(L, value);
	return 1;
}

int32_t LuaInterface::luaGetAccountByAccountId(lua_State* L)
{
	//getAccountByAccountId(accId)
	std::string value = 0;
	IOLoginData::getInstance()->getAccountName(popNumber(L), value);
	lua_pushstring(L, value.c_str());
	return 1;
}

int32_t LuaInterface::luaGetAccountFlagValue(lua_State* L)
{
	//getAccountFlagValue(name/id)
	PlayerFlags flag = (PlayerFlags)popNumber(L);
	if(lua_isnumber(L, -1))
		lua_pushboolean(L, IOLoginData::getInstance()->hasFlag((uint32_t)popNumber(L), flag));
	else
		lua_pushboolean(L, IOLoginData::getInstance()->hasFlag(flag, popString(L)));

	return 1;
}

int32_t LuaInterface::luaGetAccountCustomFlagValue(lua_State* L)
{
	//getAccountCustomFlagValue(name/id)
	PlayerCustomFlags flag = (PlayerCustomFlags)popNumber(L);
	if(lua_isnumber(L, -1))
		lua_pushboolean(L, IOLoginData::getInstance()->hasCustomFlag((uint32_t)popNumber(L), flag));
	else
		lua_pushboolean(L, IOLoginData::getInstance()->hasCustomFlag(flag, popString(L)));

	return 1;
}

int32_t LuaInterface::luaRegisterCreatureEvent(lua_State* L)
{
	//registerCreatureEvent(cid, name)
	std::string name = popString(L);

	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
		lua_pushboolean(L, creature->registerCreatureEvent(name));
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaUnregisterCreatureEvent(lua_State* L)
{
	//unregisterCreatureEvent(cid, name)
	std::string name = popString(L);

	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
		lua_pushboolean(L, creature->unregisterCreatureEvent(name));
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaUnregisterCreatureEventType(lua_State* L)
{
	//unregisterCreatureEventType(cid, type)
	std::string type = popString(L);

	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		CreatureEventType_t _type = g_creatureEvents->getType(type);
		if(_type != CREATURE_EVENT_NONE)
		{
			creature->unregisterCreatureEvent(_type);
			lua_pushboolean(L, true);
		}
		else
		{
			errorEx("Invalid event type");
			lua_pushboolean(L, false);
		}
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetContainerSize(lua_State* L)
{
	//getContainerSize(uid)
	ScriptEnviroment* env = getEnv();
	if(Container* container = env->getContainerByUID(popNumber(L)))
		lua_pushnumber(L, container->size());
	else
	{
		errorEx(getError(LUA_ERROR_CONTAINER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetContainerCap(lua_State* L)
{
	//getContainerCap(uid)
	ScriptEnviroment* env = getEnv();
	if(Container* container = env->getContainerByUID(popNumber(L)))
		lua_pushnumber(L, container->capacity());
	else
	{
		errorEx(getError(LUA_ERROR_CONTAINER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetContainerItem(lua_State* L)
{
	//getContainerItem(uid, slot)
	uint32_t slot = popNumber(L);
	ScriptEnviroment* env = getEnv();
	if(Container* container = env->getContainerByUID(popNumber(L)))
	{
		if(Item* item = container->getItem(slot))
			pushThing(L, item, env->addThing(item));
		else
			pushThing(L, NULL, 0);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CONTAINER_NOT_FOUND));
		pushThing(L, NULL, 0);
	}

	return 1;
}

int32_t LuaInterface::luaDoAddContainerItemEx(lua_State* L)
{
	//doAddContainerItemEx(uid, virtuid)
	uint32_t virtuid = popNumber(L);
	ScriptEnviroment* env = getEnv();
	if(Container* container = env->getContainerByUID(popNumber(L)))
	{
		Item* item = env->getItemByUID(virtuid);
		if(!item)
		{
			errorEx(getError(LUA_ERROR_ITEM_NOT_FOUND));
			lua_pushboolean(L, false);
			return 1;
		}

		if(item->getParent() != VirtualCylinder::virtualCylinder)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		ReturnValue ret = g_game.internalAddItem(NULL, container, item);
		if(ret == RET_NOERROR)
			env->removeTempItem(env, item);

		lua_pushnumber(L, ret);
		return 1;
	}
	else
	{
		errorEx(getError(LUA_ERROR_CONTAINER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}
}

int32_t LuaInterface::luaDoAddContainerItem(lua_State* L)
{
	//doAddContainerItem(uid, itemid[, count/subType = 1])
	uint32_t count = 1;
	if(lua_gettop(L) > 2)
		count = popNumber(L);

	uint16_t itemId = popNumber(L);
	ScriptEnviroment* env = getEnv();

	Container* container = env->getContainerByUID((uint32_t)popNumber(L));
	if(!container)
	{
		errorEx(getError(LUA_ERROR_CONTAINER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	const ItemType& it = Item::items[itemId];
	int32_t itemCount = 1, subType = 1;
	if(it.hasSubType())
	{
		if(it.stackable)
			itemCount = (int32_t)std::ceil((float)count / 100);

		subType = count;
	}
	else
		itemCount = std::max((uint32_t)1, count);

	uint32_t ret = 0;
	Item* newItem = NULL;
	while(itemCount > 0)
	{
		int32_t stackCount = std::min(100, subType);
		if(!(newItem = Item::CreateItem(itemId, stackCount)))
		{
			errorEx(getError(LUA_ERROR_ITEM_NOT_FOUND));
			lua_pushboolean(L, false);
			return 1;
		}

		if(it.stackable)
			subType -= stackCount;

		uint32_t dummy = 0;
		Item* stackItem = NULL;
		if(g_game.internalAddItem(NULL, container, newItem, INDEX_WHEREEVER, FLAG_NOLIMIT, false, dummy, &stackItem) != RET_NOERROR)
		{
			delete newItem;
			lua_pushboolean(L, false);
			return ++ret;
		}

		++ret;
		if(newItem->getParent())
			lua_pushnumber(L, env->addThing(newItem));
		else if(stackItem)
			lua_pushnumber(L, env->addThing(stackItem));
		else //stackable item stacked with existing object, newItem will be released
			lua_pushnil(L);

		--itemCount;
	}

	if(ret)
		return ret;

	lua_pushnil(L);
	return 1;
}

int32_t LuaInterface::luaDoPlayerAddOutfit(lua_State *L)
{
	//Consider using doPlayerAddOutfitId instead
	//doPlayerAddOutfit(cid, looktype, addon)
	uint32_t addon = popNumber(L), lookType = popNumber(L);
	ScriptEnviroment* env = getEnv();

	Player* player = env->getPlayerByUID((uint32_t)popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	Outfit outfit;
	if(Outfits::getInstance()->getOutfit(lookType, outfit))
	{
		lua_pushboolean(L, player->addOutfit(outfit.outfitId, addon));
		return 1;
	}

	lua_pushboolean(L, false);
	return 1;
}

int32_t LuaInterface::luaDoPlayerRemoveOutfit(lua_State *L)
{
	//Consider using doPlayerRemoveOutfitId instead
	//doPlayerRemoveOutfit(cid, looktype[, addon = 0])
	uint32_t addon = 0xFF;
	if(lua_gettop(L) > 2)
		addon = popNumber(L);

	uint32_t lookType = popNumber(L);
	ScriptEnviroment* env = getEnv();

	Player* player = env->getPlayerByUID((uint32_t)popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	Outfit outfit;
	if(Outfits::getInstance()->getOutfit(lookType, outfit))
	{
		lua_pushboolean(L, player->removeOutfit(outfit.outfitId, addon));
		return 1;
	}

	lua_pushboolean(L, false);
	return 1;
}

int32_t LuaInterface::luaDoPlayerAddOutfitId(lua_State *L)
{
	//doPlayerAddOutfitId(cid, outfitId, addon)
	uint32_t addon = popNumber(L), outfitId = popNumber(L);
	ScriptEnviroment* env = getEnv();

	Player* player = env->getPlayerByUID((uint32_t)popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	lua_pushboolean(L, player->addOutfit(outfitId, addon));
	return 1;
}

int32_t LuaInterface::luaDoPlayerRemoveOutfitId(lua_State *L)
{
	//doPlayerRemoveOutfitId(cid, outfitId[, addon = 0])
	uint32_t addon = 0xFF;
	if(lua_gettop(L) > 2)
		addon = popNumber(L);

	uint32_t outfitId = popNumber(L);
	ScriptEnviroment* env = getEnv();

	Player* player = env->getPlayerByUID((uint32_t)popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	lua_pushboolean(L, player->removeOutfit(outfitId, addon));
	return 1;
}

int32_t LuaInterface::luaCanPlayerWearOutfit(lua_State* L)
{
	//canPlayerWearOutfit(cid, looktype[, addon = 0])
	uint32_t addon = 0;
	if(lua_gettop(L) > 2)
		addon = popNumber(L);

	uint32_t lookType = popNumber(L);
	ScriptEnviroment* env = getEnv();

	Player* player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	Outfit outfit;
	if(Outfits::getInstance()->getOutfit(lookType, outfit))
	{
		lua_pushboolean(L, player->canWearOutfit(outfit.outfitId, addon));
		return 1;
	}

	lua_pushboolean(L, false);
	return 1;
}

int32_t LuaInterface::luaCanPlayerWearOutfitId(lua_State* L)
{
	//canPlayerWearOutfitId(cid, outfitId[, addon = 0])
	uint32_t addon = 0;
	if(lua_gettop(L) > 2)
		addon = popNumber(L);

	uint32_t outfitId = popNumber(L);
	ScriptEnviroment* env = getEnv();

	Player* player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	lua_pushboolean(L, player->canWearOutfit(outfitId, addon));
	return 1;
}

int32_t LuaInterface::luaDoCreatureChangeOutfit(lua_State* L)
{
	//doCreatureChangeOutfit(cid, outfit)
	Outfit_t outfit = popOutfit(L);
	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		if(Player* player = creature->getPlayer())
			player->changeOutfit(outfit, false);
		else
			creature->defaultOutfit = outfit;

		if(!creature->hasCondition(CONDITION_OUTFIT, 1))
			g_game.internalCreatureChangeOutfit(creature, outfit);

		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerPopupFYI(lua_State* L)
{
	//doPlayerPopupFYI(cid, message)
	std::string message = popString(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->sendFYIBox(message);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSendTutorial(lua_State* L)
{
	//doPlayerSendTutorial(cid, id)
	uint8_t id = (uint8_t)popNumber(L);

	ScriptEnviroment* env = getEnv();

	Player* player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	player->sendTutorial(id);
	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaDoPlayerSendMailByName(lua_State* L)
{
	//doPlayerSendMailByName(name, item[, town[, actor]])
	ScriptEnviroment* env = getEnv();
	int32_t params = lua_gettop(L);

	Creature* actor = NULL;
	if(params > 3)
		actor = env->getCreatureByUID(popNumber(L));

	uint32_t town = 0;
	if(params > 2)
		town = popNumber(L);

	Item* item = env->getItemByUID(popNumber(L));
	if(!item)
	{
		errorEx(getError(LUA_ERROR_ITEM_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	if(item->getParent() != VirtualCylinder::virtualCylinder)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	bool result = IOLoginData::getInstance()->playerMail(actor, popString(L), town, item);
	if(result)
		env->removeTempItem(env, item);

	lua_pushboolean(L, result);
	return 1;
}

int32_t LuaInterface::luaDoPlayerAddMapMark(lua_State* L)
{
	//doPlayerAddMapMark(cid, pos, type[, description])
	std::string description;
	if(lua_gettop(L) > 3)
		description = popString(L);

	MapMarks_t type = (MapMarks_t)popNumber(L);
	PositionEx pos;
	popPosition(L, pos);

	ScriptEnviroment* env = getEnv();
	Player* player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	player->sendAddMarker(pos, type, description);
	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaDoPlayerAddPremiumDays(lua_State* L)
{
	//doPlayerAddPremiumDays(cid, days)
	int32_t days = popNumber(L);
	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->addPremiumDays(days);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetCreatureLastPosition(lua_State* L)
{
	//getCreatureLastPosition(cid)
	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
		pushPosition(L, creature->getLastPosition(), 0);
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetCreatureName(lua_State* L)
{
	//getCreatureName(cid)
	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
		lua_pushstring(L, creature->getName().c_str());
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetCreatureNoMove(lua_State* L)
{
	//getCreatureNoMove(cid)
	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
		lua_pushboolean(L, creature->getNoMove());
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetCreatureGuildEmblem(lua_State* L)
{
	//getCreatureGuildEmblem(cid[, target])
	uint32_t tid = 0;
	if(lua_gettop(L) > 1)
		tid = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		if(!tid)
			lua_pushnumber(L, creature->getEmblem());
		else if(Creature* target = env->getCreatureByUID(tid))
			lua_pushnumber(L, creature->getGuildEmblem(target));
		else
		{
			errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
			lua_pushboolean(L, false);
		}
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoCreatureSetGuildEmblem(lua_State* L)
{
	//doCreatureSetGuildEmblem(cid, emblem)
	GuildEmblems_t emblem = (GuildEmblems_t)popNumber(L);
	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		creature->setEmblem(emblem);
		g_game.updateCreatureEmblem(creature);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetCreaturePartyShield(lua_State* L)
{
	//getCreaturePartyShield(cid[, target])
	uint32_t tid = 0;
	if(lua_gettop(L) > 1)
		tid = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		if(!tid)
			lua_pushnumber(L, creature->getShield());
		else if(Creature* target = env->getCreatureByUID(tid))
			lua_pushnumber(L, creature->getPartyShield(target));
		else
		{
			errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
			lua_pushboolean(L, false);
		}
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoCreatureSetPartyShield(lua_State* L)
{
	//doCreatureSetPartyShield(cid, shield)
	PartyShields_t shield = (PartyShields_t)popNumber(L);
	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		creature->setShield(shield);
		g_game.updateCreatureShield(creature);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetCreatureSkullType(lua_State* L)
{
	//getCreatureSkullType(cid[, target])
	uint32_t tid = 0;
	if(lua_gettop(L) > 1)
		tid = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		if(!tid)
			lua_pushnumber(L, creature->getSkull());
		else if(Creature* target = env->getCreatureByUID(tid))
			lua_pushnumber(L, creature->getSkullType(target));
		else
		{
			errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
			lua_pushboolean(L, false);
		}
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoCreatureSetLookDir(lua_State* L)
{
	//doCreatureSetLookDirection(cid, dir)
	Direction dir = (Direction)popNumber(L);
	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		if(dir < NORTH || dir > WEST)
		{
			lua_pushboolean(L, false);
			return 1;
		}

		g_game.internalCreatureTurn(creature, dir);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoCreatureSetSkullType(lua_State* L)
{
	//doCreatureSetSkullType(cid, skull)
	Skulls_t skull = (Skulls_t)popNumber(L);
	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		creature->setSkull(skull);
		g_game.updateCreatureSkull(creature);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSetSkullEnd(lua_State* L)
{
	//doPlayerSetSkullEnd(cid, time, type)
	Skulls_t _skull = (Skulls_t)popNumber(L);
	time_t _time = (time_t)std::max((int64_t)0, popNumber(L));

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->setSkullEnd(_time, false, _skull);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetCreatureSpeed(lua_State* L)
{
	//getCreatureSpeed(cid)
	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
		lua_pushnumber(L, creature->getSpeed());
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetCreatureBaseSpeed(lua_State* L)
{
	//getCreatureBaseSpeed(cid)
	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
		lua_pushnumber(L, creature->getBaseSpeed());
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetCreatureTarget(lua_State* L)
{
	//getCreatureTarget(cid)
	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		Creature* target = creature->getAttackedCreature();
		lua_pushnumber(L, target ? env->addThing(target) : 0);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaIsSightClear(lua_State* L)
{
	//isSightClear(fromPos, toPos, floorCheck)
	PositionEx fromPos, toPos;
	bool floorCheck = popBoolean(L);

	popPosition(L, toPos);
	popPosition(L, fromPos);

	lua_pushboolean(L, g_game.isSightClear(fromPos, toPos, floorCheck));
	return 1;
}

int32_t LuaInterface::luaAddEvent(lua_State* L)
{
	//addEvent(callback, delay, ...)
	ScriptEnviroment* env = getEnv();
	LuaInterface* interface = env->getInterface();
	if(!interface)
	{
		errorEx("No valid script interface!");
		lua_pushboolean(L, false);
		return 1;
	}

	int32_t parameters = lua_gettop(L);
	if(!lua_isfunction(L, -parameters)) //-parameters means the first parameter from left to right
	{
		errorEx("Callback parameter should be a function");
		lua_pushboolean(L, false);
		return 1;
	}

	std::list<int32_t> params;
	for(int32_t i = 0; i < parameters - 2; ++i) //-2 because addEvent needs at least two parameters
		params.push_back(luaL_ref(L, LUA_REGISTRYINDEX));

	LuaTimerEvent event;
	event.eventId = Scheduler::getInstance().addEvent(createSchedulerTask(std::max((int64_t)SCHEDULER_MINTICKS, popNumber(L)),
		boost::bind(&LuaInterface::executeTimer, interface, ++interface->m_lastTimer)));

	event.parameters = params;
	event.function = luaL_ref(L, LUA_REGISTRYINDEX);
	event.scriptId = env->getScriptId();
	event.npc = env->getNpc();

	interface->m_timerEvents[interface->m_lastTimer] = event;
	lua_pushnumber(L, interface->m_lastTimer);
	return 1;
}

int32_t LuaInterface::luaStopEvent(lua_State* L)
{
	//stopEvent(eventid)
	uint32_t eventId = popNumber(L);
	ScriptEnviroment* env = getEnv();

	LuaInterface* interface = env->getInterface();
	if(!interface)
	{
		errorEx("No valid script interface!");
		lua_pushboolean(L, false);
		return 1;
	}

	LuaTimerEvents::iterator it = interface->m_timerEvents.find(eventId);
	if(it != interface->m_timerEvents.end())
	{
		Scheduler::getInstance().stopEvent(it->second.eventId);
		for(std::list<int32_t>::iterator lt = it->second.parameters.begin(); lt != it->second.parameters.end(); ++lt)
			luaL_unref(interface->m_luaState, LUA_REGISTRYINDEX, *lt);

		it->second.parameters.clear();
		luaL_unref(interface->m_luaState, LUA_REGISTRYINDEX, it->second.function);

		interface->m_timerEvents.erase(it);
		lua_pushboolean(L, true);
	}
	else
		lua_pushboolean(L, false);

	return 1;
}

int32_t LuaInterface::luaHasCreatureCondition(lua_State* L)
{
	//hasCreatureCondition(cid, conditionType[, subId = 0[, conditionId = (both)]])
	int32_t conditionId = CONDITIONID_COMBAT;
	uint32_t params = lua_gettop(L), subId = 0;

	bool both = true;
	if(params > 3)
	{
		conditionId = popNumber(L);
		both = false;
	}

	if(params > 2)
		subId = popNumber(L);

	ConditionType_t conditionType = (ConditionType_t)popNumber(L);
	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		if(!both)
			lua_pushboolean(L, creature->getCondition(conditionType, (ConditionId_t)conditionId, subId) != NULL);
		else if(creature->getCondition(conditionType, CONDITIONID_DEFAULT, subId) != NULL)
		{
			lua_pushboolean(L, true);
			return 1;
		}
		else
			lua_pushboolean(L, creature->getCondition(conditionType, CONDITIONID_COMBAT, subId) != NULL);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetCreatureConditionInfo(lua_State* L)
{
	//getCreatureConditionInfo(cid, conditionType[, subId = 0[, conditionId = CONDITIONID_COMBAT]])
	int32_t conditionId = CONDITIONID_COMBAT;
	uint32_t params = lua_gettop(L), subId = 0;
	if(params > 3)
		conditionId = popNumber(L);

	if(params > 2)
		subId = popNumber(L);

	ConditionType_t conditionType = (ConditionType_t)popNumber(L);
	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		if(Condition* condition = creature->getCondition(conditionType, (ConditionId_t)conditionId, subId))
		{
			lua_newtable(L);
			setField(L, "icons", condition->getIcons());
			setField(L, "endTime", condition->getEndTime());
			setField(L, "ticks", condition->getTicks());
			setFieldBool(L, "persistent", condition->isPersistent());
			setField(L, "subId", condition->getSubId());
		}
		else
			lua_pushboolean(L, false);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetPlayerBlessing(lua_State* L)
{
	//getPlayerBlessing(cid, blessing)
	int16_t blessing = popNumber(L) - 1;

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
		lua_pushboolean(L, player->hasBlessing(blessing));
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerAddBlessing(lua_State* L)
{
	//doPlayerAddBlessing(cid, blessing)
	int16_t blessing = popNumber(L) - 1;
	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		if(!player->hasBlessing(blessing))
		{
			player->addBlessing(1 << blessing);
			lua_pushboolean(L, true);
		}
		else
			lua_pushboolean(L, false);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetPlayerPVPBlessing(lua_State* L)
{
	//getPlayerPVPBlessing(cid)
	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
		lua_pushboolean(L, player->hasPVPBlessing());
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSetPVPBlessing(lua_State* L)
{
	//doPlayerSetPVPBlessing(cid[, value])
	bool value = true;
	if(lua_gettop(L) > 1)
		value = popBoolean(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->setPVPBlessing(value);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSetPromotionLevel(lua_State* L)
{
	//doPlayerSetPromotionLevel(cid, level)
	uint32_t level = popNumber(L);
	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->setPromotionLevel(level);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSetGroupId(lua_State* L)
{
	//doPlayerSetGroupId(cid, groupId)
	uint32_t groupId = popNumber(L);
	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		if(Group* group = Groups::getInstance()->getGroup(groupId))
		{
			player->setGroup(group);
			lua_pushboolean(L, true);
		}
		else
			lua_pushboolean(L, false);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetCreatureMana(lua_State* L)
{
	//getCreatureMana(cid)
	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
		lua_pushnumber(L, creature->getMana());
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetCreatureMaxMana(lua_State* L)
{
	//getCreatureMaxMana(cid[, ignoreModifiers = false])
	bool ignoreModifiers = false;
	if(lua_gettop(L) > 1)
		ignoreModifiers = popBoolean(L);

	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
		lua_pushnumber(L, creature->getPlayer() && ignoreModifiers ? creature->manaMax : creature->getMaxMana());
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetCreatureHealth(lua_State* L)
{
	//getCreatureHealth(cid)
	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
		lua_pushnumber(L, creature->getHealth());
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetCreatureLookDirection(lua_State* L)
{
	//getCreatureLookDirection(cid)
	ScriptEnviroment* env = getEnv();

	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
		lua_pushnumber(L, creature->getDirection());
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetCreatureMaxHealth(lua_State* L)
{
	//getCreatureMaxHealth(cid[, ignoreModifiers = false])
	bool ignoreModifiers = false;
	if(lua_gettop(L) > 1)
		ignoreModifiers = popBoolean(L);

	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
		lua_pushnumber(L, creature->getPlayer() && ignoreModifiers ? creature->healthMax : creature->getMaxHealth());
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSetStamina(lua_State* L)
{
	//doPlayerSetStamina(cid, minutes)
	uint32_t minutes = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->setStaminaMinutes(minutes);
		player->sendStats();
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSetBalance(lua_State* L)
{
	//doPlayerSetBalance(cid, balance)
	uint64_t balance = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->balance = balance;
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSetPartner(lua_State* L)
{
	//doPlayerSetPartner(cid, guid)
	uint32_t guid = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->marriage = guid;
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerFollowCreature(lua_State* L)
{
	//doPlayerFollowCreature(cid, target)
	ScriptEnviroment* env = getEnv();

	Creature* creature = env->getCreatureByUID(popNumber(L));
	if(!creature)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	Player* player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	lua_pushboolean(L, g_game.playerFollowCreature(player->getID(), creature->getID()));
	return 1;
}

int32_t LuaInterface::luaGetPlayerParty(lua_State* L)
{
	//getPlayerParty(cid)
	uint32_t cid = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(cid))
	{
		if(Party* party = player->getParty())
			lua_pushnumber(L, env->addThing(party->getLeader()));
		else
			lua_pushnil(L);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerJoinParty(lua_State* L)
{
	//doPlayerJoinParty(cid, lid)
	ScriptEnviroment* env = getEnv();

	Player* leader = env->getPlayerByUID(popNumber(L));
	if(!leader)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	Player* player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	g_game.playerJoinParty(player->getID(), leader->getID());
	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaDoPlayerLeaveParty(lua_State* L)
{
	//doPlayerLeaveParty(cid[, forced = false])
	bool forced = false;
	if(lua_gettop(L) > 1)
		forced = popBoolean(L);

	ScriptEnviroment* env = getEnv();
	Player* player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	g_game.playerLeaveParty(player->getID(), forced);
	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaIsPlayerUsingOtclient(lua_State* L)
{
	//isPlayerUsingOtclient(cid)
	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		lua_pushboolean(L, player->isUsingOtclient());
	}

	lua_pushboolean(L, false);
	return 1;
}

int32_t LuaInterface::luaDoSendPlayerExtendedOpcode(lua_State* L)
{
	//doPlayerSendExtendedOpcode(cid, opcode, buffer)
	std::string buffer = popString(L);
	int32_t opcode = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->sendExtendedOpcode(opcode, buffer);
		lua_pushboolean(L, true);
	}

	lua_pushboolean(L, false);
	return 1;
}

int32_t LuaInterface::luaGetPartyMembers(lua_State* L)
{
	//getPartyMembers(cid)
	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		if(Party* party = player->getParty())
		{
			PlayerVector list = party->getMembers();
			list.push_back(party->getLeader());

			PlayerVector::const_iterator it = list.begin();
			lua_newtable(L);
			for(uint32_t i = 1; it != list.end(); ++it, ++i)
			{
				lua_pushnumber(L, i);
				lua_pushnumber(L, (*it)->getID());
				pushTable(L);
			}

			return 1;
		}
	}

	lua_pushboolean(L, false);
	return 1;
}

int32_t LuaInterface::luaGetVocationInfo(lua_State* L)
{
	//getVocationInfo(id)
	uint32_t id = popNumber(L);
	Vocation* voc = Vocations::getInstance()->getVocation(id);
	if(!voc)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	lua_newtable(L);
	setField(L, "id", voc->getId());
	setField(L, "name", voc->getName().c_str());
	setField(L, "description", voc->getDescription().c_str());
	setField(L, "healthGain", voc->getGain(GAIN_HEALTH));
	setField(L, "healthGainTicks", voc->getGainTicks(GAIN_HEALTH));
	setField(L, "healthGainAmount", voc->getGainAmount(GAIN_HEALTH));
	setField(L, "manaGain", voc->getGain(GAIN_MANA));
	setField(L, "manaGainTicks", voc->getGainTicks(GAIN_MANA));
	setField(L, "manaGainAmount", voc->getGainAmount(GAIN_MANA));
	setField(L, "attackSpeed", voc->getAttackSpeed());
	setField(L, "baseSpeed", voc->getBaseSpeed());
	setField(L, "fromVocation", voc->getFromVocation());
	setField(L, "promotedVocation", Vocations::getInstance()->getPromotedVocation(id));
	setField(L, "soul", voc->getGain(GAIN_SOUL));
	setField(L, "soulAmount", voc->getGainAmount(GAIN_SOUL));
	setField(L, "soulTicks", voc->getGainTicks(GAIN_SOUL));
	setField(L, "capacity", voc->getGainCap());
	setFieldBool(L, "attackable", voc->isAttackable());
	setFieldBool(L, "needPremium", voc->isPremiumNeeded());
	setFieldFloat(L, "experienceMultiplier", voc->getExperienceMultiplier());
	return 1;
}

int32_t LuaInterface::luaGetGroupInfo(lua_State* L)
{
	//getGroupInfo(id[, premium = false])
	bool premium = false;
	if(lua_gettop(L) > 1)
		premium = popBoolean(L);

	Group* group = Groups::getInstance()->getGroup(popNumber(L));
	if(!group)
	{
		lua_pushboolean(L, false);
		return 1;
	}

	lua_newtable(L);
	setField(L, "id", group->getId());
	setField(L, "name", group->getName().c_str());
	setField(L, "access", group->getAccess());
	setField(L, "ghostAccess", group->getGhostAccess());
	setField(L, "flags", group->getFlags());
	setField(L, "customFlags", group->getCustomFlags());
	setField(L, "depotLimit", group->getDepotLimit(premium));
	setField(L, "maxVips", group->getMaxVips(premium));
	setField(L, "outfit", group->getOutfit());
	return 1;
}

int32_t LuaInterface::luaGetChannelUsers(lua_State* L)
{
	//getChannelUsers(channelId)
	ScriptEnviroment* env = getEnv();
	uint16_t channelId = popNumber(L);

	if(ChatChannel* channel = g_chat.getChannelById(channelId))
	{
		UsersMap usersMap = channel->getUsers();
		UsersMap::iterator it = usersMap.begin();

		lua_newtable(L);
		for(int32_t i = 1; it != usersMap.end(); ++it, ++i)
		{
			lua_pushnumber(L, i);
			lua_pushnumber(L, env->addThing(it->second));
			pushTable(L);
		}
	}
	else
		lua_pushboolean(L, false);

	return 1;
}

int32_t LuaInterface::luaGetPlayersOnline(lua_State* L)
{
	//getPlayersOnline()
	ScriptEnviroment* env = getEnv();
	AutoList<Player>::iterator it = Player::autoList.begin();

	lua_newtable(L);
	for(int32_t i = 1; it != Player::autoList.end(); ++it, ++i)
	{
		lua_pushnumber(L, i);
		lua_pushnumber(L, env->addThing(it->second));
		pushTable(L);
	}

	return 1;
}

int32_t LuaInterface::luaSetCreatureMaxHealth(lua_State* L)
{
	//setCreatureMaxHealth(uid, health)
	uint32_t maxHealth = (uint32_t)popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		creature->changeMaxHealth(maxHealth);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaSetCreatureMaxMana(lua_State* L)
{
	//setCreatureMaxMana(uid, mana)
	uint32_t maxMana = (uint32_t)popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		creature->changeMaxMana(maxMana);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSetMaxCapacity(lua_State* L)
{
	//doPlayerSetMaxCapacity(uid, cap)
	double cap = popFloatNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->setCapacity(cap);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetCreatureMaster(lua_State* L)
{
	//getCreatureMaster(cid)
	uint32_t cid = popNumber(L);
	ScriptEnviroment* env = getEnv();

	Creature* creature = env->getCreatureByUID(cid);
	if(!creature)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	if(Creature* master = creature->getMaster())
		lua_pushnumber(L, env->addThing(master));
	else
		lua_pushnil(L);

	return 1;
}

int32_t LuaInterface::luaGetCreatureSummons(lua_State* L)
{
	//getCreatureSummons(cid)
	ScriptEnviroment* env = getEnv();

	Creature* creature = env->getCreatureByUID(popNumber(L));
	if(!creature)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	const std::list<Creature*>& summons = creature->getSummons();
	CreatureList::const_iterator it = summons.begin();

	lua_newtable(L);
	for(uint32_t i = 1; it != summons.end(); ++it, ++i)
	{
		lua_pushnumber(L, i);
		lua_pushnumber(L, env->addThing(*it));
		pushTable(L);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSetIdleTime(lua_State* L)
{
	//doPlayerSetIdleTime(cid, amount)
	int64_t amount = popNumber(L);
	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->setIdleTime(amount);
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoCreatureSetNoMove(lua_State* L)
{
	//doCreatureSetNoMove(cid, block)
	bool block = popBoolean(L);

	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
	{
		creature->setNoMove(block);
		creature->onWalkAborted();
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetPlayerModes(lua_State* L)
{
	//getPlayerModes(cid)
	ScriptEnviroment* env = getEnv();

	Player* player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	lua_newtable(L);
	setField(L, "chase", player->getChaseMode());
	setField(L, "fight", player->getFightMode());
	setField(L, "secure", player->getSecureMode());
	return 1;
}

int32_t LuaInterface::luaGetPlayerRates(lua_State* L)
{
	//getPlayerRates(cid)
	ScriptEnviroment* env = getEnv();

	Player* player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	lua_newtable(L);
	for(uint32_t i = SKILL_FIRST; i <= SKILL__LAST; ++i)
	{
		lua_pushnumber(L, i);
		lua_pushnumber(L, player->rates[(skills_t)i]);
		pushTable(L);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSetRate(lua_State* L)
{
	//doPlayerSetRate(cid, type, value)
	double value = popFloatNumber(L);
	uint32_t type = popNumber(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		if(type <= SKILL__LAST)
		{
			player->rates[(skills_t)type] = value;
			lua_pushboolean(L, true);
		}
		else
			lua_pushboolean(L, false);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSwitchSaving(lua_State* L)
{
	//doPlayerSwitchSaving(cid)
	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->switchSaving();
		lua_pushboolean(L, true);
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoPlayerSave(lua_State* L)
{
	//doPlayerSave(cid[, shallow = false])
	bool shallow = false;
	if(lua_gettop(L) > 1)
		shallow = popBoolean(L);

	ScriptEnviroment* env = getEnv();
	if(Player* player = env->getPlayerByUID(popNumber(L)))
	{
		player->loginPosition = player->getPosition();
		lua_pushboolean(L, IOLoginData::getInstance()->savePlayer(player, false, shallow));
	}
	else
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaGetTownId(lua_State* L)
{
	//getTownId(townName)
	std::string townName = popString(L);
	if(Town* town = Towns::getInstance()->getTown(townName))
		lua_pushnumber(L, town->getID());
	else
		lua_pushboolean(L, false);

	return 1;
}

int32_t LuaInterface::luaGetTownName(lua_State* L)
{
	//getTownName(townId)
	uint32_t townId = popNumber(L);
	if(Town* town = Towns::getInstance()->getTown(townId))
		lua_pushstring(L, town->getName().c_str());
	else
		lua_pushboolean(L, false);

	return 1;
}

int32_t LuaInterface::luaGetTownTemplePosition(lua_State* L)
{
	//getTownTemplePosition(townId)
	uint32_t townId = popNumber(L);
	if(Town* town = Towns::getInstance()->getTown(townId))
		pushPosition(L, town->getPosition(), 255);
	else
		lua_pushboolean(L, false);

	return 1;
}

int32_t LuaInterface::luaGetTownHouses(lua_State* L)
{
	//getTownHouses([townId])
	uint32_t townId = 0;
	if(lua_gettop(L) > 0)
		townId = popNumber(L);

	HouseMap::iterator it = Houses::getInstance()->getHouseBegin();
	lua_newtable(L);
	for(uint32_t i = 1; it != Houses::getInstance()->getHouseEnd(); ++i, ++it)
	{
		if(townId && it->second->getTownId() != townId)
			continue;

		lua_pushnumber(L, i);
		lua_pushnumber(L, it->second->getId());
		pushTable(L);
	}

	return 1;
}

int32_t LuaInterface::luaGetSpectators(lua_State* L)
{
	//getSpectators(centerPos, rangex, rangey[, multifloor = false])
	bool multifloor = false;
	if(lua_gettop(L) > 3)
		multifloor = popBoolean(L);

	uint32_t rangey = popNumber(L), rangex = popNumber(L);
	PositionEx centerPos;
	popPosition(L, centerPos);

	SpectatorVec list;
	g_game.getSpectators(list, centerPos, false, multifloor, rangex, rangex, rangey, rangey);
	if(list.empty())
	{
		lua_pushnil(L);
		return 1;
	}

	ScriptEnviroment* env = getEnv();
	SpectatorVec::const_iterator it = list.begin();

	lua_newtable(L);
	for(uint32_t i = 1; it != list.end(); ++it, ++i)
	{
		lua_pushnumber(L, i);
		lua_pushnumber(L, env->addThing(*it));
		pushTable(L);
	}

	return 1;
}

int32_t LuaInterface::luaGetHighscoreString(lua_State* L)
{
	//getHighscoreString(skillId)
	uint16_t skillId = popNumber(L);
	if(skillId <= SKILL__LAST)
		lua_pushstring(L, g_game.getHighscoreString(skillId).c_str());
	else
		lua_pushboolean(L, false);

	return 1;
}

int32_t LuaInterface::luaGetVocationList(lua_State* L)
{
	//getVocationList()
	VocationsMap::iterator it = Vocations::getInstance()->getFirstVocation();
	lua_newtable(L);
	for(uint32_t i = 1; it != Vocations::getInstance()->getLastVocation(); ++i, ++it)
	{
		createTable(L, i);
		setField(L, "id", it->first);
		setField(L, "name", it->second->getName());
		pushTable(L);
	}

	return 1;
}

int32_t LuaInterface::luaGetGroupList(lua_State* L)
{
	//getGroupList()
	GroupsMap::iterator it = Groups::getInstance()->getFirstGroup();
	lua_newtable(L);
	for(uint32_t i = 1; it != Groups::getInstance()->getLastGroup(); ++i, ++it)
	{
		createTable(L, i);
		setField(L, "id", it->first);
		setField(L, "name", it->second->getName());
		pushTable(L);
	}

	return 1;
}

int32_t LuaInterface::luaGetChannelList(lua_State* L)
{
	//getChannelList()
	lua_newtable(L);
	ChannelList list = g_chat.getPublicChannels();

	ChannelList::const_iterator it = list.begin();
	for(uint32_t i = 1; it != list.end(); ++it, ++i)
	{
		createTable(L, i);
		setField(L, "id", (*it)->getId());
		setField(L, "name", (*it)->getName());

		setField(L, "flags", (*it)->getFlags());
		setField(L, "level", (*it)->getLevel());
		setField(L, "access", (*it)->getAccess());
		pushTable(L);
	}

	return 1;
}

int32_t LuaInterface::luaGetTownList(lua_State* L)
{
	//getTownList()
	TownMap::const_iterator it = Towns::getInstance()->getFirstTown();
	lua_newtable(L);
	for(uint32_t i = 1; it != Towns::getInstance()->getLastTown(); ++it, ++i)
	{
		createTable(L, i);
		setField(L, "id", it->first);
		setField(L, "name", it->second->getName());
		pushTable(L);
	}

	return 1;
}

int32_t LuaInterface::luaGetWaypointList(lua_State* L)
{
	//getWaypointList()
	WaypointMap waypointsMap = g_game.getMap()->waypoints.getWaypointsMap();
	WaypointMap::iterator it = waypointsMap.begin();

	lua_newtable(L);
	for(uint32_t i = 1; it != waypointsMap.end(); ++it, ++i)
	{
		createTable(L, i);
		setField(L, "name", it->first);
		setField(L, "pos", it->second->pos.x);
		pushTable(L);
	}

	return 1;
}

int32_t LuaInterface::luaGetWaypointPosition(lua_State* L)
{
	//getWaypointPosition(name)
	if(WaypointPtr waypoint = g_game.getMap()->waypoints.getWaypointByName(popString(L)))
		pushPosition(L, waypoint->pos, 0);
	else
		lua_pushboolean(L, false);

	return 1;
}

int32_t LuaInterface::luaDoWaypointAddTemporial(lua_State* L)
{
	//doWaypointAddTemporial(name, pos)
	PositionEx pos;
	popPosition(L, pos);

	g_game.getMap()->waypoints.addWaypoint(WaypointPtr(new Waypoint(popString(L), pos)));
	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaGetGameState(lua_State* L)
{
	//getGameState()
	lua_pushnumber(L, g_game.getGameState());
	return 1;
}

int32_t LuaInterface::luaDoSetGameState(lua_State* L)
{
	//doSetGameState(id)
	uint32_t id = popNumber(L);
	if(id >= GAMESTATE_FIRST && id <= GAMESTATE_LAST)
	{
		Dispatcher::getInstance().addTask(createTask(
			boost::bind(&Game::setGameState, &g_game, (GameState_t)id)));
		lua_pushboolean(L, true);
	}
	else
		lua_pushboolean(L, false);

	return 1;
}

int32_t LuaInterface::luaDoCreatureExecuteTalkAction(lua_State* L)
{
	//doCreatureExecuteTalkAction(cid, text[, ignoreAccess = false[, channelId = CHANNEL_DEFAULT]])
	uint32_t params = lua_gettop(L), channelId = CHANNEL_DEFAULT;
	if(params > 3)
		channelId = popNumber(L);

	bool ignoreAccess = false;
	if(params > 2)
		ignoreAccess = popBoolean(L);

	std::string text = popString(L);
	ScriptEnviroment* env = getEnv();
	if(Creature* creature = env->getCreatureByUID(popNumber(L)))
		lua_pushboolean(L, g_talkActions->onPlayerSay(creature, channelId, text, ignoreAccess));
	else
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
	}

	return 1;
}

int32_t LuaInterface::luaDoExecuteRaid(lua_State* L)
{
	//doExecuteRaid(name)
	std::string raidName = popString(L);
	if(Raids::getInstance()->getRunning())
	{
		lua_pushboolean(L, false);
		return 1;
	}

	Raid* raid = Raids::getInstance()->getRaidByName(raidName);
	if(!raid || !raid->isLoaded())
	{
		errorEx("Raid with name " + raidName + " does not exists");
		lua_pushboolean(L, false);
		return 1;
	}

	lua_pushboolean(L, raid->startRaid());
	return 1;
}

int32_t LuaInterface::luaDoReloadInfo(lua_State* L)
{
	//doReloadInfo(id[, cid])
	uint32_t cid = 0;
	if(lua_gettop(L) > 1)
		cid = popNumber(L);

	uint32_t id = popNumber(L);
	if(id >= RELOAD_FIRST && id <= RELOAD_LAST)
	{
		// we're passing it to scheduler since talkactions reload will
		// re-init our lua state and crash due to unfinished call
		Scheduler::getInstance().addEvent(createSchedulerTask(SCHEDULER_MINTICKS,
			boost::bind(&Game::reloadInfo, &g_game, (ReloadInfo_t)id, cid, false)));
		lua_pushboolean(L, true);
	}
	else
		lua_pushboolean(L, false);

	return 1;
}

int32_t LuaInterface::luaDoSaveServer(lua_State* L)
{
	//doSaveServer([flags = 13])
	uint8_t flags = 13;
	if(lua_gettop(L) > 0)
		flags = popNumber(L);

	Dispatcher::getInstance().addTask(createTask(boost::bind(&Game::saveGameState, &g_game, flags)));
	lua_pushnil(L);
	return 1;
}

int32_t LuaInterface::luaDoSaveHouse(lua_State* L)
{
	//doSaveHouse({list})
	IntegerVec list;
	if(lua_istable(L, -1))
	{
		lua_pushnil(L);
		while(lua_next(L, -2))
			list.push_back(popNumber(L));

		lua_pop(L, 2);
	}
	else
		list.push_back(popNumber(L));

	House* house;
	std::vector<House*> houses;
	for(IntegerVec::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		if(!(house = Houses::getInstance()->getHouse(*it)))
		{
			std::stringstream s;
			s << "House not found, ID: " << (*it);
			errorEx(s.str());

			lua_pushboolean(L, false);
			return 1;
		}

		houses.push_back(house);
	}

	Database* db = Database::getInstance();
	DBTransaction trans(db);
	if(!trans.begin())
	{
		lua_pushboolean(L, false);
		return 1;
	}

	for(std::vector<House*>::iterator it = houses.begin(); it != houses.end(); ++it)
	{
		if(!IOMapSerialize::getInstance()->saveHouse(db, *it))
		{
			std::stringstream s;
			s << "Unable to save house information, ID: " << (*it)->getId();
			errorEx(s.str());
		}

		if(!IOMapSerialize::getInstance()->saveHouseItems(db, *it))
		{
			std::stringstream s;
			s << "Unable to save house items, ID: " << (*it)->getId();
			errorEx(s.str());
		}
	}

	lua_pushboolean(L, trans.commit());
	return 1;
}

int32_t LuaInterface::luaDoCleanHouse(lua_State* L)
{
	//doCleanHouse(houseId)
	uint32_t houseId = popNumber(L);
	if(House* house = Houses::getInstance()->getHouse(houseId))
	{
		house->clean();
		lua_pushboolean(L, true);
	}
	else
		lua_pushboolean(L, false);

	return 1;
}

int32_t LuaInterface::luaDoCleanMap(lua_State* L)
{
	//doCleanMap()
	uint32_t count = 0;
	g_game.cleanMapEx(count);
	lua_pushnumber(L, count);
	return 1;
}

int32_t LuaInterface::luaDoRefreshMap(lua_State* L)
{
	//doRefreshMap()
	g_game.proceduralRefresh();
	lua_pushnil(L);
	return 1;
}

int32_t LuaInterface::luaDoUpdateHouseAuctions(lua_State* L)
{
	//doUpdateHouseAuctions()
	lua_pushboolean(L, g_game.getMap()->updateAuctions());
	return 1;
}

int32_t LuaInterface::luaGetItemIdByName(lua_State* L)
{
	//getItemIdByName(name)
	int32_t itemId = Item::items.getItemIdByName(popString(L));
	if(itemId == -1)
	{
		errorEx(getError(LUA_ERROR_ITEM_NOT_FOUND));
		lua_pushboolean(L, false);
	}
	else
		lua_pushnumber(L, itemId);

	return 1;
}

int32_t LuaInterface::luaGetItemInfo(lua_State* L)
{
	//getItemInfo(itemid)
	const ItemType* item;
	if(!(item = Item::items.getElement(popNumber(L))))
	{
		lua_pushboolean(L, false);
		return 1;
	}

	lua_newtable(L);
	setFieldBool(L, "stopTime", item->stopTime);
	setFieldBool(L, "showCount", item->showCount);
	setFieldBool(L, "stackable", item->stackable);
	setFieldBool(L, "showDuration", item->showDuration);
	setFieldBool(L, "showCharges", item->showCharges);
	setFieldBool(L, "showAttributes", item->showAttributes);
	setFieldBool(L, "distRead", item->allowDistRead);
	setFieldBool(L, "readable", item->canReadText);
	setFieldBool(L, "writable", item->canWriteText);
	setFieldBool(L, "forceSerialize", item->forceSerialize);
	setFieldBool(L, "vertical", item->isVertical);
	setFieldBool(L, "horizontal", item->isHorizontal);
	setFieldBool(L, "hangable", item->isHangable);
	setFieldBool(L, "usable", item->usable);
	setFieldBool(L, "movable", item->movable);
	setFieldBool(L, "pickupable", item->pickupable);
	setFieldBool(L, "rotable", item->rotable);
	setFieldBool(L, "replacable", item->replacable);
	setFieldBool(L, "hasHeight", item->hasHeight);
	setFieldBool(L, "blockSolid", item->blockSolid);
	setFieldBool(L, "blockPickupable", item->blockPickupable);
	setFieldBool(L, "blockProjectile", item->blockProjectile);
	setFieldBool(L, "blockPathing", item->blockPathFind);
	setFieldBool(L, "allowPickupable", item->allowPickupable);
	setFieldBool(L, "alwaysOnTop", item->alwaysOnTop);

	createTable(L, "floorChange");
	for(int32_t i = CHANGE_FIRST; i <= CHANGE_LAST; ++i)
	{
		lua_pushnumber(L, i);
		lua_pushboolean(L, item->floorChange[i - 1]);
		pushTable(L);
	}

	pushTable(L);
	setField(L, "magicEffect", (int32_t)item->magicEffect);
	setField(L, "fluidSource", (int32_t)item->fluidSource);
	setField(L, "weaponType", (int32_t)item->weaponType);
	setField(L, "bedPartnerDirection", (int32_t)item->bedPartnerDir);
	setField(L, "ammoAction", (int32_t)item->ammoAction);
	setField(L, "combatType", (int32_t)item->combatType);
	setField(L, "corpseType", (int32_t)item->corpseType);
	setField(L, "shootType", (int32_t)item->shootType);
	setField(L, "ammoType", (int32_t)item->ammoType);

	createTable(L, "transformBed");
	setField(L, "female", item->transformBed[PLAYERSEX_FEMALE]);
	setField(L, "male", item->transformBed[PLAYERSEX_MALE]);

	pushTable(L);
	setField(L, "transformUseTo", item->transformUseTo);
	setField(L, "transformEquipTo", item->transformEquipTo);
	setField(L, "transformDeEquipTo", item->transformDeEquipTo);
	setField(L, "clientId", item->clientId);
	setField(L, "maxItems", item->maxItems);
	setField(L, "slotPosition", item->slotPosition);
	setField(L, "wieldPosition", item->wieldPosition);
	setField(L, "speed", item->speed);
	setField(L, "maxTextLength", item->maxTextLength);
	setField(L, "writeOnceItemId", item->writeOnceItemId);
	setField(L, "date", item->date);
	setField(L, "writer", item->writer);
	setField(L, "text", item->text);
	setField(L, "attack", item->attack);
	setField(L, "extraAttack", item->extraAttack);
	setField(L, "defense", item->defense);
	setField(L, "extraDefense", item->extraDefense);
	setField(L, "armor", item->armor);
	setField(L, "breakChance", item->breakChance);
	setField(L, "hitChance", item->hitChance);
	setField(L, "maxHitChance", item->maxHitChance);
	setField(L, "runeLevel", item->runeLevel);
	setField(L, "runeMagicLevel", item->runeMagLevel);
	setField(L, "lightLevel", item->lightLevel);
	setField(L, "lightColor", item->lightColor);
	setField(L, "decayTo", item->decayTo);
	setField(L, "rotateTo", item->rotateTo);
	setField(L, "alwaysOnTopOrder", item->alwaysOnTopOrder);
	setField(L, "shootRange", item->shootRange);
	setField(L, "charges", item->charges);
	setField(L, "decayTime", item->decayTime);
	setField(L, "attackSpeed", item->attackSpeed);
	setField(L, "wieldInfo", item->wieldInfo);
	setField(L, "minRequiredLevel", item->minReqLevel);
	setField(L, "minRequiredMagicLevel", item->minReqMagicLevel);
	setField(L, "worth", item->worth);
	setField(L, "levelDoor", item->levelDoor);
	setFieldBool(L, "specialDoor", item->specialDoor);
	setFieldBool(L, "closingDoor", item->closingDoor);
	setField(L, "name", item->name.c_str());
	setField(L, "plural", item->pluralName.c_str());
	setField(L, "article", item->article.c_str());
	setField(L, "description", item->description.c_str());
	setField(L, "runeSpellName", item->runeSpellName.c_str());
	setField(L, "vocationString", item->vocationString.c_str());

	createTable(L, "abilities");
	setFieldBool(L, "manaShield", item->hasAbilities() ? item->abilities->manaShield : false);
	setFieldBool(L, "invisible", item->hasAbilities() ? item->abilities->invisible : false);
	setFieldBool(L, "regeneration", item->hasAbilities() ? item->abilities->regeneration : false);
	setFieldBool(L, "preventLoss", item->hasAbilities() ? item->abilities->preventLoss : false);
	setFieldBool(L, "preventDrop", item->hasAbilities() ? item->abilities->preventDrop : false);
	setField(L, "elementType", (int32_t)item->hasAbilities() ? item->abilities->elementType : 0);
	setField(L, "elementDamage", item->hasAbilities() ? item->abilities->elementDamage : 0);
	setField(L, "speed", item->hasAbilities() ? item->abilities->speed : 0);
	setField(L, "healthGain", item->hasAbilities() ? item->abilities->healthGain : 0);
	setField(L, "healthTicks", item->hasAbilities() ? item->abilities->healthTicks : 0);
	setField(L, "manaGain", item->hasAbilities() ? item->abilities->manaGain : 0);
	setField(L, "manaTicks", item->hasAbilities() ? item->abilities->manaTicks : 0);
	setField(L, "conditionSuppressions", item->hasAbilities() ? item->abilities->conditionSuppressions : 0);

	//TODO: absorb, increment, reflect, skills, skillsPercent, stats, statsPercent

	pushTable(L);
	setField(L, "group", (int32_t)item->group);
	setField(L, "type", (int32_t)item->type);
	setFieldFloat(L, "weight", item->weight);
	return 1;
}

int32_t LuaInterface::luaGetItemAttribute(lua_State* L)
{
	//getItemAttribute(uid, key)
	std::string key = popString(L);
	ScriptEnviroment* env = getEnv();

	Item* item = env->getItemByUID(popNumber(L));
	if(!item)
	{
		errorEx(getError(LUA_ERROR_ITEM_NOT_FOUND));
		lua_pushnil(L);
		return 1;
	}

	boost::any value = item->getAttribute(key.c_str());
	if(value.empty())
		lua_pushnil(L);
	else if(value.type() == typeid(std::string))
		lua_pushstring(L, boost::any_cast<std::string>(value).c_str());
	else if(value.type() == typeid(int32_t))
		lua_pushnumber(L, boost::any_cast<int32_t>(value));
	else if(value.type() == typeid(float))
		lua_pushnumber(L, boost::any_cast<float>(value));
	else if(value.type() == typeid(bool))
		lua_pushboolean(L, boost::any_cast<bool>(value));
	else
		lua_pushnil(L);

	return 1;
}

int32_t LuaInterface::luaDoItemSetAttribute(lua_State* L)
{
	//doItemSetAttribute(uid, key, value)
	boost::any value;
	if(lua_isnumber(L, -1))
	{
		double tmp = popFloatNumber(L);
		if(std::floor(tmp) < tmp)
			value = tmp;
		else
			value = (int32_t)tmp;
	}
	else if(lua_isboolean(L, -1))
		value = popBoolean(L);
	else if(lua_isstring(L, -1))
		value = popString(L);
	else
	{
		lua_pop(L, 1);
		errorEx("Invalid data type");

		lua_pushboolean(L, false);
		return 1;
	}

	std::string key = popString(L);
	ScriptEnviroment* env = getEnv();

	Item* item = env->getItemByUID(popNumber(L));
	if(!item)
	{
		errorEx(getError(LUA_ERROR_ITEM_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	if(value.type() == typeid(int32_t))
	{
		if(key == "uid")
		{
			int32_t tmp = boost::any_cast<int32_t>(value);
			if(tmp < 1000 || tmp > 0xFFFF)
			{
				errorEx("Value for protected key \"uid\" must be in range of 1000 to 65535");
				lua_pushboolean(L, false);
				return 1;
			}

			item->setUniqueId(tmp);
		}
		else if(key == "aid")
			item->setActionId(boost::any_cast<int32_t>(value));
		else
			item->setAttribute(key.c_str(), boost::any_cast<int32_t>(value));
	}
	else
		item->setAttribute(key.c_str(), value);

	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaDoItemEraseAttribute(lua_State* L)
{
	//doItemEraseAttribute(uid, key)
	std::string key = popString(L);
	ScriptEnviroment* env = getEnv();

	Item* item = env->getItemByUID(popNumber(L));
	if(!item)
	{
		errorEx(getError(LUA_ERROR_ITEM_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	bool ret = true;
	if(key == "uid")
	{
		errorEx("Attempt to erase protected key \"uid\"");
		ret = false;
	}
	else if(key != "aid")
		item->eraseAttribute(key.c_str());
	else
		item->resetActionId();

	lua_pushboolean(L, ret);
	return 1;
}

int32_t LuaInterface::luaGetItemWeight(lua_State* L)
{
	//getItemWeight(itemid[, precise = true])
	bool precise = true;
	if(lua_gettop(L) > 2)
		precise = popBoolean(L);

	ScriptEnviroment* env = getEnv();
	Item* item = env->getItemByUID(popNumber(L));
	if(!item)
	{
		errorEx(getError(LUA_ERROR_ITEM_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	double weight = item->getWeight();
	if(precise)
	{
		std::stringstream ws;
		ws << std::fixed << std::setprecision(2) << weight;
		weight = atof(ws.str().c_str());
	}

	lua_pushnumber(L, weight);
	return 1;
}

int32_t LuaInterface::luaGetItemParent(lua_State* L)
{
	//getItemParent(uid)
	ScriptEnviroment* env = getEnv();

	Thing* thing = env->getThingByUID(popNumber(L));
	if(!thing)
	{
		errorEx(getError(LUA_ERROR_THING_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	if(!thing->getParent())
	{
		pushThing(L, NULL, 0);
		return 1;
	}

	if(Tile* tile = thing->getParent()->getTile())
	{
		if(tile->ground)
			pushThing(L, tile->ground, env->addThing(tile->ground));
		else
			pushThing(L, NULL, 0);
	}
	if(Item* container = thing->getParent()->getItem())
		pushThing(L, container, env->addThing(container));
	else if(Creature* creature = thing->getParent()->getCreature())
		pushThing(L, creature, env->addThing(creature));
	else
		pushThing(L, NULL, 0);

	return 1;
}

int32_t LuaInterface::luaHasItemProperty(lua_State* L)
{
	//hasItemProperty(uid, prop)
	uint32_t prop = popNumber(L);
	ScriptEnviroment* env = getEnv();

	Item* item = env->getItemByUID(popNumber(L));
	if(!item)
	{
		errorEx(getError(LUA_ERROR_ITEM_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	//Check if the item is a tile, so we can get more accurate properties
	bool tmp = item->hasProperty((ITEMPROPERTY)prop);
	if(item->getTile() && item->getTile()->ground == item)
		tmp = item->getTile()->hasProperty((ITEMPROPERTY)prop);

	lua_pushboolean(L, tmp);
	return 1;
}

int32_t LuaInterface::luaHasMonsterRaid(lua_State* L)
{
	//hasMonsterRaid(cid)
	ScriptEnviroment* env = getEnv();

	Creature* creature = env->getCreatureByUID(popNumber(L));
	if(!creature)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	Monster* monster = creature->getMonster();
	if(!monster)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	lua_pushboolean(L, monster->hasRaid());
	return 1;
}

int32_t LuaInterface::luaIsIpBanished(lua_State* L)
{
	//isIpBanished(ip[, mask])
	uint32_t mask = 0xFFFFFFFF;
	if(lua_gettop(L) > 1)
		mask = popNumber(L);

	lua_pushboolean(L, IOBan::getInstance()->isIpBanished((uint32_t)popNumber(L), mask));
	return 1;
}

int32_t LuaInterface::luaIsPlayerBanished(lua_State* L)
{
	//isPlayerBanished(name/guid, type)
	PlayerBan_t type = (PlayerBan_t)popNumber(L);
	if(lua_isnumber(L, -1))
		lua_pushboolean(L, IOBan::getInstance()->isPlayerBanished((uint32_t)popNumber(L), type));
	else
		lua_pushboolean(L, IOBan::getInstance()->isPlayerBanished(popString(L), type));

	return 1;
}

int32_t LuaInterface::luaIsAccountBanished(lua_State* L)
{
	//isAccountBanished(accountId[, playerId])
	uint32_t playerId = 0;
	if(lua_gettop(L) > 1)
		playerId = popNumber(L);

	lua_pushboolean(L, IOBan::getInstance()->isAccountBanished((uint32_t)popNumber(L), playerId));
	return 1;
}

int32_t LuaInterface::luaDoAddIpBanishment(lua_State* L)
{
	//doAddIpBanishment(ip[, mask[, length[, reason[, comment[, admin[, statement]]]]]])
	uint32_t admin = 0, reason = 21, mask = 0xFFFFFFFF, params = lua_gettop(L);
	int64_t length = time(NULL) + g_config.getNumber(ConfigManager::IPBAN_LENGTH);
	std::string statement, comment;

	if(params > 6)
		statement = popString(L);

	if(params > 5)
		admin = popNumber(L);

	if(params > 4)
		comment = popString(L);

	if(params > 3)
		reason = popNumber(L);

	if(params > 2)
		length = popNumber(L);

	if(params > 1)
		mask = popNumber(L);

	lua_pushboolean(L, IOBan::getInstance()->addIpBanishment((uint32_t)popNumber(L),
		length, reason, comment, admin, mask, statement));
	return 1;
}

int32_t LuaInterface::luaDoAddPlayerBanishment(lua_State* L)
{
	//doAddPlayerBanishment(name/guid[, type[, length[, reason[, action[, comment[, admin[, statement]]]]]]])
	uint32_t admin = 0, reason = 21, params = lua_gettop(L);
	int64_t length = -1;
	std::string statement, comment;

	ViolationAction_t action = ACTION_NAMELOCK;
	PlayerBan_t type = PLAYERBAN_LOCK;
	if(params > 7)
		statement = popString(L);

	if(params > 6)
		admin = popNumber(L);

	if(params > 5)
		comment = popString(L);

	if(params > 4)
		action = (ViolationAction_t)popNumber(L);

	if(params > 3)
		reason = popNumber(L);

	if(params > 2)
		length = popNumber(L);

	if(params > 1)
		type = (PlayerBan_t)popNumber(L);

	if(lua_isnumber(L, -1))
		lua_pushboolean(L, IOBan::getInstance()->addPlayerBanishment((uint32_t)popNumber(L),
			length, reason, action, comment, admin, type, statement));
	else
		lua_pushboolean(L, IOBan::getInstance()->addPlayerBanishment(popString(L),
			length, reason, action, comment, admin, type, statement));

	return 1;
}

int32_t LuaInterface::luaDoAddAccountBanishment(lua_State* L)
{
	//doAddAccountBanishment(accountId[, playerId[, length[, reason[, action[, comment[, admin[, statement]]]]]]])
	uint32_t admin = 0, reason = 21, playerId = 0, params = lua_gettop(L);
	int64_t length = time(NULL) + g_config.getNumber(ConfigManager::BAN_LENGTH);
	std::string statement, comment;

	ViolationAction_t action = ACTION_BANISHMENT;
	if(params > 7)
		statement = popString(L);

	if(params > 6)
		admin = popNumber(L);

	if(params > 5)
		comment = popString(L);

	if(params > 4)
		action = (ViolationAction_t)popNumber(L);

	if(params > 3)

		reason = popNumber(L);

	if(params > 2)
		length = popNumber(L);

	if(params > 1)
		playerId = popNumber(L);

	lua_pushboolean(L, IOBan::getInstance()->addAccountBanishment((uint32_t)popNumber(L),
		length, reason, action, comment, admin, playerId, statement));
	return 1;
}

int32_t LuaInterface::luaDoAddAccountWarnings(lua_State* L)
{
	//doAddAccountWarnings(accountId[, warnings])
	uint32_t warnings = 1;
	int32_t params = lua_gettop(L);
	if(params > 1)
		warnings = popNumber(L);

	Account account = IOLoginData::getInstance()->loadAccount(popNumber(L), true);
	account.warnings += warnings;
	IOLoginData::getInstance()->saveAccount(account);
	lua_pushboolean(L, true);
	return 1;
}

int32_t LuaInterface::luaDoAddNotation(lua_State* L)
{
	//doAddNotation(accountId[, playerId[, reason[, comment[, admin[, statement]]]]]])
	uint32_t admin = 0, reason = 21, playerId = 0, params = lua_gettop(L);
	std::string statement, comment;

	if(params > 5)
		statement = popString(L);

	if(params > 4)
		admin = popNumber(L);

	if(params > 3)
		comment = popString(L);

	if(params > 2)
		reason = popNumber(L);

	if(params > 1)
		playerId = popNumber(L);

	lua_pushboolean(L, IOBan::getInstance()->addNotation((uint32_t)popNumber(L),
		reason, comment, admin, playerId, statement));
	return 1;
}

int32_t LuaInterface::luaDoAddStatement(lua_State* L)
{
	//doAddStatement(name/guid[, channelId[, reason[, comment[, admin[, statement]]]]]])
	uint32_t admin = 0, reason = 21, params = lua_gettop(L);
	int16_t channelId = -1;
	std::string statement, comment;

	if(params > 5)
		statement = popString(L);

	if(params > 4)
		admin = popNumber(L);

	if(params > 3)
		comment = popString(L);

	if(params > 2)
		reason = popNumber(L);

	if(params > 1)
		channelId = popNumber(L);

	if(lua_isnumber(L, -1))
		lua_pushboolean(L, IOBan::getInstance()->addStatement((uint32_t)popNumber(L),
			reason, comment, admin, channelId, statement));
	else
		lua_pushboolean(L, IOBan::getInstance()->addStatement(popString(L),
			reason, comment, admin, channelId, statement));

	return 1;
}

int32_t LuaInterface::luaDoRemoveIpBanishment(lua_State* L)
{
	//doRemoveIpBanishment(ip[, mask])
	uint32_t mask = 0xFFFFFFFF;
	if(lua_gettop(L) > 1)
		mask = popNumber(L);

	lua_pushboolean(L, IOBan::getInstance()->removeIpBanishment(
		(uint32_t)popNumber(L), mask));
	return 1;
}

int32_t LuaInterface::luaDoRemovePlayerBanishment(lua_State* L)
{
	//doRemovePlayerBanishment(name/guid, type)
	PlayerBan_t type = (PlayerBan_t)popNumber(L);
	if(lua_isnumber(L, -1))
		lua_pushboolean(L, IOBan::getInstance()->removePlayerBanishment((uint32_t)popNumber(L), type));
	else
		lua_pushboolean(L, IOBan::getInstance()->removePlayerBanishment(popString(L), type));

	return 1;
}

int32_t LuaInterface::luaDoRemoveAccountBanishment(lua_State* L)
{
	//doRemoveAccountBanishment(accountId[, playerId])
	uint32_t playerId = 0;
	if(lua_gettop(L) > 1)
		playerId = popNumber(L);

	lua_pushboolean(L, IOBan::getInstance()->removeAccountBanishment((uint32_t)popNumber(L), playerId));
	return 1;
}

int32_t LuaInterface::luaDoRemoveNotations(lua_State* L)
{
	//doRemoveNotations(accountId[, playerId])
	uint32_t playerId = 0;
	if(lua_gettop(L) > 1)
		playerId = popNumber(L);

	lua_pushboolean(L, IOBan::getInstance()->removeNotations((uint32_t)popNumber(L), playerId));
	return 1;
}

int32_t LuaInterface::luaGetAccountWarnings(lua_State* L)
{
	//getAccountWarnings(accountId)
	Account account = IOLoginData::getInstance()->loadAccount(popNumber(L));
	lua_pushnumber(L, account.warnings);
	return 1;
}

int32_t LuaInterface::luaGetNotationsCount(lua_State* L)
{
	//getNotationsCount(accountId[, playerId])
	uint32_t playerId = 0;
	if(lua_gettop(L) > 1)
		playerId = popNumber(L);

	lua_pushnumber(L, IOBan::getInstance()->getNotationsCount((uint32_t)popNumber(L), playerId));
	return 1;
}

int32_t LuaInterface::luaGetBanData(lua_State* L)
{
	//getBanData(value[, type[, param]])
	Ban tmp;
	uint32_t params = lua_gettop(L);
	if(params > 2)
		tmp.param = popNumber(L);

	if(params > 1)
		tmp.type = (Ban_t)popNumber(L);

	tmp.value = popNumber(L);
	if(!IOBan::getInstance()->getData(tmp))
	{
		lua_pushboolean(L, false);
		return 1;
	}

	lua_newtable(L);
	setField(L, "id", tmp.id);
	setField(L, "type", tmp.type);
	setField(L, "value", tmp.value);
	setField(L, "param", tmp.param);
	setField(L, "added", tmp.added);
	setField(L, "expires", tmp.expires);
	setField(L, "adminId", tmp.adminId);
	setField(L, "comment", tmp.comment);
	return 1;
}

int32_t LuaInterface::luaGetBanList(lua_State* L)
{
	//getBanList(type[, value[, param]])
	int32_t param = 0, params = lua_gettop(L);
	if(params > 2)
		param = popNumber(L);

	uint32_t value = 0;
	if(params > 1)
		value = popNumber(L);

	BansVec bans = IOBan::getInstance()->getList((Ban_t)popNumber(L), value, param);
	BansVec::const_iterator it = bans.begin();

	lua_newtable(L);
	for(uint32_t i = 1; it != bans.end(); ++it, ++i)
	{
		createTable(L, i);
		setField(L, "id", it->id);
		setField(L, "type", it->type);
		setField(L, "value", it->value);
		setField(L, "param", it->param);
		setField(L, "added", it->added);
		setField(L, "expires", it->expires);
		setField(L, "adminId", it->adminId);
		setField(L, "comment", it->comment);
		pushTable(L);
	}

	return 1;
}

int32_t LuaInterface::luaGetExperienceStage(lua_State* L)
{
	//getExperienceStage(level[, divider])
	double divider = 1.0f;
	if(lua_gettop(L) > 1)
		divider = popFloatNumber(L);

	lua_pushnumber(L, g_game.getExperienceStage(popNumber(L), divider));
	return 1;
}

int32_t LuaInterface::luaGetDataDir(lua_State* L)
{
	//getDataDir()
	lua_pushstring(L, getFilePath(FILE_TYPE_OTHER, "").c_str());
	return 1;
}

int32_t LuaInterface::luaGetLogsDir(lua_State* L)
{
	//getLogsDir()
	lua_pushstring(L, getFilePath(FILE_TYPE_LOG, "").c_str());
	return 1;
}

int32_t LuaInterface::luaGetConfigFile(lua_State* L)
{
	//getConfigFile()
	lua_pushstring(L, g_config.getString(ConfigManager::CONFIG_FILE).c_str());
	return 1;
}

int32_t LuaInterface::luaDoPlayerSetWalkthrough(lua_State* L)
{
	//doPlayerSetWalkthrough(cid, uid, walkthrough)
	bool walkthrough = popBoolean(L);
	uint32_t uid = popNumber(L);

	ScriptEnviroment* env = getEnv();
	Player* player = env->getPlayerByUID(popNumber(L));
	if(!player)
	{
		errorEx(getError(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	Creature* creature = env->getCreatureByUID(uid);
	if(!creature)
	{
		errorEx(getError(LUA_ERROR_CREATURE_NOT_FOUND));
		lua_pushboolean(L, false);
		return 1;
	}

	if(player != creature)
	{
		player->setWalkthrough(creature, walkthrough);
		lua_pushboolean(L, true);
	}
	else
		lua_pushboolean(L, false);

	return 1;
}

int32_t LuaInterface::luaDoGuildAddEnemy(lua_State* L)
{
	//doGuildAddEnemy(guild, enemy, war, type)
	War_t war;
	war.type = (WarType_t)popNumber(L);
	war.war = popNumber(L);

	uint32_t enemy = popNumber(L), guild = popNumber(L), count = 0;
	for(AutoList<Player>::iterator it = Player::autoList.begin(); it != Player::autoList.end(); ++it)
	{
		if(it->second->isRemoved() || it->second->getGuildId() != guild)
			continue;

		++count;
		it->second->addEnemy(enemy, war);
		g_game.updateCreatureEmblem(it->second);
	}

	lua_pushnumber(L, count);
	return 1;
}

int32_t LuaInterface::luaDoGuildRemoveEnemy(lua_State* L)
{
	//doGuildRemoveEnemy(guild, enemy)
	uint32_t enemy = popNumber(L), guild = popNumber(L), count = 0;
	for(AutoList<Player>::iterator it = Player::autoList.begin(); it != Player::autoList.end(); ++it)
	{
		if(it->second->isRemoved() || it->second->getGuildId() != guild)
			continue;

		++count;
		it->second->removeEnemy(enemy);
		g_game.updateCreatureEmblem(it->second);
	}

	lua_pushnumber(L, count);
	return 1;
}

int32_t LuaInterface::luaGetConfigValue(lua_State* L)
{
	//getConfigValue(key)
	g_config.getValue(popString(L), L);
	return 1;
}

int32_t LuaInterface::luaGetModList(lua_State* L)
{
	//getModList()
	ModMap::iterator it = ScriptManager::getInstance()->getFirstMod();
	lua_newtable(L);
	for(uint32_t i = 1; it != ScriptManager::getInstance()->getLastMod(); ++it, ++i)
	{
		createTable(L, i);
		setField(L, "name", it->first);
		setField(L, "description", it->second.description);
		setField(L, "file", it->second.file);

		setField(L, "version", it->second.version);
		setField(L, "author", it->second.author);
		setField(L, "contact", it->second.contact);

		setFieldBool(L, "enabled", it->second.enabled);
		pushTable(L);
	}

	return 1;
}

int32_t LuaInterface::luaL_loadmodlib(lua_State* L)
{
	//loadmodlib(lib)
	std::string name = asLowerCaseString(popString(L));
	for(LibMap::iterator it = ScriptManager::getInstance()->getFirstLib();
		it != ScriptManager::getInstance()->getLastLib(); ++it)
	{
		if(asLowerCaseString(it->first) != name)
			continue;

		luaL_loadstring(L, it->second.second.c_str());
		lua_pushvalue(L, -1);
		break;
	}

	return 1;
}

int32_t LuaInterface::luaL_domodlib(lua_State* L)
{
	//domodlib(lib)
	std::string name = asLowerCaseString(popString(L));
	for(LibMap::iterator it = ScriptManager::getInstance()->getFirstLib();
		it != ScriptManager::getInstance()->getLastLib(); ++it)
	{
		if(asLowerCaseString(it->first) != name)
			continue;

		bool ret = luaL_dostring(L, it->second.second.c_str());
		if(ret)
			error(NULL, popString(L));

		lua_pushboolean(L, !ret);
		break;
	}

	return 1;
}

int32_t LuaInterface::luaL_dodirectory(lua_State* L)
{
	//dodirectory(dir[, recursively = false[, loadSystems = true]])
	bool recursively = false, loadSystems = true;
	int32_t params = lua_gettop(L);
	if(params > 2)
		loadSystems = popBoolean(L);

	if(params > 1)
		recursively = popBoolean(L);

	std::string dir = popString(L);
	if(!getEnv()->getInterface()->loadDirectory(dir, recursively, loadSystems, NULL))
	{
		errorEx("Failed to load directory " + dir);
		lua_pushboolean(L, false);
	}
	else
		lua_pushboolean(L, true);

	return 1;
}

int32_t LuaInterface::luaL_errors(lua_State* L)
{
	//errors(var)
	bool status = getEnv()->getInterface()->m_errors;
	getEnv()->getInterface()->m_errors = popBoolean(L);
	lua_pushboolean(L, status);
	return 1;
}

#define EXPOSE_LOG(Name, Stream)\
	int32_t LuaInterface::luaStd##Name(lua_State* L)\
	{\
		StringVec data;\
		for(int32_t i = 0, params = lua_gettop(L); i < params; ++i)\
		{\
			if(lua_isnil(L, -1))\
			{\
				data.push_back("nil");\
				lua_pop(L, 1);\
			}\
			else if(lua_isboolean(L, -1))\
				data.push_back(popBoolean(L) ? "true" : "false");\
			else if(lua_istable(L, -1)) {/* "table: address" */}\
			else if(lua_isfunction(L, -1)) {/* "function: address" */}\
			else\
				data.push_back(popString(L));\
		}\
\
		for(StringVec::reverse_iterator it = data.rbegin(); it != data.rend(); ++it)\
			Stream << (*it) << std::endl;\
\
		lua_pushnumber(L, data.size());\
		return 1;\
	}

EXPOSE_LOG(Cout, std::cout)
EXPOSE_LOG(Clog, std::clog)
EXPOSE_LOG(Cerr, std::cerr)

#undef EXPOSE_LOG

int32_t LuaInterface::luaStdMD5(lua_State* L)
{
	//std.md5(string[, upperCase = false])
	bool upperCase = false;
	if(lua_gettop(L) > 1)
		upperCase = popBoolean(L);

	lua_pushstring(L, transformToMD5(popString(L), upperCase).c_str());
	return 1;
}

int32_t LuaInterface::luaStdSHA1(lua_State* L)
{
	//std.sha1(string[, upperCase = false])
	bool upperCase = false;
	if(lua_gettop(L) > 1)
		upperCase = popBoolean(L);

	lua_pushstring(L, transformToSHA1(popString(L), upperCase).c_str());
	return 1;
}

int32_t LuaInterface::luaStdSHA256(lua_State* L)
{
	//std.sha256(string[, upperCase = false])
	bool upperCase = false;
	if(lua_gettop(L) > 1)
		upperCase = popBoolean(L);

	lua_pushstring(L, transformToSHA256(popString(L), upperCase).c_str());
	return 1;
}

int32_t LuaInterface::luaStdSHA512(lua_State* L)
{
	//std.sha512(string[, upperCase = false])
	bool upperCase = false;
	if(lua_gettop(L) > 1)
		upperCase = popBoolean(L);

	lua_pushstring(L, transformToSHA512(popString(L), upperCase).c_str());
	return 1;
}

int32_t LuaInterface::luaStdCheckName(lua_State* L)
{
	//std.checkName(string[, forceUppercaseOnFirstLetter = true])
	bool forceUppercaseOnFirstLetter = true;
	if(lua_gettop(L) > 1)
		forceUppercaseOnFirstLetter = popBoolean(L);

	lua_pushboolean(L, isValidName(popString(L), forceUppercaseOnFirstLetter));
	return 1;
}

int32_t LuaInterface::luaSystemTime(lua_State* L)
{
	//os.mtime()
	lua_pushnumber(L, OTSYS_TIME());
	return 1;
}

int32_t LuaInterface::luaDatabaseExecute(lua_State* L)
{
	//db.query(query)
	DBQuery query; //lock mutex
	query << popString(L);

	lua_pushboolean(L, Database::getInstance()->query(query.str()));
	return 1;
}

int32_t LuaInterface::luaDatabaseStoreQuery(lua_State* L)
{
	//db.storeQuery(query)
	ScriptEnviroment* env = getEnv();
	DBQuery query; //lock mutex

	query << popString(L);
	if(DBResult* res = Database::getInstance()->storeQuery(query.str()))
		lua_pushnumber(L, env->addResult(res));
	else
		lua_pushboolean(L, false);

	return 1;
}

int32_t LuaInterface::luaDatabaseEscapeString(lua_State* L)
{
	//db.escapeString(str)
	lua_pushstring(L, Database::getInstance()->escapeString(popString(L)).c_str());
	return 1;
}

int32_t LuaInterface::luaDatabaseEscapeBlob(lua_State* L)
{
	//db.escapeBlob(s, length)
	uint32_t length = popNumber(L);
	lua_pushstring(L, Database::getInstance()->escapeBlob(popString(L).c_str(), length).c_str());
	return 1;
}

int32_t LuaInterface::luaDatabaseLastInsertId(lua_State* L)
{
	//db.lastInsertId()
	lua_pushnumber(L, Database::getInstance()->getLastInsertId());
	return 1;
}

int32_t LuaInterface::luaDatabaseStringComparer(lua_State* L)
{
	//db.stringComparer()
	lua_pushstring(L, Database::getInstance()->getStringComparer().c_str());
	return 1;
}

int32_t LuaInterface::luaDatabaseUpdateLimiter(lua_State* L)
{
	//db.updateLimiter()
	lua_pushstring(L, Database::getInstance()->getUpdateLimiter().c_str());
	return 1;
}

int32_t LuaInterface::luaDatabaseConnected(lua_State* L)
{
	//db.connected()
	lua_pushboolean(L, Database::getInstance()->isConnected());
	return 1;
}

int32_t LuaInterface::luaDatabaseTableExists(lua_State* L)
{
	//db.tableExists(table)
	lua_pushboolean(L, DatabaseManager::getInstance()->tableExists(popString(L)));
	return 1;
}

int32_t LuaInterface::luaDatabaseTransBegin(lua_State* L)
{
	//db.transBegin()
	lua_pushboolean(L, Database::getInstance()->beginTransaction());
	return 1;
}

int32_t LuaInterface::luaDatabaseTransRollback(lua_State* L)
{
	//db.transRollback()
	lua_pushboolean(L, Database::getInstance()->rollback());
	return 1;
}

int32_t LuaInterface::luaDatabaseTransCommit(lua_State* L)
{
	//db.transCommit()
	lua_pushboolean(L, Database::getInstance()->commit());
	return 1;
}

#define CHECK_RESULT()\
	if(!res)\
	{\
		lua_pushboolean(L, false);\
		return 1;\
	}

int32_t LuaInterface::luaResultGetDataInt(lua_State* L)
{
	//result.getDataInt(res, s)
	const std::string& s = popString(L);
	ScriptEnviroment* env = getEnv();

	DBResult* res = env->getResultByID(popNumber(L));
	CHECK_RESULT()

	lua_pushnumber(L, res->getDataInt(s));
	return 1;
}

int32_t LuaInterface::luaResultGetDataLong(lua_State* L)
{
	//result.getDataLong(res, s)
	const std::string& s = popString(L);
	ScriptEnviroment* env = getEnv();

	DBResult* res = env->getResultByID(popNumber(L));
	CHECK_RESULT()

	lua_pushnumber(L, res->getDataLong(s));
	return 1;
}

int32_t LuaInterface::luaResultGetDataString(lua_State* L)
{
	//result.getDataString(res, s)
	const std::string& s = popString(L);
	ScriptEnviroment* env = getEnv();

	DBResult* res = env->getResultByID(popNumber(L));
	CHECK_RESULT()

	lua_pushstring(L, res->getDataString(s).c_str());
	return 1;
}

int32_t LuaInterface::luaResultGetDataStream(lua_State* L)
{
	//result.getDataStream(res, s)
	const std::string s = popString(L);
	ScriptEnviroment* env = getEnv();

	DBResult* res = env->getResultByID(popNumber(L));
	CHECK_RESULT()

	uint64_t length = 0;
	lua_pushstring(L, res->getDataStream(s, length));

	lua_pushnumber(L, length);
	return 2;
}

int32_t LuaInterface::luaResultNext(lua_State* L)
{
	//result.next(res)
	ScriptEnviroment* env = getEnv();

	DBResult* res = env->getResultByID(popNumber(L));
	CHECK_RESULT()

	lua_pushboolean(L, res->next());
	return 1;
}

int32_t LuaInterface::luaResultFree(lua_State* L)
{
	//result.free(res)
	uint32_t rid = popNumber(L);
	ScriptEnviroment* env = getEnv();

	DBResult* res = env->getResultByID(rid);
	CHECK_RESULT()

	lua_pushboolean(L, env->removeResult(rid));
	return 1;
}

#undef CHECK_RESULT

int32_t LuaInterface::luaBitNot(lua_State* L)
{
	int32_t number = (int32_t)popNumber(L);
	lua_pushnumber(L, ~number);
	return 1;
}

int32_t LuaInterface::luaBitUNot(lua_State* L)
{
	uint32_t number = (uint32_t)popNumber(L);
	lua_pushnumber(L, ~number);
	return 1;
}

#define MULTI_OPERATOR(type, name, op)\
	int32_t LuaInterface::luaBit##name(lua_State* L)\
	{\
		int32_t params = lua_gettop(L);\
		type value = (type)popNumber(L);\
		for(int32_t i = 2; i <= params; ++i)\
			value op popNumber(L);\
\
		lua_pushnumber(L, value);\
		return 1;\
	}

MULTI_OPERATOR(int32_t, And, &=)
MULTI_OPERATOR(int32_t, Or, |=)
MULTI_OPERATOR(int32_t, Xor, ^=)
MULTI_OPERATOR(uint32_t, UAnd, &=)
MULTI_OPERATOR(uint32_t, UOr, |=)
MULTI_OPERATOR(uint32_t, UXor, ^=)

#undef MULTI_OPERATOR

#define SHIFT_OPERATOR(type, name, op)\
	int32_t LuaInterface::luaBit##name(lua_State* L)\
	{\
		type v2 = (type)popNumber(L), v1 = (type)popNumber(L);\
		lua_pushnumber(L, (v1 op v2));\
		return 1;\
	}

SHIFT_OPERATOR(int32_t, LeftShift, <<)
SHIFT_OPERATOR(int32_t, RightShift, >>)
SHIFT_OPERATOR(uint32_t, ULeftShift, <<)
SHIFT_OPERATOR(uint32_t, URightShift, >>)

#undef SHIFT_OPERATOR
