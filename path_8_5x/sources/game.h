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

#ifndef __GAME__
#define __GAME__
#include "otsystem.h"

#include "enums.h"
#include "templates.h"
#include "server.h"
#include "scheduler.h"

#include "map.h"
#include "spawn.h"

#include "item.h"
#include "player.h"
#include "npc.h"
#include "monster.h"

class Creature;
class Player;
class Monster;
class Npc;
class CombatInfo;
struct CombatParams;

enum stackposType_t
{
	STACKPOS_NORMAL,
	STACKPOS_MOVE,
	STACKPOS_LOOK,
	STACKPOS_USE,
	STACKPOS_USEITEM
};

enum WorldType_t
{
	WORLDTYPE_FIRST = 1,
	WORLDTYPE_OPTIONAL = WORLDTYPE_FIRST,
	WORLDTYPE_OPEN = 2,
	WORLDTYPE_HARDCORE = 3,
	WORLDTYPE_LAST = WORLDTYPE_HARDCORE
};

enum GameState_t
{
	GAMESTATE_FIRST = 1,
	GAMESTATE_STARTUP = GAMESTATE_FIRST,
	GAMESTATE_INIT = 2,
	GAMESTATE_NORMAL = 3,
	GAMESTATE_MAINTAIN = 4,
	GAMESTATE_CLOSED = 5,
	GAMESTATE_CLOSING = 6,
	GAMESTATE_SHUTDOWN = 7,
	GAMESTATE_LAST = GAMESTATE_SHUTDOWN
};

enum LightState_t
{
	LIGHT_STATE_DAY,
	LIGHT_STATE_NIGHT,
	LIGHT_STATE_SUNSET,
	LIGHT_STATE_SUNRISE
};

enum ReloadInfo_t
{
	RELOAD_FIRST = 1,
	RELOAD_ACTIONS = RELOAD_FIRST,
	RELOAD_CHAT = 2,
	RELOAD_CONFIG = 3,
	RELOAD_CREATUREEVENTS = 4,
#ifdef __LOGIN_SERVER__
	RELOAD_GAMESERVERS = 5,
#endif
	RELOAD_GLOBALEVENTS = 6,
	RELOAD_GROUPS = 7,
	RELOAD_HIGHSCORES = 8,
	RELOAD_ITEMS = 9,
	RELOAD_MONSTERS = 10,
	RELOAD_MOVEEVENTS = 11,
	RELOAD_NPCS = 12,
	RELOAD_OUTFITS = 13,
	RELOAD_QUESTS = 14,
	RELOAD_RAIDS = 15,
	RELOAD_SPELLS = 16,
	RELOAD_STAGES = 17,
	RELOAD_TALKACTIONS = 18,
	RELOAD_VOCATIONS = 19,
	RELOAD_WEAPONS = 20,
	RELOAD_MODS = 21,
	RELOAD_ALL = 22,
	RELOAD_LAST = RELOAD_MODS
};

struct RuleViolation
{
	RuleViolation(Player* _reporter, const std::string& _text, uint32_t _time):
		reporter(_reporter), gamemaster(NULL), text(_text), time(_time), isOpen(true) {}

	Player* reporter;
	Player* gamemaster;
	std::string text;
	uint32_t time;
	bool isOpen;

	private:
		RuleViolation(const RuleViolation&);
};

enum SaveFlag_t
{
	SAVE_PLAYERS = 1 << 0,
	SAVE_PLAYERS_SHALLOW = 1 << 1,
	SAVE_MAP = 1 << 2,
	SAVE_STATE = 1 << 3
};

struct RefreshBlock_t
{
	TileItemVector list;
	uint64_t lastRefresh;
};

typedef std::map<uint32_t, shared_ptr<RuleViolation> > RuleViolationsMap;
typedef std::map<Tile*, RefreshBlock_t> RefreshTiles;
typedef std::vector< std::pair<std::string, uint32_t> > Highscore;
typedef std::list<Position> Trash;
typedef std::map<int32_t, float> StageList;

#define EVENT_LIGHTINTERVAL 10000
#define EVENT_DECAYINTERVAL 250
#define EVENT_DECAYBUCKETS 4
#define STATE_DELAY 1000
#define EVENT_WARSINTERVAL 450000

/**
  * Main Game class.
  * This class is responsible to control everything that happens
  */

class Game
{
	public:
		Game();
		virtual ~Game();
		void start(ServiceManager* servicer);

		Highscore getHighscore(uint16_t skill);
		std::string getHighscoreString(uint16_t skill);
		void checkHighscores();
		bool reloadHighscores();

		bool isSwimmingPool(Item* item, const Tile* tile, bool checkProtection) const;

		void prepareGlobalSave(uint8_t minutes);
		void globalSave();

		/**
		  * Load a map.
		  * \param filename Mapfile to load
		  * \returns int32_t 0 built-in spawns, 1 needs xml spawns, 2 needs sql spawns, -1 if got error
		  */
		int32_t loadMap(std::string filename);

		/**
		  * Get the map size - info purpose only
		  * \param width width of the map
		  * \param height height of the map
		  */
		inline void getMapDimensions(uint32_t& width, uint32_t& height) const
		{
			width = map->mapWidth;
			height = map->mapHeight;
		}

		void setWorldType(WorldType_t type) {worldType = type;}
		WorldType_t getWorldType() const {return worldType;}

		Cylinder* internalGetCylinder(Player* player, const Position& pos);
		Thing* internalGetThing(Player* player, const Position& pos, int32_t index,
			uint32_t spriteId = 0, stackposType_t type = STACKPOS_NORMAL);
		void internalGetPosition(Item* item, Position& pos, int16_t& stackpos);

		std::string getTradeErrorDescription(ReturnValue ret, Item* item);

		/**
		  * Get a single tile of the map.
		  * \returns A pointer to the tile
		  */
		Tile* getTile(int32_t x, int32_t y, int32_t z) {return map->getTile(x, y, z);}
		Tile* getTile(const Position& pos) {return map->getTile(pos);}

		/**
		  * Set a single tile of the map, position is read from this tile
		  */
		void setTile(Tile* newTile) {if(map) return map->setTile(newTile->getPosition(), newTile);}

		/**
		  * Get a leaf of the map.
		  * \returns A pointer to a leaf
		  */
		QTreeLeafNode* getLeaf(uint32_t x, uint32_t y) {return map->getLeaf(x, y);}

		/**
		  * Returns a creature based on the unique creature identifier
		  * \param id is the unique creature id to get a creature pointer to
		  * \returns A Creature pointer to the creature
		  */
		Creature* getCreatureByID(uint32_t id);

		/**
		  * Returns a player based on the unique creature identifier
		  * \param id is the unique player id to get a player pointer to
		  * \returns A Pointer to the player
		  */
		Player* getPlayerByID(uint32_t id);

		/**
		  * Returns a player based on guid
		  * \param guid
		  * \returns A Pointer to the player
		  */
		Player* getPlayerByGUID(const uint32_t& guid);

		/**
		  * Returns a creature based on a string name identifier
		  * \param s is the name identifier
		  * \returns A Pointer to the creature
		  */
		Creature* getCreatureByName(std::string s);

		/**
		  * Returns a player based on a string name identifier
		  * \param s is the name identifier
		  * \returns A Pointer to the player
		  */
		Player* getPlayerByName(std::string s);

		/**
		  * Returns a player based on a string name identifier
		  * this function returns a pointer even if the player is offline,
		  * it is up to the caller of the function to delete the pointer - if the player is offline
		  * use isOffline() to determine if the player was offline
		  * \param s is the name identifier
		  * \return A Pointer to the player
		  */
		Player* getPlayerByNameEx(const std::string& s);

		/**
		  * Returns a player based on a guid identifier
		  * this function returns a pointer even if the player is offline,
		  * it is up to the caller of the function to delete the pointer - if the player is offline
		  * use isOffline() to determine if the player was offline
		  * \param guid is the identifier
		  * \return A Pointer to the player
		  */
		Player* getPlayerByGuid(uint32_t guid);

		/**
		  * Returns a player based on a guid identifier
		  * this function returns a pointer even if the player is offline,
		  * it is up to the caller of the function to delete the pointer - if the player is offline
		  * use isOffline() to determine if the player was offline
		  * \param guid is the identifier
		  */
		Player* getPlayerByGuidEx(uint32_t guid);

		/**
		  * Returns a player based on a string name identifier, with support for the "~" wildcard.
		  * \param s is the name identifier, with or without wildcard
		  * \param player will point to the found player (if any)
		  * \return "RET_PLAYERWITHTHISNAMEISNOTONLINE" or "RET_NAMEISTOOAMBIGUOUS"
		  */
		ReturnValue getPlayerByNameWildcard(std::string s, Player*& player);

		/**
		  * Returns a player based on an account number identifier
		  * \param acc is the account identifier
		  * \returns A Pointer to the player
		  */
		Player* getPlayerByAccount(uint32_t acc);

		/**
		  * Returns all players based on their name
		  * \param s is the player name
		  * \return A vector of all players with the selected name
		  */
		PlayerVector getPlayersByName(std::string s);

		/**
		  * Returns all players based on their account number identifier
		  * \param acc is the account identifier
		  * \return A vector of all players with the selected account number
		  */
		PlayerVector getPlayersByAccount(uint32_t acc);

		/**
		  * Returns all players with a certain IP address
		  * \param ip is the IP address of the clients, as an unsigned long
		  * \param mask An IP mask, default 255.255.255.255
		  * \return A vector of all players with the selected IP
		  */
		PlayerVector getPlayersByIP(uint32_t ip, uint32_t mask = 0xFFFFFFFF);

		/**
		  * Place Creature on the map without sending out events to the surrounding.
		  * \param creature Creature to place on the map
		  * \param pos The position to place the creature
		  * \param forced If true, placing the creature will not fail because of obstacles (creatures/items)
		  */
		bool internalPlaceCreature(Creature* creature, const Position& pos, bool extendedPos = false, bool forced = false);

		/**
		  * Place Creature on the map.
		  * \param creature Creature to place on the map
		  * \param pos The position to place the creature
		  * \param forced If true, placing the creature will not fail because of obstacles (creatures/items)
		  */
		bool placeCreature(Creature* creature, const Position& pos, bool extendedPos = false, bool forced = false);
		ReturnValue placeSummon(Creature* creature, const std::string& name);

		/**
		  * Remove Creature from the map.
		  * Removes the Creature the map
		  * \param c Creature to remove
		  */
		bool removeCreature(Creature* creature, bool isLogout = true);

		void addCreatureCheck(Creature* creature);
		void removeCreatureCheck(Creature* creature);

		uint32_t getPlayersOnline() {return (uint32_t)Player::autoList.size();}
		uint32_t getMonstersOnline() {return (uint32_t)Monster::autoList.size();}
		uint32_t getNpcsOnline() {return (uint32_t)Npc::autoList.size();}
		uint32_t getCreaturesOnline() {return (uint32_t)autoList.size();}

		uint32_t getPlayersRecord() const {return playersRecord;}
		void getWorldLightInfo(LightInfo& lightInfo);

		void getSpectators(SpectatorVec& list, const Position& centerPos, bool checkforduplicate = false, bool multifloor = false,
			int32_t minRangeX = 0, int32_t maxRangeX = 0,
			int32_t minRangeY = 0, int32_t maxRangeY = 0)
			{map->getSpectators(list, centerPos, checkforduplicate, multifloor, minRangeX, maxRangeX, minRangeY, maxRangeY);}
		const SpectatorVec& getSpectators(const Position& centerPos) {return map->getSpectators(centerPos);}
		void clearSpectatorCache() {if(map) map->clearSpectatorCache();}

		ReturnValue internalMoveCreature(Creature* creature, Direction direction, uint32_t flags = 0);
		ReturnValue internalMoveCreature(Creature* actor, Creature* creature, Cylinder* fromCylinder,
			Cylinder* toCylinder, uint32_t flags = 0, bool forceTeleport = false);

		ReturnValue internalMoveItem(Creature* actor, Cylinder* fromCylinder, Cylinder* toCylinder, int32_t index,
			Item* item, uint32_t count, Item** _moveItem, uint32_t flags = 0);

		ReturnValue internalAddItem(Creature* actor, Cylinder* toCylinder, Item* item, int32_t index = INDEX_WHEREEVER,
			uint32_t flags = 0, bool test = false);
		ReturnValue internalAddItem(Creature* actor, Cylinder* toCylinder, Item* item, int32_t index,
			uint32_t flags, bool test, uint32_t& remainderCount);
		ReturnValue internalAddItem(Creature* actor, Cylinder* toCylinder, Item* item, int32_t index,
			uint32_t flags, bool test, uint32_t& remainderCount, Item** stackItem);
		ReturnValue internalRemoveItem(Creature* actor, Item* item, int32_t count = -1, bool test = false, uint32_t flags = 0);

		ReturnValue internalPlayerAddItem(Creature* actor, Player* player, Item* item,
			bool dropOnMap = true, slots_t slot = SLOT_WHEREEVER);
		ReturnValue internalPlayerAddItem(Creature* actor, Player* player, Item* item,
			bool dropOnMap, slots_t slot, Item** toItem);

		/**
		  * Find an item of a certain type
		  * \param cylinder to search the item
		  * \param itemId is the item to remove
		  * \param subType is the extra type an item can have such as charges/fluidtype, default is -1
			* meaning it's not used
		  * \param depthSearch if true it will check child containers aswell
		  * \returns A pointer to the item to an item and NULL if not found
		  */
		Item* findItemOfType(Cylinder* cylinder, uint16_t itemId,
			bool depthSearch = true, int32_t subType = -1);

		/**
		  * Remove item(s) of a certain type
		  * \param cylinder to remove the item(s) from
		  * \param itemId is the item to remove
		  * \param count is the amount to remove
		  * \param subType is the extra type an item can have such as charges/fluidtype, default is -1
			* meaning it's not used
		  * \param onlyContainers if true it will remove only items from containers in cylinder, default is false
			* meaning it's disabled
		  * \returns true if the removal was successful
		  */
		bool removeItemOfType(Cylinder* cylinder, uint16_t itemId, int32_t count, int32_t subType = -1, bool onlyContainers = false);

		/**
		  * Get the amount of money in a a cylinder
		  * \returns the amount of money found
		  */
		uint64_t getMoney(const Cylinder* cylinder);

		/**
		  * Remove/Add item(s) with a monetary value
		  * \param cylinder to remove the money from
		  * \param money is the amount to remove
		  * \param flags optional flags to modifiy the default behaviour
		  * \returns true if the removal was successful
		  */
		bool removeMoney(Cylinder* cylinder, int64_t money, uint32_t flags = 0);

		/**
		  * Add item(s) with monetary value
		  * \param cylinder which will receive money
		  * \param money the amount to give
		  * \param flags optional flags to modify default behavior
		  */
		void addMoney(Cylinder* cylinder, int64_t money, uint32_t flags = 0);

		/**
		  * Transform one item to another type/count
		  * \param item is the item to transform
		  * \param newId is the new itemid
		  * \param newCount is the new count value, use default value (-1) to not change it
		  * \returns true if the tranformation was successful
		  */
		Item* transformItem(Item* item, uint16_t newId, int32_t newCount = -1);

		/**
		  * Teleports an object to another position
		  * \param thing is the object to teleport
		  * \param newPos is the new position
		  * \param flags optional flags to modify default behavior
		  * \returns true if the teleportation was successful
		  */
		ReturnValue internalTeleport(Thing* thing, const Position& newPos, bool forceTeleport, uint32_t flags = FLAG_NOLIMIT, bool fullTeleport = true);

		/**
			* Turn a creature to a different direction.
			* \param creature Creature to change the direction
			* \param dir Direction to turn to
			*/
		bool internalCreatureTurn(Creature* creature, Direction dir);

		/**
		  * Creature wants to say something.
		  * \param creature Creature pointer
		  * \param type Type of message
		  * \param text The text to say
		  * \param ghostMode Is creature on ghost mode
		  * \param spectators Send message only to creatures pointed in vector
		  * \param pos Appear as sent from different position
		  */
		bool internalCreatureSay(Creature* creature, MessageClasses type, const std::string& text,
			bool ghostMode, SpectatorVec* spectators = NULL, Position* pos = NULL, uint32_t statementId = 0);

		bool internalStartTrade(Player* player, Player* partner, Item* tradeItem);
		bool internalCloseTrade(Player* player);

		//Implementation of player invoked events
		bool playerBroadcastMessage(Player* player, MessageClasses type, const std::string& text, uint32_t statementId);
		bool playerReportBug(uint32_t playerId, std::string comment);
		bool playerReportViolation(uint32_t playerId, ReportType_t type, uint8_t reason, const std::string& name,
			const std::string& comment, const std::string& translation, uint32_t statementId);
		bool playerViolationWindow(uint32_t playerId, std::string name, uint8_t reason,
			ViolationAction_t action, std::string comment, std::string statement,
			uint32_t statementId, bool ipBanishment);
		bool playerMoveThing(uint32_t playerId, const Position& fromPos, uint16_t spriteId,
			int16_t fromStackpos, const Position& toPos, uint8_t count);
		bool playerMoveCreature(uint32_t playerId, uint32_t movingCreatureId,
			const Position& movingCreatureOrigPos, const Position& toPos, bool delay);
		bool playerMoveItem(uint32_t playerId, const Position& fromPos,
			uint16_t spriteId, int16_t fromStackpos, const Position& toPos, uint8_t count);
		bool playerMove(uint32_t playerId, Direction dir);
		bool playerCreatePrivateChannel(uint32_t playerId);
		bool playerChannelInvite(uint32_t playerId, const std::string& name);
		bool playerChannelExclude(uint32_t playerId, const std::string& name);
		bool playerRequestChannels(uint32_t playerId);
		bool playerOpenChannel(uint32_t playerId, uint16_t channelId);
		bool playerCloseChannel(uint32_t playerId, uint16_t channelId);
		bool playerOpenPrivateChannel(uint32_t playerId, std::string& receiver);
		bool playerCloseNpcChannel(uint32_t playerId);
		bool playerProcessRuleViolation(uint32_t playerId, const std::string& name);
		bool playerCloseRuleViolation(uint32_t playerId, const std::string& name);
		bool playerCancelRuleViolation(uint32_t playerId);
		bool playerReceivePing(uint32_t playerId);
		bool playerAutoWalk(uint32_t playerId, std::list<Direction>& listDir);
		bool playerStopAutoWalk(uint32_t playerId);
		bool playerUseItemEx(uint32_t playerId, const Position& fromPos, int16_t fromStackpos,
			uint16_t fromSpriteId, const Position& toPos, int16_t toStackpos, uint16_t toSpriteId, bool isHotkey);
		bool playerUseItem(uint32_t playerId, const Position& pos, int16_t stackpos,
			uint8_t index, uint16_t spriteId, bool isHotkey);
		bool playerUseBattleWindow(uint32_t playerId, const Position& fromPos,
			int16_t fromStackpos, uint32_t creatureId, uint16_t spriteId, bool isHotkey);
		bool playerCloseContainer(uint32_t playerId, uint8_t cid);
		bool playerMoveUpContainer(uint32_t playerId, uint8_t cid);
		bool playerUpdateContainer(uint32_t playerId, uint8_t cid);
		bool playerUpdateTile(uint32_t playerId, const Position& pos);
		bool playerRotateItem(uint32_t playerId, const Position& pos, int16_t stackpos, const uint16_t spriteId);
		bool playerWriteItem(uint32_t playerId, uint32_t windowTextId, const std::string& text);
		bool playerUpdateHouseWindow(uint32_t playerId, uint8_t listId, uint32_t windowTextId, const std::string& text);
		bool playerRequestTrade(uint32_t playerId, const Position& pos, int16_t stackpos,
			uint32_t tradePlayerId, uint16_t spriteId);
		bool playerAcceptTrade(uint32_t playerId);
		bool playerLookInTrade(uint32_t playerId, bool lookAtCounterOffer, int index);
		bool playerPurchaseItem(uint32_t playerId, uint16_t spriteId, uint8_t count, uint8_t amount,
			bool ignoreCap = false, bool inBackpacks = false);
		bool playerSellItem(uint32_t playerId, uint16_t spriteId, uint8_t count, uint8_t amount,
			bool ignoreEquipped = false);
		bool playerCloseShop(uint32_t playerId);
		bool playerLookInShop(uint32_t playerId, uint16_t spriteId, uint8_t count);
		bool playerCloseTrade(uint32_t playerId);
		bool playerSetAttackedCreature(uint32_t playerId, uint32_t creatureId);
		bool playerFollowCreature(uint32_t playerId, uint32_t creatureId);
		bool playerCancelAttackAndFollow(uint32_t playerId);
		bool playerSetFightModes(uint32_t playerId, fightMode_t fightMode, chaseMode_t chaseMode, secureMode_t secureMode);
		bool playerLookAt(uint32_t playerId, const Position& pos, uint16_t spriteId, int16_t stackpos);
		bool playerLookInBattleList(uint32_t playerId, uint32_t creatureId);
		bool playerQuests(uint32_t playerId);
		bool playerQuestInfo(uint32_t playerId, uint16_t questId);
		bool playerRequestAddVip(uint32_t playerId, const std::string& name);
		bool playerRequestRemoveVip(uint32_t playerId, uint32_t guid);
		bool playerTurn(uint32_t playerId, Direction dir);
		bool playerRequestOutfit(uint32_t playerId);
		bool playerSay(uint32_t playerId, uint16_t channelId, MessageClasses type,
			const std::string& receiver, const std::string& text);
		bool playerChangeOutfit(uint32_t playerId, Outfit_t outfit);
		bool playerInviteToParty(uint32_t playerId, uint32_t invitedId);
		bool playerJoinParty(uint32_t playerId, uint32_t leaderId);
		bool playerRevokePartyInvitation(uint32_t playerId, uint32_t invitedId);
		bool playerPassPartyLeadership(uint32_t playerId, uint32_t newLeaderId);
		bool playerLeaveParty(uint32_t playerId, bool forced = false);
		bool playerSharePartyExperience(uint32_t playerId, bool activate);
		void parsePlayerExtendedOpcode(uint32_t playerId, uint8_t opcode, const std::string& buffer);

		void kickPlayer(uint32_t playerId, bool displayEffect);
		bool broadcastMessage(const std::string& text, MessageClasses type);
		void showHotkeyUseMessage(Player* player, Item* item);

		int32_t getMotdId();
		void loadMotd();
		void loadPlayersRecord();
		void checkPlayersRecord(Player* player);

		bool reloadInfo(ReloadInfo_t reload, uint32_t playerId = 0, bool completeReload = false);
		void cleanup();
		void shutdown();
		void freeThing(Thing* thing);

		bool canThrowObjectTo(const Position& fromPos, const Position& toPos, bool checkLineOfSight = true,
			int32_t rangex = Map::maxClientViewportX, int32_t rangey = Map::maxClientViewportY);
		bool isSightClear(const Position& fromPos, const Position& toPos, bool sameFloor);

		bool getPathTo(const Creature* creature, const Position& destPos,
			std::list<Direction>& listDir, int32_t maxSearchDist /*= -1*/);

		bool getPathToEx(const Creature* creature, const Position& targetPos, std::list<Direction>& dirList,
			const FindPathParams& fpp);

		bool getPathToEx(const Creature* creature, const Position& targetPos, std::list<Direction>& dirList,
			uint32_t minTargetDist, uint32_t maxTargetDist, bool fullPathSearch = true,
			bool clearSight = true, int32_t maxSearchDist = -1);

		bool steerCreature(Creature* creature, const Position& position, uint16_t maxNodes/* = 100*/);

		Position getClosestFreeTile(Creature* creature, Position pos, bool extended = false, bool ignoreHouse = true);
		std::string getSearchString(const Position& fromPos, const Position& toPos, bool fromIsCreature = false, bool toIsCreature = false);

		void changeLight(const Creature* creature);
		void changeSpeed(Creature* creature, int32_t varSpeedDelta);

		void internalCreatureChangeOutfit(Creature* creature, const Outfit_t& oufit, bool forced = false);
		void internalCreatureChangeVisible(Creature* creature, Visible_t visible);

		void updateCreatureSkull(Creature* creature);
		void updateCreatureShield(Creature* creature);
		void updateCreatureEmblem(Creature* creature);
		void updateCreatureWalkthrough(Creature* creature);

		GameState_t getGameState() const {return gameState;}
		void setGameState(GameState_t newState);

		void saveGameState(uint8_t flags);
		void loadGameState();

		void cleanMapEx(uint32_t& count);
		void cleanMap();

		void refreshMap(RefreshTiles::iterator* it = NULL, uint32_t limit = 0);
		void proceduralRefresh(RefreshTiles::iterator* it = NULL);

		void addTrash(Position pos) {trash.push_back(pos);}
		void addRefreshTile(Tile* tile, RefreshBlock_t rb) {refreshTiles[tile] = rb;}

		//Events
		void checkCreatureWalk(uint32_t creatureId);
		void updateCreatureWalk(uint32_t creatureId);
		void checkCreatureAttack(uint32_t creatureId);
		void checkCreatures();
		void checkLight();
		void checkWars();

		bool combatBlockHit(CombatType_t combatType, Creature* attacker, Creature* target,
			int32_t& healthChange, bool checkDefense, bool checkArmor, bool field = false, bool element = false);

		bool combatChangeHealth(CombatType_t combatType, Creature* attacker, Creature* target, int32_t healthChange,
			MagicEffect_t hitEffect = MAGIC_EFFECT_UNKNOWN, Color_t hitColor = COLOR_UNKNOWN, bool force = false);
		bool combatChangeHealth(const CombatParams& params, Creature* attacker, Creature* target, int32_t healthChange, bool force);
		bool combatChangeMana(Creature* attacker, Creature* target, int32_t manaChange,
			CombatType_t combatType = COMBAT_MANADRAIN, bool inherited = false);

		//animation help functions
		void addCreatureHealth(const Creature* target);
		void addCreatureHealth(const SpectatorVec& list, const Creature* target);
		void addCreatureSquare(const Creature* target, uint8_t squareColor);
		void addCreatureSquare(const SpectatorVec& list, const Creature* target, uint8_t squareColor);

		void addAnimatedText(const Position& pos, uint8_t textColor, const std::string& text);
		void addAnimatedText(const SpectatorVec& list, const Position& pos, uint8_t textColor, const std::string& text);
		void addMagicEffect(const Position& pos, uint8_t effect, bool ghostMode = false);
		void addMagicEffect(const SpectatorVec& list, const Position& pos, uint8_t effect, bool ghostMode = false);
		void addDistanceEffect(const SpectatorVec& list, const Position& fromPos, const Position& toPos, uint8_t effect);
		void addDistanceEffect(const Position& fromPos, const Position& toPos, uint8_t effect);

		void addStatsMessage(const SpectatorVec& list, MessageClasses mClass, const std::string& message,
			const Position& pos, MessageDetails* details = NULL);

		const RuleViolationsMap& getRuleViolations() const {return ruleViolations;}
		bool cancelRuleViolation(Player* player);
		bool closeRuleViolation(Player* player);

		bool loadExperienceStages();
		double getExperienceStage(uint32_t level, double divider = 1.);

		inline StageList::const_iterator getFirstStage() const {return stages.begin();}
		inline StageList::const_iterator getLastStage() const {return stages.end();}
		size_t getStagesCount() const {return stages.size();}

		Map* getMap() {return map;}
		const Map* getMap() const {return map;}

		bool isRunning() const {return services && services->isRunning();}
		int32_t getLightHour() const {return lightHour;}
		void startDecay(Item* item);

#ifdef __GROUND_CACHE__
		std::map<Item*, int32_t> grounds;
#endif

	protected:
		bool playerWhisper(Player* player, const std::string& text, uint32_t statementId);
		bool playerYell(Player* player, const std::string& text, uint32_t statementId);
		bool playerSpeakTo(Player* player, MessageClasses type, const std::string& receiver, const std::string& text, uint32_t statementId);
		bool playerSpeakToChannel(Player* player, MessageClasses type, const std::string& text, uint16_t channelId, uint32_t statementId);
		bool playerSpeakToNpc(Player* player, const std::string& text);
		bool playerReportRuleViolation(Player* player, const std::string& text);
		bool playerContinueReport(Player* player, const std::string& text);

		struct GameEvent
		{
			int64_t tick;
			int32_t type;
			void* data;
		};

		std::vector<Thing*> releaseThings;
		std::map<Item*, uint32_t> tradeItems;
		AutoList<Creature> autoList;
		RuleViolationsMap ruleViolations;

		size_t checkCreatureLastIndex;
		std::vector<Creature*> checkCreatureVectors[EVENT_CREATURECOUNT];
		std::vector<Creature*> toAddCheckCreatureVector;

		void checkDecay();
		void internalDecayItem(Item* item);

		typedef std::list<Item*> DecayList;
		DecayList decayItems[EVENT_DECAYBUCKETS];
		DecayList toDecayItems;
		size_t lastBucket;

		static const int32_t LIGHT_LEVEL_DAY = 250;
		static const int32_t LIGHT_LEVEL_NIGHT = 40;
		static const int32_t SUNSET = 1305;
		static const int32_t SUNRISE = 430;
		int32_t lightLevel, lightHour, lightHourDelta;
		LightState_t lightState;

		GameState_t gameState;
		WorldType_t worldType;

		ServiceManager* services;
		Map* map;

		std::string lastMotd;
		int32_t lastMotdId;
		uint32_t playersRecord;
		uint32_t checkLightEvent, checkCreatureEvent, checkDecayEvent, saveEvent;
		uint32_t checkWarsEvent;
		bool checkEndingWars;

		RefreshTiles refreshTiles;
		Trash trash;

		StageList stages;
		uint32_t lastStageLevel;

		Highscore highscoreStorage[9];
		time_t lastHighscoreCheck;
};
#endif
