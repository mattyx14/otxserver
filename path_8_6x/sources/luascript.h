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

#ifndef __LUASCRIPT__
#define __LUASCRIPT__
#include "otsystem.h"

#if defined(__ALT_LUA_PATH__)
extern "C"
{
	#include <lua5.1/lua.h>
	#include <lua5.1/lauxlib.h>
	#include <lua5.1/lualib.h>
}
#else
extern "C"
{
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>

	#ifdef __LUAJIT__
	#include <luajit.h>
	#endif
}
#endif

#include "database.h"
#include "position.h"

enum LuaVariantType_t
{
	VARIANT_NONE = 0,
	VARIANT_NUMBER,
	VARIANT_POSITION,
	VARIANT_TARGETPOSITION,
	VARIANT_STRING
};

struct LuaVariant
{
	LuaVariant()
	{
		type = VARIANT_NONE;
		text = "";
		pos = PositionEx();
		number = 0;
	}

	LuaVariantType_t type;
	std::string text;
	PositionEx pos;
	uint32_t number;
};

typedef std::map<std::string, std::string> StorageMap;

class Game;
class Thing;
class LuaInterface;

class Creature;
class Player;
class Npc;

class Item;
class Container;

class Combat;
class CombatArea;
class Condition;

struct Outfit_t;
class ScriptEnviroment
{
	public:
		ScriptEnviroment();
		virtual ~ScriptEnviroment();

		static bool saveGameState();
		static bool loadGameState();

		bool getStorage(const std::string& key, std::string& value) const;
		void setStorage(const std::string& key, const std::string& value) {m_storageMap[key] = value;}
		void eraseStorage(const std::string& key) {m_storageMap.erase(key);}

		inline StorageMap::const_iterator getStorageBegin() const {return m_storageMap.begin();}
		inline StorageMap::const_iterator getStorageEnd() const {return m_storageMap.end();}

		int32_t getScriptId() const {return m_scriptId;};
		void setScriptId(int32_t scriptId, LuaInterface* interface)
			{m_scriptId = scriptId; m_interface = interface;}

		int32_t getCallbackId() const {return m_callbackId;};
		bool setCallbackId(int32_t callbackId, LuaInterface* interface);

		std::string getEvent() const {return m_event;};
		void setEvent(const std::string& desc) {m_event = desc;}

		Position getRealPos() const {return m_realPos;};
		void setRealPos(const Position& realPos) {m_realPos = realPos;}

		Npc* getNpc() const {return m_curNpc;}
		void setNpc(Npc* npc) {m_curNpc = npc;}

		static uint32_t addCombatArea(CombatArea* area);
		static CombatArea* getCombatArea(uint32_t areaId);

		static uint32_t addCombatObject(Combat* combat);
		static Combat* getCombatObject(uint32_t combatId);

		static uint32_t addConditionObject(Condition* condition);
		static uint32_t addTempConditionObject(Condition* condition);
		static Condition* getConditionObject(uint32_t conditionId, bool loaded);

		Thing* getThingByUID(uint32_t uid);
		Item* getItemByUID(uint32_t uid);
		Container* getContainerByUID(uint32_t uid);
		Creature* getCreatureByUID(uint32_t uid);
		Player* getPlayerByUID(uint32_t uid);

		uint32_t addThing(Thing* thing);
		void insertThing(uint32_t uid, Thing* thing);
		void removeThing(uint32_t uid);

		static void addTempItem(ScriptEnviroment* env, Item* item);
		static void removeTempItem(ScriptEnviroment* env, Item* item);
		static void removeTempItem(Item* item);

		DBResult* getResultByID(uint32_t id);
		uint32_t addResult(DBResult* res);
		bool removeResult(uint32_t id);

		static void addUniqueThing(Thing* thing);
		static void removeUniqueThing(Thing* thing);

		static uint32_t getLastConditionId() {return m_lastConditionId;}
		static uint32_t getLastCombatId() {return m_lastCombatId;}
		static uint32_t getLastAreaId() {return m_lastAreaId;}

		void setTimerEvent() {m_timerEvent = true;}
		void resetTimerEvent() {m_timerEvent = false;}

		LuaInterface* getInterface() {return m_interface;}
		void getInfo(int32_t& scriptId, std::string& desc, LuaInterface*& interface, int32_t& callbackId, bool& timerEvent);
		void reset();
		void resetCallback() {m_callbackId = 0;}

		void streamVariant(std::stringstream& stream, const std::string& local, const LuaVariant& var);
		void streamThing(std::stringstream& stream, const std::string& local, Thing* thing, uint32_t id = 0);
		void streamPosition(std::stringstream& stream, const std::string& local, const PositionEx& position)
			{streamPosition(stream, local, position, position.stackpos);}
		void streamPosition(std::stringstream& stream, const std::string& local, const Position& position, uint32_t stackpos);
		void streamOutfit(std::stringstream& stream, const std::string& local, const Outfit_t& outfit);

	private:
		typedef std::map<uint64_t, Thing*> ThingMap;
		typedef std::vector<const LuaVariant*> VariantVector;
		typedef std::list<Item*> ItemList;
		typedef std::map<ScriptEnviroment*, ItemList> TempItemListMap;

		typedef std::map<uint32_t, CombatArea*> AreaMap;
		typedef std::map<uint32_t, Combat*> CombatMap;
		typedef std::map<uint32_t, Condition*> ConditionMap;
		typedef std::map<uint32_t, DBResult*> DBResultMap;

		LuaInterface* m_interface;
		int32_t m_scriptId, m_callbackId;
		std::string m_event;
		bool m_timerEvent;

		ThingMap m_localMap;
		DBResultMap m_tempResults;

		static TempItemListMap m_tempItems;
		static StorageMap m_storageMap;
		static ThingMap m_globalMap;

		static uint32_t m_lastAreaId;
		static AreaMap m_areaMap;

		static uint32_t m_lastCombatId;
		static CombatMap m_combatMap;

		static uint32_t m_lastConditionId, m_lastTempConditionId;
		static ConditionMap m_conditionMap;
		static ConditionMap m_tempConditionMap;

		int32_t m_lastUID;
		bool m_loaded;
		Position m_realPos;
		Npc* m_curNpc;
};

enum ErrorCode_t
{
	LUA_ERROR_PLAYER_NOT_FOUND,
	LUA_ERROR_MONSTER_NOT_FOUND,
	LUA_ERROR_NPC_NOT_FOUND,
	LUA_ERROR_CREATURE_NOT_FOUND,
	LUA_ERROR_ITEM_NOT_FOUND,
	LUA_ERROR_THING_NOT_FOUND,
	LUA_ERROR_TILE_NOT_FOUND,
	LUA_ERROR_HOUSE_NOT_FOUND,
	LUA_ERROR_COMBAT_NOT_FOUND,
	LUA_ERROR_CONDITION_NOT_FOUND,
	LUA_ERROR_AREA_NOT_FOUND,
	LUA_ERROR_CONTAINER_NOT_FOUND,
	LUA_ERROR_VARIANT_NOT_FOUND,
	LUA_ERROR_VARIANT_UNKNOWN,
	LUA_ERROR_SPELL_NOT_FOUND
};

enum Recursive_t
{
	RECURSE_FIRST = -1,
	RECURSE_NONE = 0,
	RECURSE_ALL = 1
};

#define errorEx(a) error(__FUNCTION__, a)
class LuaInterface
{
	public:
		LuaInterface(std::string interfaceName);
		virtual ~LuaInterface();

		virtual bool initState();
		bool reInitState();

		static bool reserveEnv()
		{
			if(++m_scriptEnvIndex > 20)
			{
				--m_scriptEnvIndex;
				return false;
			}

			return true;
		}
		static void releaseEnv()
		{
			if(m_scriptEnvIndex >= 0)
			{
				m_scriptEnv[m_scriptEnvIndex].reset();
				--m_scriptEnvIndex;
			}
		}

		bool loadBuffer(const std::string& text, Npc* npc = NULL);
		bool loadFile(const std::string& file, Npc* npc = NULL);
		bool loadDirectory(std::string dir, bool recursively, bool loadSystems, Npc* npc = NULL);

		std::string getName() const {return m_interfaceName;};
		std::string getScript(int32_t scriptId);
		std::string getLastError() const {return m_lastError;}

		int32_t getEvent(const std::string& eventName);
		lua_State* getState() {return m_luaState;}
		static ScriptEnviroment* getEnv()
		{
			assert(m_scriptEnvIndex >= 0 && m_scriptEnvIndex < 21);
			return &m_scriptEnv[m_scriptEnvIndex];
		}

		bool pushFunction(int32_t functionId);
		bool callFunction(uint32_t params);
		static int32_t handleFunction(lua_State* L);

		void dumpStack(lua_State* L = NULL);

		//push/pop common structures
		static void pushThing(lua_State* L, Thing* thing, uint32_t id = 0, Recursive_t recursive = RECURSE_FIRST);
		static void pushVariant(lua_State* L, const LuaVariant& var);
		static void pushPosition(lua_State* L, const PositionEx& position) {pushPosition(L, position, position.stackpos);}
		static void pushPosition(lua_State* L, const Position& position, uint32_t stackpos);
		static void pushOutfit(lua_State* L, const Outfit_t& outfit);
		static void pushCallback(lua_State* L, int32_t callback);

		static LuaVariant popVariant(lua_State* L);
		static void popPosition(lua_State* L, PositionEx& position);
		static void popPosition(lua_State* L, Position& position, uint32_t& stackpos);
		static bool popBoolean(lua_State* L);
		static int64_t popNumber(lua_State* L);
		static double popFloatNumber(lua_State* L);
		static std::string popString(lua_State* L);
		static int32_t popCallback(lua_State* L);
		static Outfit_t popOutfit(lua_State* L);

		static int64_t getField(lua_State* L, const char* key);
		static uint64_t getFieldUnsigned(lua_State* L, const char* key);
		static std::string getFieldString(lua_State* L, const char* key);
		static bool getFieldBool(lua_State* L, const char* key);

		static void setField(lua_State* L, const char* index, int32_t val);
		static void setField(lua_State* L, const char* index, const std::string& val);
		static void setFieldBool(lua_State* L, const char* index, bool val);
		static void setFieldFloat(lua_State* L, const char* index, double val);

		static void createTable(lua_State* L, const char* index);
		static void createTable(lua_State* L, const char* index, int32_t narr, int32_t nrec);
		static void createTable(lua_State* L, int32_t index);
		static void createTable(lua_State* L, int32_t index, int32_t narr, int32_t nrec);
		static void pushTable(lua_State* L);

		static std::string getGlobalString(lua_State* L, const std::string& _identifier, const std::string& _default = "");
		static bool getGlobalBool(lua_State* L, const std::string& _identifier, bool _default = false);
		static int64_t getGlobalNumber(lua_State* L, const std::string& _identifier, const int64_t _default = 0);
		static double getGlobalDouble(lua_State* L, const std::string& _identifier, const double _default = 0);

		static void getValue(const std::string& key, lua_State* L, lua_State* _L);
		static void moveValue(lua_State* from, lua_State* to);

		static void error(const char* function, const std::string& desc);

	protected:
		virtual bool closeState();

		static std::string getError(ErrorCode_t code);
		static bool getArea(lua_State* L, std::list<uint32_t>& list, uint32_t& rows);

		virtual void registerFunctions();

		//lua functions
		static int32_t luaDoRemoveItem(lua_State* L);
		static int32_t luaDoPlayerFeed(lua_State* L);
		static int32_t luaDoPlayerSendCancel(lua_State* L);
		static int32_t luaDoSendDefaultCancel(lua_State* L);
		static int32_t luaGetSearchString(lua_State* L);
		static int32_t luaGetClosestFreeTile(lua_State* L);
		static int32_t luaDoTeleportThing(lua_State* L);
		static int32_t luaDoItemSetDestination(lua_State* L);
		static int32_t luaDoTransformItem(lua_State* L);
		static int32_t luaDoSendCreatureSquare(lua_State* L);
		static int32_t luaDoSendAnimatedText(lua_State* L);
		static int32_t luaDoSendMagicEffect(lua_State* L);
		static int32_t luaDoSendDistanceShoot(lua_State* L);
		static int32_t luaDoShowTextWindow(lua_State* L);
		static int32_t luaDoShowTextDialog(lua_State* L);
		static int32_t luaDoDecayItem(lua_State* L);
		static int32_t luaDoCreateItem(lua_State* L);
		static int32_t luaDoCreateItemEx(lua_State* L);
		static int32_t luaDoCreateTeleport(lua_State* L);
		static int32_t luaDoCreateMonster(lua_State* L);
		static int32_t luaDoCreateNpc(lua_State* L);
		static int32_t luaDoSummonMonster(lua_State* L);
		static int32_t luaDoConvinceCreature(lua_State* L);
		static int32_t luaGetMonsterTargetList(lua_State* L);
		static int32_t luaGetMonsterFriendList(lua_State* L);
		static int32_t luaDoMonsterSetTarget(lua_State* L);
		static int32_t luaDoMonsterChangeTarget(lua_State* L);
		static int32_t luaDoAddCondition(lua_State* L);
		static int32_t luaDoRemoveCondition(lua_State* L);
		static int32_t luaDoRemoveConditions(lua_State* L);
		static int32_t luaDoRemoveCreature(lua_State* L);
		static int32_t luaDoMoveCreature(lua_State* L);
		static int32_t luaDoSteerCreature(lua_State* L);
		static int32_t luaDoCreatureSay(lua_State* L);
		static int32_t luaDoPlayerAddSkillTry(lua_State* L);
		static int32_t luaDoCreatureAddHealth(lua_State* L);
		static int32_t luaDoCreatureAddMana(lua_State* L);
		static int32_t luaSetCreatureMaxHealth(lua_State* L);
		static int32_t luaSetCreatureMaxMana(lua_State* L);
		static int32_t luaDoPlayerSetMaxCapacity(lua_State* L);
		static int32_t luaDoPlayerAddSpentMana(lua_State* L);
		static int32_t luaDoPlayerAddItem(lua_State* L);
		static int32_t luaDoPlayerAddItemEx(lua_State* L);
		static int32_t luaDoTileAddItemEx(lua_State* L);
		static int32_t luaDoAddContainerItemEx(lua_State* L);
		static int32_t luaDoRelocate(lua_State* L);
		static int32_t luaDoCleanTile(lua_State* L);
		static int32_t luaDoPlayerSendTextMessage(lua_State* L);
		static int32_t luaDoPlayerSendChannelMessage(lua_State* L);
		static int32_t luaDoCreatureChannelSay(lua_State* L);
		static int32_t luaDoPlayerOpenChannel(lua_State* L);
		static int32_t luaDoPlayerSendChannels(lua_State* L);
		static int32_t luaDoPlayerAddMoney(lua_State* L);
		static int32_t luaDoPlayerRemoveMoney(lua_State* L);
		static int32_t luaDoPlayerTransferMoneyTo(lua_State* L);
		static int32_t luaDoPlayerSetPzLocked(lua_State* L);
		static int32_t luaDoPlayerSetTown(lua_State* L);
		static int32_t luaDoPlayerSetVocation(lua_State* L);
		static int32_t luaDoPlayerRemoveItem(lua_State* L);
		static int32_t luaDoPlayerAddSoul(lua_State* L);
		static int32_t luaDoPlayerSetStamina(lua_State* L);
		static int32_t luaDoPlayerAddExperience(lua_State* L);
		static int32_t luaDoPlayerSetGuildId(lua_State* L);
		static int32_t luaDoPlayerSetGuildLevel(lua_State* L);
		static int32_t luaDoPlayerSetGuildNick(lua_State* L);
		static int32_t luaDoPlayerSetSex(lua_State* L);
		static int32_t luaDoPlayerSetIdleTime(lua_State* L);
		static int32_t luaGetPlayerIdleTime(lua_State* L);
		static int32_t luaDoCreatureSetLookDir(lua_State* L);
		static int32_t luaGetCreatureHideHealth(lua_State* L);
		static int32_t luaDoCreatureSetHideHealth(lua_State* L);
		static int32_t luaGetCreatureSpeakType(lua_State* L);
		static int32_t luaDoCreatureSetSpeakType(lua_State* L);
		static int32_t luaGetCreatureGuildEmblem(lua_State* L);
		static int32_t luaDoCreatureSetGuildEmblem(lua_State* L);
		static int32_t luaGetCreaturePartyShield(lua_State* L);
		static int32_t luaDoCreatureSetPartyShield(lua_State* L);
		static int32_t luaGetCreatureSkullType(lua_State* L);
		static int32_t luaDoCreatureSetSkullType(lua_State* L);
		static int32_t luaGetPlayerSkullEnd(lua_State* L);
		static int32_t luaDoPlayerSetSkullEnd(lua_State* L);
		static int32_t luaDoPlayerSwitchSaving(lua_State* L);
		static int32_t luaDoPlayerSave(lua_State* L);
		static int32_t luaDoPlayerSendOutfitWindow(lua_State* L);
		static int32_t luaDoCreatureExecuteTalkAction(lua_State* L);
		static int32_t luaGetCreatureByName(lua_State* L);
		static int32_t luaGetPlayerByGUID(lua_State* L);
		static int32_t luaGetPlayerByNameWildcard(lua_State* L);
		static int32_t luaGetPlayerGUIDByName(lua_State* L);
		static int32_t luaGetPlayerNameByGUID(lua_State* L);
		static int32_t luaDoPlayerChangeName(lua_State* L);
		static int32_t luaGetPlayersByAccountId(lua_State* L);
		static int32_t luaGetAccountIdByName(lua_State* L);
		static int32_t luaGetAccountByName(lua_State* L);
		static int32_t luaGetAccountIdByAccount(lua_State* L);
		static int32_t luaGetAccountByAccountId(lua_State* L);
		static int32_t luaGetAccountFlagValue(lua_State* L);
		static int32_t luaGetAccountCustomFlagValue(lua_State* L);
		static int32_t luaGetIpByName(lua_State* L);
		static int32_t luaGetPlayersByIp(lua_State* L);
		static int32_t luaIsIpBanished(lua_State* L);
		static int32_t luaIsPlayerBanished(lua_State* L);
		static int32_t luaIsAccountBanished(lua_State* L);
		static int32_t luaDoAddIpBanishment(lua_State* L);
		static int32_t luaDoAddPlayerBanishment(lua_State* L);
		static int32_t luaDoAddAccountBanishment(lua_State* L);
		static int32_t luaDoAddNotation(lua_State* L);
		static int32_t luaDoAddStatement(lua_State* L);
		static int32_t luaDoRemoveIpBanishment(lua_State* L);
		static int32_t luaDoRemovePlayerBanishment(lua_State* L);
		static int32_t luaDoRemoveAccountBanishment(lua_State* L);
		static int32_t luaDoAddAccountWarnings(lua_State* L);
		static int32_t luaGetAccountWarnings(lua_State* L);
		static int32_t luaDoRemoveNotations(lua_State* L);
		static int32_t luaDoRemoveStatements(lua_State* L);
		static int32_t luaGetNotationsCount(lua_State* L);
		static int32_t luaGetStatementsCount(lua_State* L);
		static int32_t luaGetBanData(lua_State* L);
		static int32_t luaGetBanReason(lua_State* L);
		static int32_t luaGetBanAction(lua_State* L);
		static int32_t luaGetBanList(lua_State* L);
		static int32_t luaGetPlayerModes(lua_State* L);
		static int32_t luaGetPlayerRates(lua_State* L);
		static int32_t luaDoPlayerSetRate(lua_State* L);
		static int32_t luaDoCreatureSetDropLoot(lua_State* L);
		static int32_t luaGetPlayerLossPercent(lua_State* L);
		static int32_t luaDoPlayerSetLossPercent(lua_State* L);
		static int32_t luaDoPlayerSetLossSkill(lua_State* L);
		static int32_t luaGetPlayerLossSkill(lua_State* L);
		static int32_t luaGetThing(lua_State* L);
		static int32_t luaGetThingPosition(lua_State* L);
		static int32_t luaDoItemRaidUnref(lua_State* L);
		static int32_t luaHasItemProperty(lua_State* L);
		static int32_t luaHasMonsterRaid(lua_State* L);
		static int32_t luaGetThingFromPosition(lua_State* L);
		static int32_t luaGetTileItemById(lua_State* L);
		static int32_t luaGetTileItemByType(lua_State* L);
		static int32_t luaGetTileThingByPos(lua_State* L);
		static int32_t luaGetTopCreature(lua_State* L);
		static int32_t luaGetTileInfo(lua_State* L);
		static int32_t luaDoTileQueryAdd(lua_State* L);
		static int32_t luaGetHouseInfo(lua_State* L);
		static int32_t luaGetHouseAccessList(lua_State* L);
		static int32_t luaGetHouseByPlayerGUID(lua_State* L);
		static int32_t luaGetHouseFromPosition(lua_State* L);
		static int32_t luaSetHouseOwner(lua_State* L);
		static int32_t luaSetHouseAccessList(lua_State* L);
		static int32_t luaDoPlayerSetNameDescription(lua_State* L);
		static int32_t luaGetPlayerNameDescription(lua_State* L);
		static int32_t luaDoPlayerSetSpecialDescription(lua_State* L);
		static int32_t luaGetPlayerSpecialDescription(lua_State* L);
		static int32_t luaGetPlayerFood(lua_State* L);
		static int32_t luaGetPlayerAccess(lua_State* L);
		static int32_t luaGetPlayerGhostAccess(lua_State* L);
		static int32_t luaGetPlayerLevel(lua_State* L);
		static int32_t luaGetPlayerExperience(lua_State* L);
		static int32_t luaGetPlayerMagLevel(lua_State* L);
		static int32_t luaGetPlayerSpentMana(lua_State* L);
		static int32_t luaGetCreatureMana(lua_State* L);
		static int32_t luaGetCreatureMaxMana(lua_State* L);
		static int32_t luaGetCreatureHealth(lua_State* L);
		static int32_t luaGetCreatureMaxHealth(lua_State* L);
		static int32_t luaGetCreatureSpeed(lua_State* L);
		static int32_t luaGetCreatureBaseSpeed(lua_State* L);
		static int32_t luaGetCreatureTarget(lua_State* L);
		static int32_t luaGetCreatureLookDirection(lua_State* L);
		static int32_t luaGetPlayerSkillLevel(lua_State* L);
		static int32_t luaGetPlayerSkillTries(lua_State* L);
		static int32_t luaDoPlayerSetOfflineTrainingSkill(lua_State* L);
		static int32_t luaGetPlayerVocation(lua_State* L);
		static int32_t luaGetPlayerTown(lua_State* L);
		static int32_t luaGetPlayerItemCount(lua_State* L);
		static int32_t luaGetPlayerMoney(lua_State* L);
		static int32_t luaGetPlayerSoul(lua_State* L);
		static int32_t luaGetPlayerStamina(lua_State* L);
		static int32_t luaGetPlayerFreeCap(lua_State* L);
		static int32_t luaGetPlayerLight(lua_State* L);
		static int32_t luaGetPlayerSlotItem(lua_State* L);
		static int32_t luaGetPlayerWeapon(lua_State* L);
		static int32_t luaGetPlayerItemById(lua_State* L);
		static int32_t luaGetPlayerRequiredMana(lua_State* L);
		static int32_t luaGetPlayerRequiredSkillTries(lua_State* L);
		static int32_t luaGetPlayerIp(lua_State* L);
		static int32_t luaGetPlayerLastLoad(lua_State* L);
		static int32_t luaGetPlayerLastLogin(lua_State* L);
		static int32_t luaGetPlayerTradeState(lua_State* L);
		static int32_t luaGetPlayerAccountManager(lua_State* L);
		static int32_t luaGetPlayerAccountId(lua_State* L);
		static int32_t luaGetPlayerAccount(lua_State* L);
		static int32_t luaGetPlayerDepotItems(lua_State* L);
		static int32_t luaGetPlayerGuildId(lua_State* L);
		static int32_t luaGetPlayerGuildName(lua_State* L);
		static int32_t luaGetPlayerGuildRank(lua_State* L);
		static int32_t luaGetPlayerGuildRankId(lua_State* L);
		static int32_t luaGetPlayerGuildLevel(lua_State* L);
		static int32_t luaGetPlayerGuildNick(lua_State* L);
		static int32_t luaGetPlayerSex(lua_State* L);
		static int32_t luaGetPlayerGUID(lua_State* L);
		static int32_t luaGetPlayerOperatingSystem(lua_State* L);
		static int32_t luaGetPlayerClientVersion(lua_State* L);
		static int32_t luaGetPlayerFlagValue(lua_State* L);
		static int32_t luaGetPlayerCustomFlagValue(lua_State* L);
		static int32_t luaHasCreatureCondition(lua_State* L);
		static int32_t luaGetCreatureConditionInfo(lua_State* L);
		static int32_t luaHasPlayerClient(lua_State* L);
		static int32_t luaGetDepotId(lua_State* L);
		static int32_t luaGetVocationInfo(lua_State* L);
		static int32_t luaGetGroupInfo(lua_State* L);
		static int32_t luaGetMonsterInfo(lua_State* L);
		static int32_t luaGetPlayerPromotionLevel(lua_State* L);
		static int32_t luaDoPlayerSetPromotionLevel(lua_State* L);
		static int32_t luaGetPlayerGroupId(lua_State* L);
		static int32_t luaDoPlayerSetGroupId(lua_State* L);
		static int32_t luaDoPlayerLearnInstantSpell(lua_State* L);
		static int32_t luaDoPlayerUnlearnInstantSpell(lua_State* L);
		static int32_t luaGetPlayerLearnedInstantSpell(lua_State* L);
		static int32_t luaGetPlayerInstantSpellCount(lua_State* L);
		static int32_t luaGetPlayerInstantSpellInfo(lua_State* L);
		static int32_t luaGetInstantSpellInfo(lua_State* L);
		static int32_t luaDoCreatureCastSpell(lua_State* L);
		static int32_t luaGetPlayerPartner(lua_State* L);
		static int32_t luaDoPlayerSetPartner(lua_State* L);
		static int32_t luaDoPlayerFollowCreature(lua_State* L);
		static int32_t luaGetPlayerParty(lua_State* L);
		static int32_t luaDoPlayerJoinParty(lua_State* L);
		static int32_t luaDoPlayerLeaveParty(lua_State* L);
		static int32_t luaGetPartyMembers(lua_State* L);
		static int32_t luaGetCreatureStorageList(lua_State* L);
		static int32_t luaGetCreatureStorage(lua_State* L);
		static int32_t luaDoCreatureSetStorage(lua_State* L);
		static int32_t luaGetPlayerSpectators(lua_State* L);
		static int32_t luaDoPlayerSetSpectators(lua_State* L);
		static int32_t luaDoPlayerAddBlessing(lua_State* L);
		static int32_t luaGetPlayerBlessing(lua_State* L);
		static int32_t luaDoPlayerSetPVPBlessing(lua_State* L);
		static int32_t luaGetPlayerPVPBlessing(lua_State* L);
		static int32_t luaDoGuildAddEnemy(lua_State* L);
		static int32_t luaDoGuildRemoveEnemy(lua_State* L);
		static int32_t luaGetStorageList(lua_State* L);
		static int32_t luaGetStorage(lua_State* L);
		static int32_t luaDoSetStorage(lua_State* L);
		static int32_t luaDoPlayerAddOutfit(lua_State* L);
		static int32_t luaDoPlayerRemoveOutfit(lua_State* L);
		static int32_t luaDoPlayerAddOutfitId(lua_State* L);
		static int32_t luaDoPlayerRemoveOutfitId(lua_State* L);
		static int32_t luaCanPlayerWearOutfit(lua_State* L);
		static int32_t luaCanPlayerWearOutfitId(lua_State* L);
		static int32_t luaGetWorldType(lua_State* L);
		static int32_t luaSetWorldType(lua_State* L);
		static int32_t luaGetWorldTime(lua_State* L);
		static int32_t luaGetWorldLight(lua_State* L);
		static int32_t luaGetWorldCreatures(lua_State* L);
		static int32_t luaGetWorldUpTime(lua_State* L);
		static int32_t luaGetGuildId(lua_State* L);
		static int32_t luaGetGuildMotd(lua_State* L);
		static int32_t luaIsPlayerPzLocked(lua_State* L);
		static int32_t luaIsPlayerSaving(lua_State* L);
		static int32_t luaIsPlayerProtected(lua_State* L);
		static int32_t luaIsCreature(lua_State* L);
		static int32_t luaIsMovable(lua_State* L);
		static int32_t luaGetContainerSize(lua_State* L);
		static int32_t luaGetContainerCap(lua_State* L);
		static int32_t luaGetContainerItem(lua_State* L);
		static int32_t luaDoAddContainerItem(lua_State* L);
		static int32_t luaCreateCombatObject(lua_State* L);
		static int32_t luaCreateCombatArea(lua_State* L);
		static int32_t luaSetCombatArea(lua_State* L);
		static int32_t luaSetCombatCondition(lua_State* L);
		static int32_t luaSetCombatParam(lua_State* L);
		static int32_t luaCreateConditionObject(lua_State* L);
		static int32_t luaSetConditionParam(lua_State* L);
		static int32_t luaAddDamageCondition(lua_State* L);
		static int32_t luaAddOutfitCondition(lua_State* L);
		static int32_t luaSetCombatCallBack(lua_State* L);
		static int32_t luaSetCombatFormula(lua_State* L);
		static int32_t luaSetConditionFormula(lua_State* L);
		static int32_t luaDoCombat(lua_State* L);
		static int32_t luaDoCombatAreaHealth(lua_State* L);
		static int32_t luaDoTargetCombatHealth(lua_State* L);
		static int32_t luaDoCombatAreaMana(lua_State* L);
		static int32_t luaDoTargetCombatMana(lua_State* L);
		static int32_t luaDoCombatAreaCondition(lua_State* L);
		static int32_t luaDoTargetCombatCondition(lua_State* L);
		static int32_t luaDoCombatAreaDispel(lua_State* L);
		static int32_t luaDoTargetCombatDispel(lua_State* L);
		static int32_t luaDoChallengeCreature(lua_State* L);
		static int32_t luaNumberToVariant(lua_State* L);
		static int32_t luaStringToVariant(lua_State* L);
		static int32_t luaPositionToVariant(lua_State* L);
		static int32_t luaTargetPositionToVariant(lua_State* L);
		static int32_t luaVariantToNumber(lua_State* L);
		static int32_t luaVariantToString(lua_State* L);
		static int32_t luaVariantToPosition(lua_State* L);
		static int32_t luaDoChangeSpeed(lua_State* L);
		static int32_t luaGetExperienceStage(lua_State* L);
		static int32_t luaDoCreatureChangeOutfit(lua_State* L);
		static int32_t luaSetCreatureOutfit(lua_State* L);
		static int32_t luaGetCreatureOutfit(lua_State* L);
		static int32_t luaSetMonsterOutfit(lua_State* L);
		static int32_t luaSetItemOutfit(lua_State* L);
		static int32_t luaGetCreatureLastPosition(lua_State* L);
		static int32_t luaGetCreatureName(lua_State* L);
		static int32_t luaGetCreatureMaster(lua_State* L);
		static int32_t luaGetCreatureSummons(lua_State* L);
		static int32_t luaGetHighscoreString(lua_State* L);
		static int32_t luaIsSightClear(lua_State* L);
		static int32_t luaAddEvent(lua_State* L);
		static int32_t luaStopEvent(lua_State* L);
		static int32_t luaRegisterCreatureEvent(lua_State* L);
		static int32_t luaUnregisterCreatureEvent(lua_State* L);
		static int32_t luaUnregisterCreatureEventType(lua_State* L);
		static int32_t luaGetPlayerBalance(lua_State* L);
		static int32_t luaDoPlayerSetBalance(lua_State* L);
		static int32_t luaDoPlayerPopupFYI(lua_State* L);
		static int32_t luaDoPlayerSendTutorial(lua_State* L);
		static int32_t luaDoPlayerSendMailByName(lua_State* L);
		static int32_t luaDoPlayerAddMapMark(lua_State* L);
		static int32_t luaGetPlayerPremiumDays(lua_State* L);
		static int32_t luaDoPlayerAddPremiumDays(lua_State* L);
		static int32_t luaGetCreatureNoMove(lua_State* L);
		static int32_t luaDoCreatureSetNoMove(lua_State* L);
		static int32_t luaGetTownId(lua_State* L);
		static int32_t luaGetTownName(lua_State* L);
		static int32_t luaGetTownTemplePosition(lua_State* L);
		static int32_t luaGetTownHouses(lua_State* L);
		static int32_t luaGetSpectators(lua_State* L);
		static int32_t luaGetGameState(lua_State* L);
		static int32_t luaDoSetGameState(lua_State* L);
		static int32_t luaGetChannelUsers(lua_State* L);
		static int32_t luaGetPlayersOnline(lua_State* L);
		static int32_t luaDoExecuteRaid(lua_State* L);
		static int32_t luaDoReloadInfo(lua_State* L);
		static int32_t luaDoSaveServer(lua_State* L);
		static int32_t luaDoSaveHouse(lua_State* L);
		static int32_t luaDoCleanHouse(lua_State* L);
		static int32_t luaDoCleanMap(lua_State* L);
		static int32_t luaDoRefreshMap(lua_State* L);
		static int32_t luaDoUpdateHouseAuctions(lua_State* L);
		static int32_t luaGetItemIdByName(lua_State* L);
		static int32_t luaGetItemInfo(lua_State* L);
		static int32_t luaGetItemWeight(lua_State* L);
		static int32_t luaGetItemParent(lua_State* L);
		static int32_t luaGetItemAttribute(lua_State* L);
		static int32_t luaDoItemSetAttribute(lua_State* L);
		static int32_t luaDoItemEraseAttribute(lua_State* L);
		static int32_t luaGetVocationList(lua_State* L);
		static int32_t luaGetGroupList(lua_State* L);
		static int32_t luaGetChannelList(lua_State* L);
		static int32_t luaGetTalkActionList(lua_State* L);
		static int32_t luaGetExperienceStageList(lua_State* L);
		static int32_t luaGetTownList(lua_State* L);
		static int32_t luaGetWaypointList(lua_State* L);
		static int32_t luaGetWaypointPosition(lua_State* L);
		static int32_t luaDoWaypointAddTemporial(lua_State* L);
		static int32_t luaGetDataDir(lua_State* L);
		static int32_t luaGetLogsDir(lua_State* L);
		static int32_t luaGetConfigFile(lua_State* L);
		static int32_t luaGetConfigValue(lua_State* L);
		static int32_t luaGetModList(lua_State* L);
		static int32_t luaDoPlayerSetWalkthrough(lua_State* L);
		static int32_t luaIsPlayerUsingOtclient(lua_State* L);
		static int32_t luaDoSendPlayerExtendedOpcode(lua_State* L);

		static int32_t luaL_errors(lua_State* L);
		static int32_t luaL_loadmodlib(lua_State* L);
		static int32_t luaL_domodlib(lua_State* L);
		static int32_t luaL_dodirectory(lua_State* L);

		static const luaL_Reg luaSystemTable[2];
		static int32_t luaSystemTime(lua_State* L);

		static const luaL_Reg luaDatabaseTable[13];
		static int32_t luaDatabaseExecute(lua_State* L);
		static int32_t luaDatabaseStoreQuery(lua_State* L);
		static int32_t luaDatabaseEscapeString(lua_State* L);
		static int32_t luaDatabaseEscapeBlob(lua_State* L);
		static int32_t luaDatabaseLastInsertId(lua_State* L);
		static int32_t luaDatabaseStringComparer(lua_State* L);
		static int32_t luaDatabaseUpdateLimiter(lua_State* L);
		static int32_t luaDatabaseConnected(lua_State* L);
		static int32_t luaDatabaseTableExists(lua_State* L);
		static int32_t luaDatabaseTransBegin(lua_State* L);
		static int32_t luaDatabaseTransRollback(lua_State* L);
		static int32_t luaDatabaseTransCommit(lua_State* L);

		static const luaL_Reg luaResultTable[7];
		static int32_t luaResultGetDataInt(lua_State* L);
		static int32_t luaResultGetDataLong(lua_State* L);
		static int32_t luaResultGetDataString(lua_State* L);
		static int32_t luaResultGetDataStream(lua_State* L);
		static int32_t luaResultNext(lua_State* L);
		static int32_t luaResultFree(lua_State* L);

		static const luaL_Reg luaBitTable[13];
		static int32_t luaBitNot(lua_State* L);
		static int32_t luaBitAnd(lua_State* L);
		static int32_t luaBitOr(lua_State* L);
		static int32_t luaBitXor(lua_State* L);
		static int32_t luaBitLeftShift(lua_State* L);
		static int32_t luaBitRightShift(lua_State* L);
		static int32_t luaBitUNot(lua_State* L);
		static int32_t luaBitUAnd(lua_State* L);
		static int32_t luaBitUOr(lua_State* L);
		static int32_t luaBitUXor(lua_State* L);
		static int32_t luaBitULeftShift(lua_State* L);
		static int32_t luaBitURightShift(lua_State* L);

		static const luaL_Reg luaStdTable[9];
		static int32_t luaStdCout(lua_State* L);
		static int32_t luaStdClog(lua_State* L);
		static int32_t luaStdCerr(lua_State* L);
		static int32_t luaStdMD5(lua_State* L);
		static int32_t luaStdSHA1(lua_State* L);
		static int32_t luaStdSHA256(lua_State* L);
		static int32_t luaStdSHA512(lua_State* L);
		static int32_t luaStdCheckName(lua_State* L);

		lua_State* m_luaState;
		bool m_errors;
		std::string m_lastError;

	private:
		void executeTimer(uint32_t eventIndex);

		enum PlayerInfo_t
		{
			PlayerInfoFood,
			PlayerInfoAccess,
			PlayerInfoGhostAccess,
			PlayerInfoLevel,
			PlayerInfoExperience,
			PlayerInfoManaSpent,
			PlayerInfoVocation,
			PlayerInfoTown,
			PlayerInfoPromotionLevel,
			PlayerInfoMoney,
			PlayerInfoFreeCap,
			PlayerInfoGuildId,
			PlayerInfoGuildName,
			PlayerInfoGuildRankId,
			PlayerInfoGuildRank,
			PlayerInfoGuildLevel,
			PlayerInfoGuildNick,
			PlayerInfoGroupId,
			PlayerInfoGUID,
			PlayerInfoAccountId,
			PlayerInfoAccount,
			PlayerInfoPremiumDays,
			PlayerInfoBalance,
			PlayerInfoStamina,
			PlayerInfoLossSkill,
			PlayerInfoMarriage,
			PlayerInfoPzLock,
			PlayerInfoSaving,
			PlayerInfoProtected,
			PlayerInfoIp,
			PlayerInfoSkullEnd,
			PlayerInfoOutfitWindow,
			PlayerInfoNameDescription,
			PlayerInfoSpecialDescription,
			PlayerInfoIdleTime,
			PlayerInfoClient,
			PlayerInfoLastLoad,
			PlayerInfoLastLogin,
			PlayerInfoAccountManager,
			PlayerInfoTradeState,
			PlayerInfoOperatingSystem,
			PlayerInfoClientVersion
		};
		static int32_t internalGetPlayerInfo(lua_State* L, PlayerInfo_t info);

		int32_t m_runningEvent;
		uint32_t m_lastTimer;
		std::string m_loadingFile, m_interfaceName;

		static ScriptEnviroment m_scriptEnv[21];
		static int32_t m_scriptEnvIndex;

		//events information
		struct LuaTimerEvent
		{
			int32_t scriptId, function;
			uint32_t eventId;
			Npc* npc;
			std::list<int32_t> parameters;
		};

		typedef std::map<uint32_t, LuaTimerEvent> LuaTimerEvents;
		LuaTimerEvents m_timerEvents;

		//script file cache
		typedef std::map<int32_t, std::string> ScriptsCache;
		ScriptsCache m_cacheFiles;
};
#endif
