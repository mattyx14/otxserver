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
#include "game.h"

#include "configmanager.h"
#ifdef __LOGIN_SERVER__
#include "gameservers.h"
#endif
#include "server.h"
#include "chat.h"

#include "luascript.h"
#include "creature.h"
#include "combat.h"
#include "tile.h"

#include "database.h"
#include "iologindata.h"
#include "ioban.h"
#include "ioguild.h"

#include "items.h"
#include "trashholder.h"
#include "container.h"
#include "monsters.h"

#include "house.h"

#include "actions.h"
#include "globalevent.h"
#include "movement.h"
#include "raids.h"
#include "scriptmanager.h"
#include "spells.h"
#include "talkaction.h"
#include "weapons.h"

#include "textlogger.h"
#include "vocation.h"
#include "group.h"

#ifdef __EXCEPTION_TRACER__
#include "exception.h"
#endif

extern ConfigManager g_config;
extern Actions* g_actions;
extern Monsters g_monsters;
extern Npcs g_npcs;
extern Chat g_chat;
extern TalkActions* g_talkActions;
extern Spells* g_spells;
extern MoveEvents* g_moveEvents;
extern Weapons* g_weapons;
extern CreatureEvents* g_creatureEvents;
extern GlobalEvents* g_globalEvents;

Game::Game()
{
	gameState = GAMESTATE_NORMAL;
	worldType = WORLDTYPE_OPEN;
	map = NULL;
	playersRecord = lastStageLevel = 0;

	//(1440 minutes/day) * 10 seconds event interval / (3600 seconds/day)
	lightHourDelta = 1440 * 10 / 3600;
	lightHour = SUNRISE + (SUNSET - SUNRISE) / 2;
	lightLevel = LIGHT_LEVEL_DAY;
	lightState = LIGHT_STATE_DAY;

	lastBucket = checkCreatureLastIndex = checkLightEvent = checkCreatureEvent = checkDecayEvent = saveEvent = 0;
	checkWarsEvent = 0;
	checkEndingWars = false;
}

Game::~Game()
{
	delete map;
}

void Game::start(ServiceManager* servicer)
{
	checkDecayEvent = Scheduler::getInstance().addEvent(createSchedulerTask(EVENT_DECAYINTERVAL,
		boost::bind(&Game::checkDecay, this)));
	checkCreatureEvent = Scheduler::getInstance().addEvent(createSchedulerTask(EVENT_CREATURE_THINK_INTERVAL,
		boost::bind(&Game::checkCreatures, this)));
	checkLightEvent = Scheduler::getInstance().addEvent(createSchedulerTask(EVENT_LIGHTINTERVAL,
		boost::bind(&Game::checkLight, this)));
	checkWarsEvent = Scheduler::getInstance().addEvent(createSchedulerTask(EVENT_WARSINTERVAL,
		boost::bind(&Game::checkWars, this)));

	services = servicer;
	if(!g_config.getBool(ConfigManager::GLOBALSAVE_ENABLED))
		return;

	int32_t prepareHour = g_config.getNumber(ConfigManager::GLOBALSAVE_H),
		prepareMinute = g_config.getNumber(ConfigManager::GLOBALSAVE_M);

	if(prepareHour < 0 || prepareHour > 24)
	{
		std::clog << "> WARNING: No valid hour (" << prepareHour << ") for a global save, should be between 0-23. Global save disabled." << std::endl;
		return;
	}

	if(prepareMinute < 0 || prepareMinute > 59)
	{
		std::clog << "> WARNING: No valid minute (" << prepareMinute << ") for a global save, should be between 0-59. Global save disabled." << std::endl;
		return;
	}

	time_t timeNow = time(NULL);
	const tm* theTime = localtime(&timeNow);

	int32_t hour = theTime->tm_hour, minute = theTime->tm_min, second = theTime->tm_sec,
		hoursLeft = 0, minutesLeft = 0, broadcast = 5;

	if(!prepareHour)
		prepareHour = 24;

	if(hour != prepareHour)
	{
		if(prepareMinute >= 5)
			prepareMinute -= 5;
		else
		{
			prepareHour--;
			prepareMinute = 55 + prepareMinute;
		}

		if(hour > prepareHour)
			hoursLeft = 24 - (hour - prepareHour);
		else
			hoursLeft = prepareHour - hour;

		if(minute > prepareMinute)
		{
			minutesLeft = 60 - (minute - prepareMinute);
			hoursLeft--;
		}
		else if(minute != prepareMinute)
			minutesLeft = prepareMinute - minute;
	}
	else
	{
		if(minute > prepareMinute)
		{
			minutesLeft = 55 - (minute - prepareMinute);
			hoursLeft = 23;
		}
		else
		{
			minutesLeft = prepareMinute - minute;
			if(minutesLeft >= 5)
				minutesLeft = minutesLeft - 5;
			else if(minutesLeft == 3 || minutesLeft == 1)
			{
				prepareGlobalSave(minutesLeft);
				return;
			}
			else if(minutesLeft > 0)
			{
				broadcast = (minutesLeft == 2 ? 1 : 3);
				minutesLeft = 1;
			}
		}
	}

	uint32_t timeLeft = (hoursLeft * 60 * 60 * 1000) + (minutesLeft * 60 * 1000);
	if(timeLeft > 0)
	{
		timeLeft -= second * 1000;
		saveEvent = Scheduler::getInstance().addEvent(createSchedulerTask(timeLeft,
			boost::bind(&Game::prepareGlobalSave, this, broadcast)));
	}
}

void Game::loadGameState()
{
	ScriptEnviroment::loadGameState();
	loadPlayersRecord();
	loadMotd();
}

void Game::setGameState(GameState_t newState)
{
	if(gameState == GAMESTATE_SHUTDOWN)
		return; //this cannot be stopped

	if(gameState != newState)
	{
		gameState = newState;
		switch(newState)
		{
			case GAMESTATE_INIT:
			{
				Spawns::getInstance()->startup();
				Raids::getInstance()->startup();

				loadGameState();

				g_globalEvents->startup();
				if(g_config.getBool(ConfigManager::INIT_PREMIUM_UPDATE))
					IOLoginData::getInstance()->updatePremiumDays();

				IOGuild::getInstance()->checkWars();
				IOGuild::getInstance()->checkEndingWars();
				break;
			}

			case GAMESTATE_SHUTDOWN:
			{
				g_globalEvents->execute(GLOBALEVENT_SHUTDOWN);
				AutoList<Player>::iterator it = Player::autoList.begin();
				while(it != Player::autoList.end()) //kick all players that are still online
				{
					it->second->kick(true, true);
					it = Player::autoList.begin();
				}

				Houses::getInstance()->check();
				saveGameState((uint8_t)SAVE_PLAYERS | (uint8_t)SAVE_MAP | (uint8_t)SAVE_STATE);
				Dispatcher::getInstance().addTask(createTask(boost::bind(&Game::shutdown, this)));

				Scheduler::getInstance().stop();
				Dispatcher::getInstance().stop();
				break;
			}

			case GAMESTATE_CLOSED:
			{
				AutoList<Player>::iterator it = Player::autoList.begin();
				while(it != Player::autoList.end()) //kick all players who not allowed to stay
				{
					if(!it->second->hasFlag(PlayerFlag_CanAlwaysLogin))
					{
						it->second->kick(true, true);
						it = Player::autoList.begin();
					}
					else
						++it;
				}

				map->updateAuctions();
				saveGameState((uint8_t)SAVE_PLAYERS | (uint8_t)SAVE_MAP | (uint8_t)SAVE_STATE);
				break;
			}

			case GAMESTATE_NORMAL:
			case GAMESTATE_MAINTAIN:
			case GAMESTATE_STARTUP:
			case GAMESTATE_CLOSING:
			default:
				break;
		}
	}
}

void Game::saveGameState(uint8_t flags)
{
	std::clog << "> Saving server..." << std::endl;
	uint64_t start = OTSYS_TIME();
	if(gameState == GAMESTATE_NORMAL)
		setGameState(GAMESTATE_MAINTAIN);

	if(hasBitSet(SAVE_PLAYERS, flags))
	{
		IOLoginData* io = IOLoginData::getInstance();
		for(AutoList<Player>::iterator it = Player::autoList.begin(); it != Player::autoList.end(); ++it)
		{
			it->second->loginPosition = it->second->getPosition();
			io->savePlayer(it->second, false, hasBitSet(SAVE_PLAYERS_SHALLOW, flags));
		}
	}

	if(hasBitSet(SAVE_MAP, flags))
		map->saveMap();

	if(hasBitSet(SAVE_STATE, flags))
		ScriptEnviroment::saveGameState();

	if(gameState == GAMESTATE_MAINTAIN)
		setGameState(GAMESTATE_NORMAL);

	std::clog << "> SAVE: Complete in " << (OTSYS_TIME() - start) / (1000.) << " seconds using "
		<< asLowerCaseString(g_config.getString(ConfigManager::HOUSE_STORAGE))
		<< " house storage." << std::endl;
}

int32_t Game::loadMap(std::string filename)
{
	if(!map)
		map = new Map;

	#ifdef _MULTIPLATFORM76
	std::string file = getFilePath(FILE_TYPE_CONFIG, "world/" + filename);
	if(!fileExists(file.c_str()))
		file = getFilePath(FILE_TYPE_OTHER, "world/" + filename);
	#else
	std::string file = getFilePath(FILE_TYPE_CONFIG, "world/" + ITEMS_PATH + "/" + filename);
	if(!fileExists(file.c_str()))
		file = getFilePath(FILE_TYPE_OTHER, "world/" + ITEMS_PATH + "/" + filename);
	#endif

	return map->loadMap(file);
}

void Game::cleanMapEx(uint32_t& count)
{
	uint64_t start = OTSYS_TIME();
	uint32_t tiles = 0; count = 0;

	int32_t marked = -1;
	if(gameState == GAMESTATE_NORMAL)
		setGameState(GAMESTATE_MAINTAIN);

	Tile* tile = NULL;
	ItemVector::iterator tit;
	if(g_config.getBool(ConfigManager::STORE_TRASH))
	{
		marked = trash.size();
		Trash::iterator it = trash.begin();
		if(g_config.getBool(ConfigManager::CLEAN_PROTECTED_ZONES))
		{
			for(; it != trash.end(); ++it)
			{
				if(!(tile = getTile(*it)))
					continue;

				tile->resetFlag(TILESTATE_TRASHED);
				if(tile->hasFlag(TILESTATE_HOUSE) || !tile->getItemList())
					continue;

				++tiles;
				tit = tile->getItemList()->begin();
				while(tile->getItemList() && tit != tile->getItemList()->end())
				{
					if((*tit)->isMovable() && !(*tit)->isLoadedFromMap()
						&& !(*tit)->isScriptProtected())
					{
						internalRemoveItem(NULL, *tit);
						if(tile->getItemList())
							tit = tile->getItemList()->begin();

						++count;
					}
					else
						++tit;
				}
			}
		}
		else
		{
			for(; it != trash.end(); ++it)
			{
				if(!(tile = getTile(*it)))
					continue;

				tile->resetFlag(TILESTATE_TRASHED);
				if(tile->hasFlag(TILESTATE_PROTECTIONZONE) || !tile->getItemList())
					continue;

				++tiles;
				tit = tile->getItemList()->begin();
				while(tile->getItemList() && tit != tile->getItemList()->end())
				{
					if((*tit)->isMovable() && !(*tit)->isLoadedFromMap()
						&& !(*tit)->isScriptProtected())
					{
						internalRemoveItem(NULL, *tit);
						if(tile->getItemList())
							tit = tile->getItemList()->begin();

						++count;
					}
					else
						++tit;
				}
			}
		}

		trash.clear();
	}
	else if(g_config.getBool(ConfigManager::CLEAN_PROTECTED_ZONES))
	{
		for(uint16_t z = 0; z < (uint16_t)MAP_MAX_LAYERS; z++)
		{
			for(uint16_t y = 1; y <= map->mapHeight; y++)
			{
				for(uint16_t x = 1; x <= map->mapWidth; x++)
				{
					if(!(tile = getTile(x, y, z)) || tile->hasFlag(TILESTATE_HOUSE) || !tile->getItemList())
						continue;

					++tiles;
					tit = tile->getItemList()->begin();
					while(tile->getItemList() && tit != tile->getItemList()->end())
					{
						if((*tit)->isMovable() && !(*tit)->isLoadedFromMap()
							&& !(*tit)->isScriptProtected())
						{
							internalRemoveItem(NULL, *tit);
							if(tile->getItemList())
								tit = tile->getItemList()->begin();

							++count;
						}
						else
							++tit;
					}
				}
			}
		}
	}
	else
	{
		for(uint16_t z = 0; z < (uint16_t)MAP_MAX_LAYERS; z++)
		{
			for(uint16_t y = 1; y <= map->mapHeight; y++)
			{
				for(uint16_t x = 1; x <= map->mapWidth; x++)
				{
					if(!(tile = getTile(x, y, z)) || tile->hasFlag(TILESTATE_PROTECTIONZONE) || !tile->getItemList())
						continue;

					++tiles;
					tit = tile->getItemList()->begin();
					while(tile->getItemList() && tit != tile->getItemList()->end())
					{
						if((*tit)->isMovable() && !(*tit)->isLoadedFromMap()
							&& !(*tit)->isScriptProtected())
						{
							internalRemoveItem(NULL, *tit);
							if(tile->getItemList())
								tit = tile->getItemList()->begin();

							++count;
						}
						else
							++tit;
					}
				}
			}
		}
	}

	if(gameState == GAMESTATE_MAINTAIN)
		setGameState(GAMESTATE_NORMAL);

	std::clog << "> CLEAN: Removed " << count << " item" << (count != 1 ? "s" : "")
		<< " from " << tiles << " tile" << (tiles != 1 ? "s" : "");
	if(marked >= 0)
		std::clog << " (" << marked << " were marked)";

	std::clog << " in " << (OTSYS_TIME() - start) / (1000.) << " seconds." << std::endl;
}

void Game::cleanMap()
{
	uint32_t dummy;
	cleanMapEx(dummy);
}

void Game::proceduralRefresh(RefreshTiles::iterator* it/* = NULL*/)
{
	if(!it)
		it = new RefreshTiles::iterator(refreshTiles.begin());

	// Refresh 250 tiles each cycle
	refreshMap(it, 250);
	if((*it) != refreshTiles.end())
	{
		delete it;
		return;
	}

	// Refresh some items every 100 ms until all tiles has been checked
	// For 100k tiles, this would take 100000/2500 = 40s = half a minute
	Scheduler::getInstance().addEvent(createSchedulerTask(SCHEDULER_MINTICKS,
		boost::bind(&Game::proceduralRefresh, this, it)));
}

void Game::refreshMap(RefreshTiles::iterator* it/* = NULL*/, uint32_t limit/* = 0*/)
{
	RefreshTiles::iterator end = refreshTiles.end();
	if(!it)
	{
		RefreshTiles::iterator begin = refreshTiles.begin();
		it = &begin;
	}

	Tile* tile = NULL;
	TileItemVector* items = NULL;

	Item* item = NULL;
	for(uint32_t cleaned = 0, downItemsSize = 0; (*it) != end &&
		(limit ? (cleaned < limit) : true); ++(*it), ++cleaned)
	{
		if(!(tile = (*it)->first))
			continue;

		if((items = tile->getItemList()))
		{
			downItemsSize = tile->getDownItemCount();
			for(uint32_t i = downItemsSize - 1; i; --i)
			{
				if((item = items->at(i)))
				{
					#ifndef __DEBUG__
					internalRemoveItem(NULL, item);
					#else
					if(internalRemoveItem(NULL, item) != RET_NOERROR)
					{
						std::clog << "> WARNING: Could not refresh item: " << item->getID();
						std::clog << " at position: " << tile->getPosition() << std::endl;
					}
					#endif
				}
			}
		}

		cleanup();
		TileItemVector list = (*it)->second.list;
		for(ItemVector::reverse_iterator it = list.rbegin(); it != list.rend(); ++it)
		{
			Item* item = (*it)->clone();
			if(internalAddItem(NULL, tile, item, INDEX_WHEREEVER, FLAG_NOLIMIT) == RET_NOERROR)
			{
				if(item->getUniqueId())
					ScriptEnviroment::addUniqueThing(item);

				startDecay(item);
			}
			else
			{
				std::clog << "> WARNING: Could not refresh item: " << item->getID()
					<< " at position: " << tile->getPosition() << std::endl;
				delete item;
			}
		}
	}
}

bool Game::isSwimmingPool(Item* item, const Tile* tile, bool checkProtection) const
{
	if(!tile)
		return false;

	TrashHolder* trashHolder = NULL;
	if(!item)
		trashHolder = tile->getTrashHolder();
	else
		trashHolder = item->getTrashHolder();

	return trashHolder && trashHolder->getEffect() == MAGIC_EFFECT_LOSE_ENERGY && (!checkProtection
		|| tile->getZone() == ZONE_PROTECTION || tile->getZone() == ZONE_OPTIONAL);
}

Cylinder* Game::internalGetCylinder(Player* player, const Position& pos)
{
	if(pos.x != 0xFFFF)
		return getTile(pos);

	//container
	if(pos.y & 0x40)
		return player->getContainer((uint8_t)(pos.y & 0x0F));

	return player;
}

Thing* Game::internalGetThing(Player* player, const Position& pos, int32_t index,
	uint32_t spriteId/* = 0*/, stackposType_t type/* = STACKPOS_NORMAL*/)
{
	if(pos.x != 0xFFFF)
	{
		Tile* tile = getTile(pos);
		if(!tile)
			return NULL;

		if(type == STACKPOS_LOOK)
			return tile->getTopVisibleThing(player);

		Thing* thing = NULL;
		switch(type)
		{
			case STACKPOS_MOVE:
			{
				Item* item = tile->getTopDownItem();
				if(item && item->isMovable())
					thing = item;
				else
					thing = tile->getTopVisibleCreature(player);

				break;
			}

			case STACKPOS_USE:
			{
				thing = tile->getTopDownItem();
				break;
			}

			case STACKPOS_USEITEM:
			{
				thing = tile->getTopDownItem();
				#ifdef _MULTIPLATFORM76
				Item* item = tile->getItemByTopOrder(2);
				#else
				Item* item = tile->getItemByTopOrder(1);
				#endif
				if(item && g_actions->hasAction(item))
				{
					const ItemType& it = Item::items[item->getID()];
					if(!thing || (!it.hasHeight && !it.allowPickupable))
						thing = item;
				}

				if(!thing)
					thing = tile->getTopTopItem();

				if(!thing)
					thing = tile->ground;

				break;
			}

			default:
				thing = tile->__getThing(index);
				break;
		}

		if(player && thing && thing->getItem())
		{
			if(tile->hasProperty(ISVERTICAL))
			{
				if(player->getPosition().x + 1 == tile->getPosition().x)
					thing = NULL;
			}
			else if(tile->hasProperty(ISHORIZONTAL) && player->getPosition().y + 1 == tile->getPosition().y)
				thing = NULL;
		}

		return thing;
	}
	else if(pos.y & 0x40)
	{
		uint8_t fromCid = pos.y & 0x0F, slot = pos.z;
		if(Container* parentcontainer = player->getContainer(fromCid))
			return parentcontainer->getItem(slot);
	}
	else if(!pos.y && !pos.z)
	{
		const ItemType& it = Item::items.getItemIdByClientId(spriteId);
		if(!it.id)
			return NULL;

		int32_t subType = -1;
		if(it.isFluidContainer() && index < int32_t(sizeof(reverseFluidMap) / sizeof(int8_t)))
			subType = reverseFluidMap[index];

		return findItemOfType(player, it.id, true, subType);
	}

	return player->getInventoryItem((slots_t)static_cast<uint8_t>(pos.y));
}

void Game::internalGetPosition(Item* item, Position& pos, int16_t& stackpos)
{
	if(!item)
		return;

	pos.x = pos.y = pos.z = stackpos = 0;
	if(Cylinder* topParent = item->getTopParent())
	{
		if(Player* player = dynamic_cast<Player*>(topParent))
		{
			pos.x = 0xFFFF;

			Container* container = dynamic_cast<Container*>(item->getParent());
			if(container)
			{
				pos.y = ((uint16_t) ((uint16_t)0x40) | ((uint16_t)player->getContainerID(container)) );
				pos.z = container->__getIndexOfThing(item);
				stackpos = pos.z;
			}
			else
			{
				pos.y = player->__getIndexOfThing(item);
				stackpos = pos.y;
			}
		}
		else if(Tile* tile = topParent->getTile())
		{
			pos = tile->getPosition();
			stackpos = tile->__getIndexOfThing(item);
		}
	}
}

Creature* Game::getCreatureByID(uint32_t id)
{
	if(!id)
		return NULL;

	AutoList<Creature>::iterator it = autoList.find(id);
	if(it != autoList.end() && !it->second->isRemoved())
		return it->second;

	return NULL; //just in case the player doesnt exist
}

Player* Game::getPlayerByID(uint32_t id)
{
	if(!id)
		return NULL;

	AutoList<Player>::iterator it = Player::autoList.find(id);
	if(it != Player::autoList.end() && !it->second->isRemoved())
		return it->second;

	return NULL; //just in case the player doesnt exist
}

Player* Game::getPlayerByGUID(const uint32_t& guid)
{
	if(!guid)
		return NULL;

	for(AutoList<Player>::iterator it = Player::autoList.begin(); it != Player::autoList.end(); ++it)
	{
		Player* player = (*it).second;
		if(!player->isRemoved())
		{
			if(guid == player->getGUID())
				return player;
		}
	}
	return NULL;
}

Creature* Game::getCreatureByName(std::string s)
{
	if(s.empty())
		return NULL;

	toLowerCaseString(s);
	for(AutoList<Creature>::iterator it = autoList.begin(); it != autoList.end(); ++it)
	{
		if(!it->second->isRemoved() && asLowerCaseString(it->second->getName()) == s)
			return it->second;
	}

	return NULL; //just in case the creature doesnt exist
}

Player* Game::getPlayerByName(std::string s)
{
	if(s.empty())
		return NULL;

	toLowerCaseString(s);
	for(AutoList<Player>::iterator it = Player::autoList.begin(); it != Player::autoList.end(); ++it)
	{
		if(!it->second->isRemoved() && asLowerCaseString(it->second->getName()) == s)
			return it->second;
	}

	return NULL;
}

Player* Game::getPlayerByNameEx(const std::string& s)
{
	Player* player = getPlayerByName(s);
	if(player)
		return player;

	std::string name = s;
	if(!IOLoginData::getInstance()->playerExists(name))
		return NULL;

	player = new Player(name, NULL);
	if(IOLoginData::getInstance()->loadPlayer(player, name))
		return player;

#ifdef __DEBUG__
	std::clog << "[Failure - Game::getPlayerByNameEx] Cannot load player: " << name << std::endl;
#endif
	delete player;
	return NULL;
}

Player* Game::getPlayerByGuid(uint32_t guid)
{
	if(!guid)
		return NULL;

	for(AutoList<Player>::iterator it = Player::autoList.begin(); it != Player::autoList.end(); ++it)
	{
		if(!it->second->isRemoved() && it->second->getGUID() == guid)
			return it->second;
	}

	return NULL;
}

Player* Game::getPlayerByGuidEx(uint32_t guid)
{
	Player* player = getPlayerByGuid(guid);
	if(player)
		return player;

	std::string name;
	if(!IOLoginData::getInstance()->getNameByGuid(guid, name))
		return NULL;

	player = new Player(name, NULL);
	if(IOLoginData::getInstance()->loadPlayer(player, name))
		return player;

#ifdef __DEBUG__
	std::clog << "[Failure - Game::getPlayerByGuidEx] Cannot load player: " << name << std::endl;
#endif
	delete player;
	return NULL;
}

ReturnValue Game::getPlayerByNameWildcard(std::string s, Player*& player)
{
	player = NULL;
	if(s.empty())
		return RET_PLAYERWITHTHISNAMEISNOTONLINE;

	char tmp = *s.rbegin();
	if(tmp != '~' && tmp != '*')
	{
		player = getPlayerByName(s);
		if(!player)
			return RET_PLAYERWITHTHISNAMEISNOTONLINE;

		return RET_NOERROR;
	}

	Player* last = NULL;
	s = s.substr(0, s.length() - 1);

	toLowerCaseString(s);
	for(AutoList<Player>::iterator it = Player::autoList.begin(); it != Player::autoList.end(); ++it)
	{
		if(it->second->isRemoved())
			continue;

		std::string name = asLowerCaseString(it->second->getName());
		if(name.substr(0, s.length()) != s)
			continue;

		if(last)
			return RET_NAMEISTOOAMBIGUOUS;

		last = it->second;
	}

	if(!last)
		return RET_PLAYERWITHTHISNAMEISNOTONLINE;

	player = last;
	return RET_NOERROR;
}

Player* Game::getPlayerByAccount(uint32_t acc)
{
	for(AutoList<Player>::iterator it = Player::autoList.begin(); it != Player::autoList.end(); ++it)
	{
		if(!it->second->isRemoved() && it->second->getAccount() == acc)
			return it->second;
	}

	return NULL;
}

PlayerVector Game::getPlayersByName(std::string s)
{
	toLowerCaseString(s);
	PlayerVector players;
	for(AutoList<Player>::iterator it = Player::autoList.begin(); it != Player::autoList.end(); ++it)
	{
		if(!it->second->isRemoved() && asLowerCaseString(it->second->getName()) == s)
			players.push_back(it->second);
	}

	return players;
}

PlayerVector Game::getPlayersByAccount(uint32_t acc)
{
	PlayerVector players;
	for(AutoList<Player>::iterator it = Player::autoList.begin(); it != Player::autoList.end(); ++it)
	{
		if(!it->second->isRemoved() && it->second->getAccount() == acc)
			players.push_back(it->second);
	}

	return players;
}

PlayerVector Game::getPlayersByIP(uint32_t ip, uint32_t mask)
{
	PlayerVector players;
	for(AutoList<Player>::iterator it = Player::autoList.begin(); it != Player::autoList.end(); ++it)
	{
		if(!it->second->isRemoved() && (it->second->getIP() & mask) == (ip & mask))
			players.push_back(it->second);
	}

	return players;
}

bool Game::internalPlaceCreature(Creature* creature, const Position& pos, bool extendedPos /*= false*/, bool forced /*= false*/)
{
	if(creature->getParent())
		return false;

	if(!map->placeCreature(pos, creature, extendedPos, forced))
		return false;

	creature->addRef();
	creature->setID();

	autoList[creature->getID()] = creature;
	creature->addList();
	return true;
}

bool Game::placeCreature(Creature* creature, const Position& pos, bool extendedPos /*= false*/, bool forced /*= false*/)
{
	if(!internalPlaceCreature(creature, pos, extendedPos, forced))
		return false;

	Player* tmpPlayer = NULL;
	if((tmpPlayer = creature->getPlayer()) && !tmpPlayer->storedConditionList.empty())
	{
		for(ConditionList::iterator it = tmpPlayer->storedConditionList.begin(); it != tmpPlayer->storedConditionList.end(); ++it)
		{
			if((*it)->getType() == CONDITION_MUTED && (*it)->getTicks() != -1 &&
				(*it)->getTicks() - ((time(NULL) - tmpPlayer->getLastLogout()) * 1000) <= 0)
				continue;

			tmpPlayer->addCondition(*it);
		}

		tmpPlayer->storedConditionList.clear();
	}

	SpectatorVec::iterator it;
	SpectatorVec list;

	getSpectators(list, creature->getPosition(), false, true);
	for(it = list.begin(); it != list.end(); ++it)
	{
		if((tmpPlayer = (*it)->getPlayer()))
			tmpPlayer->sendCreatureAppear(creature);
	}

	for(it = list.begin(); it != list.end(); ++it)
		(*it)->onCreatureAppear(creature);

	creature->setLastPosition(pos);
	creature->getParent()->postAddNotification(NULL, creature, NULL, creature->getParent()->__getIndexOfThing(creature));

	addCreatureCheck(creature);
	creature->onPlacedCreature();
	return true;
}

ReturnValue Game::placeSummon(Creature* creature, const std::string& name)
{
	Monster* monster = Monster::createMonster(name);
	if(!monster)
		return RET_NOTPOSSIBLE;

	// Place the monster
	creature->addSummon(monster);
	if(placeCreature(monster, creature->getPosition(), true))
	{
		addMagicEffect(monster->getPosition(), MAGIC_EFFECT_TELEPORT);
		return RET_NOERROR;
	}

	creature->removeSummon(monster);
	return RET_NOTENOUGHROOM;
}

bool Game::removeCreature(Creature* creature, bool isLogout /*= true*/)
{
	if(creature->isRemoved())
		return false;

	Tile* tile = creature->getTile();
	SpectatorVec list;
	getSpectators(list, tile->getPosition(), false, true);

	SpectatorVec::iterator it;
	for(it = list.begin(); it != list.end(); ++it)
		(*it)->onCreatureDisappear(creature, isLogout);

	if(tile != creature->getTile())
	{
		list.clear();
		tile = creature->getTile();
		getSpectators(list, tile->getPosition(), false, true);
	}

	Player* player = NULL;
	int32_t oldIndex = tile->__getIndexOfThing(creature);
	if(!map->removeCreature(creature))
		return false;

	//send to client
	uint32_t i = 0;
	for(it = list.begin(); it != list.end(); ++it)
	{
		if(creature != (*it))
			(*it)->updateTileCache(tile);

		if(!(player = (*it)->getPlayer()))
			continue;

		player->sendCreatureDisappear(creature, oldIndex);
		++i;
	}

	creature->getParent()->postRemoveNotification(NULL, creature, NULL, oldIndex, true);
	creature->onRemovedCreature();

	autoList.erase(creature->getID());
	freeThing(creature);

	removeCreatureCheck(creature);
	for(std::list<Creature*>::iterator it = creature->summons.begin(); it != creature->summons.end(); ++it)
		removeCreature(*it);

	return true;
}

bool Game::playerMoveThing(uint32_t playerId, const Position& fromPos,
	uint16_t spriteId, int16_t fromStackpos, const Position& toPos, uint8_t count)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	if(player->getNoMove())
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		return false;
	}

	uint8_t fromIndex = 0;
	if(fromPos.x == 0xFFFF)
	{
		if(fromPos.y & 0x40)
			fromIndex = static_cast<uint8_t>(fromPos.z);
		else
			fromIndex = static_cast<uint8_t>(fromPos.y);
	}
	else
		fromIndex = fromStackpos;

	Thing* thing = internalGetThing(player, fromPos, fromIndex, spriteId, STACKPOS_MOVE);
	Cylinder* toCylinder = internalGetCylinder(player, toPos);
	if(!thing || !toCylinder)
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		return false;
	}

	if(Creature* movingCreature = thing->getCreature())
		playerMoveCreature(playerId, movingCreature->getID(), movingCreature->getPosition(), toCylinder->getPosition(), true);
	else if(thing->getItem())
		playerMoveItem(playerId, fromPos, spriteId, fromStackpos, toPos, count);

	return true;
}

bool Game::playerMoveCreature(uint32_t playerId, uint32_t movingCreatureId,
	const Position& movingCreaturePos, const Position& toPos, bool delay)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved() || player->hasFlag(PlayerFlag_CannotMoveCreatures))
		return false;

	if(!player->canDoAction())
	{
		SchedulerTask* task = createSchedulerTask(player->getNextActionTime(),
			boost::bind(&Game::playerMoveCreature, this, playerId, movingCreatureId, movingCreaturePos, toPos, true));
		player->setNextActionTask(task);
		return false;
	}

	Creature* movingCreature = getCreatureByID(movingCreatureId);
	if(!movingCreature || movingCreature->isRemoved() || !player->canSeeCreature(movingCreature))
		return false;

	player->setNextActionTask(NULL);
	if(!Position::areInRange<1,1,0>(movingCreaturePos, player->getPosition()) && !player->hasCustomFlag(PlayerCustomFlag_CanMoveFromFar))
	{
		//need to walk to the creature first before moving it
		std::list<Direction> listDir;
		if(getPathToEx(player, movingCreaturePos, listDir, 0, 1, true, true))
		{
			Dispatcher::getInstance().addTask(createTask(boost::bind(&Game::playerAutoWalk,
				this, player->getID(), listDir)));
			SchedulerTask* task = createSchedulerTask(std::max((int32_t)SCHEDULER_MINTICKS, player->getStepDuration()),
				boost::bind(&Game::playerMoveCreature, this, playerId, movingCreatureId, movingCreaturePos, toPos, true));

			player->setNextWalkActionTask(task);
			return true;
		}

		player->sendCancelMessage(RET_THEREISNOWAY);
		return false;
	}
	else if(delay)
	{
		uint32_t delayTime = g_config.getNumber(ConfigManager::PUSH_CREATURE_DELAY);
		if(delayTime > 0)
		{
			SchedulerTask* task = createSchedulerTask(delayTime,
				boost::bind(&Game::playerMoveCreature, this, playerId, movingCreatureId, movingCreaturePos, toPos, false));
			player->setNextActionTask(task);
			return true;
		}
	}

	Tile* toTile = map->getTile(toPos);
	if(!toTile)
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		return false;
	}

	if(!player->hasFlag(PlayerFlag_CanPushAllCreatures))
	{
		if(!movingCreature->isPushable())
		{
			player->sendCancelMessage(RET_NOTMOVABLE);
			return false;
		}

		if(movingCreature->getNoMove())
		{
			player->sendCancelMessage(RET_NOTPOSSIBLE);
			return false;
		}

		if(toTile->getHeight() > 1)
		{
			player->sendCancelMessage(RET_NOTPOSSIBLE);
			return false;
		}

		if(toTile->hasProperty(BLOCKPATH))
		{
			player->sendCancelMessage(RET_NOTENOUGHROOM);
			return false;
		}
	}

	//check throw distance
	const Position& pos = movingCreature->getPosition();
	if(!player->hasCustomFlag(PlayerCustomFlag_CanThrowAnywhere) && ((std::abs(pos.x - toPos.x) > movingCreature->getThrowRange()) ||
		(std::abs(pos.y - toPos.y) > movingCreature->getThrowRange()) || (std::abs(pos.z - toPos.z) * 4 > movingCreature->getThrowRange())))
	{
		player->sendCancelMessage(RET_DESTINATIONOUTOFREACH);
		return false;
	}

	if(player != movingCreature)
	{
		if(!player->hasFlag(PlayerFlag_IgnoreProtectionZone) && (movingCreature->getZone() == ZONE_PROTECTION
			|| movingCreature->getZone() == ZONE_OPTIONAL) && !toTile->hasFlag(TILESTATE_OPTIONALZONE)
			&& !toTile->hasFlag(TILESTATE_PROTECTIONZONE))
		{
			player->sendCancelMessage(RET_ACTIONNOTPERMITTEDINANOPVPZONE);
			return false;
		}

		if(!player->hasFlag(PlayerFlag_CanPushAllCreatures))
		{
			if(toTile->getCreatureCount() && !Item::items[
				movingCreature->getTile()->ground->getID()].walkStack)
			{
				player->sendCancelMessage(RET_NOTENOUGHROOM);
				return false;
			}

			if(MagicField* field = toTile->getFieldItem())
			{
				if(field->isUnstepable() || field->isBlocking(movingCreature)
					|| !movingCreature->isImmune(field->getCombatType()))
				{
					player->sendCancelMessage(RET_NOTPOSSIBLE);
					return false;
				}
			}

			if(player->isProtected())
			{
				Player* movingPlayer = movingCreature->getPlayer();
				if(movingPlayer && !movingPlayer->isProtected())
				{
					player->sendCancelMessage(RET_NOTMOVABLE);
					return false;
				}
			}
		}
	}

	bool deny = false;
	CreatureEventList pushEvents = player->getCreatureEvents(CREATURE_EVENT_PUSH);
	for(CreatureEventList::iterator it = pushEvents.begin(); it != pushEvents.end(); ++it)
	{
		if(!(*it)->executePush(player, movingCreature, toTile) && !deny)
			deny = true;
	}

	if(deny)
		return false;

	ReturnValue ret = internalMoveCreature(player, movingCreature, movingCreature->getTile(), toTile);
	if(ret != RET_NOERROR)
	{
		if(!player->hasCustomFlag(PlayerCustomFlag_CanMoveAnywhere))
		{
			player->sendCancelMessage(ret);
			return false;
		}

		if(!toTile->ground)
		{
			player->sendCancelMessage(RET_NOTPOSSIBLE);
			return false;
		}

		internalTeleport(movingCreature, toTile->getPosition(), false);
	}
	else if(Player* movingPlayer = movingCreature->getPlayer())
	{
		uint64_t delay = OTSYS_TIME() + movingPlayer->getStepDuration();
		if(delay > movingPlayer->getNextActionTime(false))
			movingPlayer->setNextAction(delay);
	}

	return true;
}

ReturnValue Game::internalMoveCreature(Creature* creature, Direction direction, uint32_t flags/* = 0*/)
{
	const Position& currentPos = creature->getPosition();
	Cylinder* fromTile = creature->getTile();
	Cylinder* toTile = NULL;

	Position destPos = getNextPosition(direction, currentPos);
	if(direction < SOUTHWEST && creature->getPlayer())
	{
		Tile* tmpTile = NULL;
		if(currentPos.z != 8 && creature->getTile()->hasHeight(3)) //try go up
		{
			if((!(tmpTile = map->getTile(Position(currentPos.x, currentPos.y, currentPos.z - 1)))
				|| (!tmpTile->ground && !tmpTile->hasProperty(BLOCKSOLID))) &&
				(tmpTile = map->getTile(Position(destPos.x, destPos.y, destPos.z - 1)))
				&& tmpTile->ground && !tmpTile->hasProperty(BLOCKSOLID) && !tmpTile->hasProperty(FLOORCHANGEDOWN))
			{
				flags = flags | FLAG_IGNOREBLOCKITEM | FLAG_IGNOREBLOCKCREATURE;
				destPos.z--;
			}
		}
		else if(currentPos.z != 7 && (!(tmpTile = map->getTile(destPos)) || (!tmpTile->ground &&
			!tmpTile->hasProperty(BLOCKSOLID))) && (tmpTile = map->getTile(Position(
			destPos.x, destPos.y, destPos.z + 1))) && tmpTile->hasHeight(3)) //try go down
		{
			flags = flags | FLAG_IGNOREBLOCKITEM | FLAG_IGNOREBLOCKCREATURE;
			destPos.z++;
		}
	}

	ReturnValue ret = RET_NOTPOSSIBLE;
	if((toTile = map->getTile(destPos)))
	{
		if(g_config.getBool(ConfigManager::TILE_HEIGHT_BLOCK))
		{
			if(currentPos.z > destPos.z && toTile->getTile()->getHeight() > 1);
			else if((((int32_t)(toTile->getTile()->getHeight() - fromTile->getTile()->getHeight()) < 2)) || (fromTile->getTile()->hasHeight(3) && (currentPos.z == destPos.z)) || ((currentPos.z < destPos.z) && (toTile->getTile()->hasHeight(3) && (fromTile->getTile()->getHeight() < 2))))
				ret = internalMoveCreature(NULL, creature, fromTile, toTile, flags);
		}
		else
			ret = internalMoveCreature(NULL, creature, fromTile, toTile, flags);
	}

	if(ret == RET_NOERROR)
		return RET_NOERROR;

	Player* player = creature->getPlayer();
	if(!player)
		return ret;

	player->sendCancelMessage(ret);
	player->sendCancelWalk();
	return ret;
}

ReturnValue Game::internalMoveCreature(Creature* actor, Creature* creature, Cylinder* fromCylinder,
	Cylinder* toCylinder, uint32_t flags/* = 0*/, bool forceTeleport/* = false*/)
{
	//check if we can move the creature to the destination
	ReturnValue ret = toCylinder->__queryAdd(0, creature, 1, flags, actor);
	if(ret != RET_NOERROR)
		return ret;

	fromCylinder->getTile()->moveCreature(actor, creature, toCylinder, forceTeleport);
	if(creature->getParent() != toCylinder)
		return RET_NOERROR;

	Item* toItem = NULL;
	Cylinder* subCylinder = NULL;

	int32_t n = 0, tmp = 0;
	while((subCylinder = toCylinder->__queryDestination(tmp, creature, &toItem, flags)) != toCylinder)
	{
		internalCreatureTurn(creature, getDirectionTo(toCylinder->getTile()->getPosition(), subCylinder->getTile()->getPosition(), false));
		toCylinder->getTile()->moveCreature(actor, creature, subCylinder);
		if(creature->getParent() != subCylinder) //could happen if a script move the creature
			break;

		toCylinder = subCylinder;
		flags = 0;
		if(++n >= MAP_MAX_LAYERS) //to prevent infinite loop
			break;
	}

	return RET_NOERROR;
}

bool Game::playerMoveItem(uint32_t playerId, const Position& fromPos,
	uint16_t spriteId, int16_t fromStackpos, const Position& toPos, uint8_t count)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved() || player->hasFlag(PlayerFlag_CannotMoveItems))
		return false;

	if(!player->canDoAction())
	{
		SchedulerTask* task = createSchedulerTask(player->getNextActionTime(),
			boost::bind(&Game::playerMoveItem, this, playerId, fromPos, spriteId, fromStackpos, toPos, count));
		player->setNextActionTask(task);
		return false;
	}

	player->setNextActionTask(NULL);
	Cylinder* fromCylinder = internalGetCylinder(player, fromPos);

	uint8_t fromIndex = 0;
	if(fromPos.x == 0xFFFF)
	{
		if(fromPos.y & 0x40)
			fromIndex = static_cast<uint8_t>(fromPos.z);
		else
			fromIndex = static_cast<uint8_t>(fromPos.y);
	}
	else
		fromIndex = fromStackpos;

	Thing* thing = internalGetThing(player, fromPos, fromIndex, spriteId, STACKPOS_MOVE);
	if(!thing || !thing->getItem())
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		return false;
	}

	Item* item = thing->getItem();
	Cylinder* toCylinder = internalGetCylinder(player, toPos);

	uint8_t toIndex = 0;
	if(toPos.x == 0xFFFF)
	{
		if(toPos.y & 0x40)
			toIndex = static_cast<uint8_t>(toPos.z);
		else
			toIndex = static_cast<uint8_t>(toPos.y);
	}

	if(!fromCylinder || !toCylinder || !item || item->getClientID() != spriteId)
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		return false;
	}

	if(!player->hasCustomFlag(PlayerCustomFlag_CanPushAllItems) && (!item->isPushable() || (item->isLoadedFromMap() &&
		(item->getUniqueId() || (item->getActionId() && item->getContainer())))))
	{
		player->sendCancelMessage(RET_NOTMOVABLE);
		return false;
	}

	const Position &mapToPos = toCylinder->getTile()->getPosition(), &playerPos = player->getPosition(),
		&mapFromPos = fromCylinder->getTile()->getPosition();
	if(playerPos.z > mapFromPos.z && !player->hasCustomFlag(PlayerCustomFlag_CanThrowAnywhere))
	{
		player->sendCancelMessage(RET_FIRSTGOUPSTAIRS);
		return false;
	}

	if(playerPos.z < mapFromPos.z && !player->hasCustomFlag(PlayerCustomFlag_CanThrowAnywhere))
	{
		player->sendCancelMessage(RET_FIRSTGODOWNSTAIRS);
		return false;
	}

	if(!Position::areInRange<1,1,0>(playerPos, mapFromPos) && !player->hasCustomFlag(PlayerCustomFlag_CanMoveFromFar))
	{
		//need to walk to the item first before using it
		std::list<Direction> listDir;
		if(getPathToEx(player, item->getPosition(), listDir, 0, 1, true, true))
		{
			Dispatcher::getInstance().addTask(createTask(boost::bind(&Game::playerAutoWalk,
				this, player->getID(), listDir)));
			SchedulerTask* task = createSchedulerTask(std::max((int32_t)SCHEDULER_MINTICKS, player->getStepDuration()),
				boost::bind(&Game::playerMoveItem, this, playerId, fromPos, spriteId, fromStackpos, toPos, count));

			player->setNextWalkActionTask(task);
			return true;
		}

		player->sendCancelMessage(RET_THEREISNOWAY);
		return false;
	}

	//hangable item specific code
	if(item->isHangable() && toCylinder->getTile()->hasProperty(SUPPORTHANGABLE))
	{
		//destination supports hangable objects so need to move there first
		if(toCylinder->getTile()->hasProperty(ISVERTICAL))
		{
			if(player->getPosition().x + 1 == mapToPos.x)
			{
				player->sendCancelMessage(RET_NOTPOSSIBLE);
				return false;
			}
		}
		else if(toCylinder->getTile()->hasProperty(ISHORIZONTAL))
		{
			if(player->getPosition().y + 1 == mapToPos.y)
			{
				player->sendCancelMessage(RET_NOTPOSSIBLE);
				return false;
			}
		}

		if(!Position::areInRange<1,1,0>(playerPos, mapToPos) && !player->hasCustomFlag(PlayerCustomFlag_CanMoveFromFar))
		{
			Position walkPos = mapToPos;
			if(toCylinder->getTile()->hasProperty(ISVERTICAL))
				walkPos.x -= -1;

			if(toCylinder->getTile()->hasProperty(ISHORIZONTAL))
				walkPos.y -= -1;

			Position itemPos = fromPos;
			int16_t itemStackpos = fromStackpos;
			if(fromPos.x != 0xFFFF && Position::areInRange<1,1,0>(mapFromPos, player->getPosition())
				&& !Position::areInRange<1,1,0>(mapFromPos, walkPos))
			{
				//need to pickup the item first
				Item* moveItem = NULL;
				ReturnValue ret = internalMoveItem(player, fromCylinder, player, INDEX_WHEREEVER, item, count, &moveItem);
				if(ret != RET_NOERROR)
				{
					player->sendCancelMessage(ret);
					return false;
				}

				//changing the position since its now in the inventory of the player
				internalGetPosition(moveItem, itemPos, itemStackpos);
			}

			std::list<Direction> listDir;
			if(map->getPathTo(player, walkPos, listDir))
			{
				Dispatcher::getInstance().addTask(createTask(boost::bind(&Game::playerAutoWalk,
					this, player->getID(), listDir)));
				SchedulerTask* task = createSchedulerTask(std::max((int32_t)SCHEDULER_MINTICKS, player->getStepDuration()),
					boost::bind(&Game::playerMoveItem, this, playerId, itemPos, spriteId, itemStackpos, toPos, count));

				player->setNextWalkActionTask(task);
				return true;
			}

			player->sendCancelMessage(RET_THEREISNOWAY);
			return false;
		}
	}

	if(!player->hasCustomFlag(PlayerCustomFlag_CanThrowAnywhere))
	{
		if((std::abs(playerPos.x - mapToPos.x) > item->getThrowRange()) ||
			(std::abs(playerPos.y - mapToPos.y) > item->getThrowRange()) ||
			(std::abs(mapFromPos.z - mapToPos.z) * 4 > item->getThrowRange()))
		{
			player->sendCancelMessage(RET_DESTINATIONOUTOFREACH);
			return false;
		}

		if(!canThrowObjectTo(mapFromPos, mapToPos))
		{
			player->sendCancelMessage(RET_CANNOTTHROW);
			return false;
		}
	}

	bool deny = false;
	CreatureEventList throwEvents = player->getCreatureEvents(CREATURE_EVENT_THROW);
	for(CreatureEventList::iterator it = throwEvents.begin(); it != throwEvents.end(); ++it)
	{
		if(!(*it)->executeThrow(player, item, fromPos, toPos) && !deny)
			deny = true;
	}

	if(deny)
		return false;

	ReturnValue ret = internalMoveItem(player, fromCylinder, toCylinder, toIndex, item, count, NULL);
	if(ret != RET_NOERROR)
	{
		player->sendCancelMessage(ret);
		return false;
	}

	player->setNextAction(OTSYS_TIME() + g_config.getNumber(ConfigManager::ACTIONS_DELAY_INTERVAL) - 10);
	return true;
}

ReturnValue Game::internalMoveItem(Creature* actor, Cylinder* fromCylinder, Cylinder* toCylinder,
	int32_t index, Item* item, uint32_t count, Item** _moveItem, uint32_t flags /*= 0*/)
{
	if(!toCylinder)
		return RET_NOTPOSSIBLE;

	if(!item->getParent())
	{
		assert(fromCylinder == item->getParent());
		return internalAddItem(actor, toCylinder, item, INDEX_WHEREEVER, FLAG_NOLIMIT);
	}

	Item* toItem = NULL;
	Cylinder* subCylinder = NULL;

	int32_t floor = 0;
	while((subCylinder = toCylinder->__queryDestination(index, item, &toItem, flags)) != toCylinder)
	{
		flags = 0;
		toCylinder = subCylinder;
		//to prevent infinite loop
		if(++floor >= MAP_MAX_LAYERS)
			break;
	}

	//destination is the same as the source?
	if(item == toItem)
		return RET_NOERROR; //silently ignore move

	//check if we can add this item
	ReturnValue ret = toCylinder->__queryAdd(index, item, count, flags, actor);
	if(ret == RET_NEEDEXCHANGE)
	{
		//check if we can add it to source cylinder
		int32_t fromIndex = fromCylinder->__getIndexOfThing(item);
		if((ret = fromCylinder->__queryAdd(fromIndex, toItem, toItem->getItemCount(), 0, actor)) == RET_NOERROR)
		{
			//check how much we can move
			uint32_t maxExchangeQueryCount = 0;
			ReturnValue retExchangeMaxCount = fromCylinder->__queryMaxCount(INDEX_WHEREEVER, toItem, toItem->getItemCount(), maxExchangeQueryCount, 0);
			if(retExchangeMaxCount != RET_NOERROR && !maxExchangeQueryCount)
				return retExchangeMaxCount;

			if((toCylinder->__queryRemove(toItem, toItem->getItemCount(), flags, actor) == RET_NOERROR) && ret == RET_NOERROR)
			{
				int32_t oldToItemIndex = toCylinder->__getIndexOfThing(toItem);
				toCylinder->__removeThing(toItem, toItem->getItemCount());

				fromCylinder->__addThing(actor, toItem);
				if(oldToItemIndex != -1)
					toCylinder->postRemoveNotification(actor, toItem, fromCylinder, oldToItemIndex, true);

				int32_t newToItemIndex = fromCylinder->__getIndexOfThing(toItem);
				if(newToItemIndex != -1)
					fromCylinder->postAddNotification(actor, toItem, toCylinder, newToItemIndex);

				ret = toCylinder->__queryAdd(index, item, count, flags, actor);
				toItem = NULL;
			}
		}
	}

	if(ret != RET_NOERROR)
		return ret;

	//check how much we can move
	uint32_t maxQueryCount = 0;
	ReturnValue retMaxCount = toCylinder->__queryMaxCount(index, item, count, maxQueryCount, flags);
	if(retMaxCount != RET_NOERROR && !maxQueryCount)
		return ret;

	uint32_t m = maxQueryCount;
	if(item->isStackable())
		m = std::min((uint32_t)count, m);

	Item* moveItem = item;
	//check if we can remove this item
	if((ret = fromCylinder->__queryRemove(item, m, flags, actor)) != RET_NOERROR)
		return ret;

	//remove the item
	int32_t itemIndex = fromCylinder->__getIndexOfThing(item);
	fromCylinder->__removeThing(item, m);
	bool isCompleteRemoval = item->isRemoved();

	Item* updateItem = NULL;
	if(item->isStackable())
	{
		uint8_t n = 0;
		if(toItem && toItem->getID() == item->getID())
		{
			n = std::min((uint32_t)100 - toItem->getItemCount(), m);
			toCylinder->__updateThing(toItem, toItem->getID(), toItem->getItemCount() + n);
			updateItem = toItem;
		}

		if(m - n > 0)
			moveItem = Item::CreateItem(item->getID(), m - n);
		else
			moveItem = NULL;

		if(item->isRemoved())
			freeThing(item);
	}

	if(moveItem)
		toCylinder->__addThing(actor, index, moveItem);

	if(itemIndex != -1)
		fromCylinder->postRemoveNotification(actor, item, toCylinder, itemIndex, isCompleteRemoval);

	if(moveItem)
	{
		int32_t moveItemIndex = toCylinder->__getIndexOfThing(moveItem);
		if(moveItemIndex != -1)
			toCylinder->postAddNotification(actor, moveItem, fromCylinder, moveItemIndex);
	}

	if(updateItem)
	{
		int32_t updateItemIndex = toCylinder->__getIndexOfThing(updateItem);
		if(updateItemIndex != -1)
			toCylinder->postAddNotification(actor, updateItem, fromCylinder, updateItemIndex);
	}

	if(_moveItem)
	{
		if(moveItem)
			*_moveItem = moveItem;
		else
			*_moveItem = item;
	}

	//we could not move all, inform the player
	if(item->isStackable() && maxQueryCount < count)
		return retMaxCount;

	return ret;
}

ReturnValue Game::internalAddItem(Creature* actor, Cylinder* toCylinder, Item* item, int32_t index /*= INDEX_WHEREEVER*/,
	uint32_t flags/* = 0*/, bool test/* = false*/)
{
	uint32_t remainderCount = 0;
	return internalAddItem(actor, toCylinder, item, index, flags, test, remainderCount);
}

ReturnValue Game::internalAddItem(Creature* actor, Cylinder* toCylinder, Item* item, int32_t index,
	uint32_t flags, bool test, uint32_t& remainderCount)
{
	Item* stackItem = NULL;
	return internalAddItem(actor, toCylinder, item, index, flags, test, remainderCount, &stackItem);
}

ReturnValue Game::internalAddItem(Creature* actor, Cylinder* toCylinder, Item* item, int32_t index,
	uint32_t flags, bool test, uint32_t& remainderCount, Item** stackItem)
{
	*stackItem = NULL;
	remainderCount = 0;
	if(!toCylinder || !item)
		return RET_NOTPOSSIBLE;

	Cylinder* destCylinder = toCylinder;
	toCylinder = toCylinder->__queryDestination(index, item, stackItem, flags);

	//check if we can add this item
	ReturnValue ret = toCylinder->__queryAdd(index, item, item->getItemCount(), flags, actor);
	if(ret != RET_NOERROR)
		return ret;

	uint32_t maxQueryCount = 0;
	ret = destCylinder->__queryMaxCount(INDEX_WHEREEVER, item, item->getItemCount(), maxQueryCount, flags);
	if(ret != RET_NOERROR)
		return ret;

	if(test)
		return RET_NOERROR;

	Item* toItem = *stackItem;
	if(item->isStackable() && toItem)
	{
		uint32_t m = std::min((uint32_t)item->getItemCount(), maxQueryCount), n = 0;
		if(toItem->getID() == item->getID())
		{
			n = std::min((uint32_t)100 - toItem->getItemCount(), m);
			toCylinder->__updateThing(toItem, toItem->getID(), toItem->getItemCount() + n);
		}

		uint32_t count = m - n;
		if(count > 0)
		{
			if(item->getItemCount() != count)
			{
				Item* remainderItem = Item::CreateItem(item->getID(), count);
				if((ret = internalAddItem(NULL, toCylinder, remainderItem, INDEX_WHEREEVER, flags, false)) == RET_NOERROR)
				{
					if(item->getParent() != VirtualCylinder::virtualCylinder)
					{
						item->onRemoved();
						freeThing(item);
					}

					return RET_NOERROR;
				}

				delete remainderItem;
				remainderCount = count;
				return ret;
			}
		}
		else
		{
			if(item->getParent() != VirtualCylinder::virtualCylinder)
			{
				item->onRemoved();
				freeThing(item);
			}

			return RET_NOERROR;
		}
	}

	toCylinder->__addThing(NULL, index, item);
	int32_t itemIndex = toCylinder->__getIndexOfThing(item);
	if(itemIndex != -1)
		toCylinder->postAddNotification(actor, item, NULL, itemIndex);

	return RET_NOERROR;
}

ReturnValue Game::internalRemoveItem(Creature* actor, Item* item, int32_t count/* = -1*/, bool test/* = false*/, uint32_t flags/* = 0*/)
{
	Cylinder* cylinder = item->getParent();
	if(!cylinder)
		return RET_NOTPOSSIBLE;

	if(count == -1)
		count = item->getItemCount();

	//check if we can remove this item
	ReturnValue ret = cylinder->__queryRemove(item, count, flags | FLAG_IGNORENOTMOVABLE, actor);
	if(ret != RET_NOERROR)
		return ret;

	if(!item->canRemove())
		return RET_NOTPOSSIBLE;

	if(!test)
	{
		//remove the item
		int32_t index = cylinder->__getIndexOfThing(item);
		cylinder->__removeThing(item, count);

		cylinder->postRemoveNotification(actor, item, NULL, index, item->isRemoved());
		if(item->isRemoved())
			freeThing(item);
	}

	item->onRemoved();
	return RET_NOERROR;
}

ReturnValue Game::internalPlayerAddItem(Creature* actor, Player* player, Item* item,
	bool dropOnMap/* = true*/, slots_t slot/* = SLOT_WHEREEVER*/)
{
	Item* toItem = NULL;
	return internalPlayerAddItem(actor, player, item, dropOnMap, slot, &toItem);
}

ReturnValue Game::internalPlayerAddItem(Creature* actor, Player* player, Item* item,
	bool dropOnMap, slots_t slot, Item** toItem)
{
	uint32_t remainderCount = 0, count = item->getItemCount();
	ReturnValue ret = internalAddItem(actor, player, item, (int32_t)slot, 0, false, remainderCount, toItem);
	if(ret == RET_NOERROR)
		return RET_NOERROR;

	if(dropOnMap)
	{
		if(!remainderCount)
			return internalAddItem(actor, player->getTile(), item, (int32_t)slot, FLAG_NOLIMIT);

		Item* remainderItem = Item::CreateItem(item->getID(), remainderCount);
		if(internalAddItem(actor, player->getTile(), remainderItem, INDEX_WHEREEVER, FLAG_NOLIMIT) == RET_NOERROR)
			return RET_NOERROR;

		delete remainderItem;
	}

	if(remainderCount && toItem)
		transformItem(*toItem, (*toItem)->getID(), ((*toItem)->getItemCount() - (count - remainderCount)));

	return ret;
}

Item* Game::findItemOfType(Cylinder* cylinder, uint16_t itemId,
	bool depthSearch /*= true*/, int32_t subType /*= -1*/)
{
	if(!cylinder)
		return NULL;

	std::list<Container*> listContainer;
	Container* tmpContainer = NULL;
	Item* item = NULL;

	Thing* thing = NULL;
	for(int32_t i = cylinder->__getFirstIndex(); i < cylinder->__getLastIndex();)
	{
		if((thing = cylinder->__getThing(i)) && (item = thing->getItem()))
		{
			if(item->getID() == itemId && (subType == -1 || subType == item->getSubType()))
				return item;

			++i;
			if(depthSearch && (tmpContainer = item->getContainer()))
				listContainer.push_back(tmpContainer);
		}
		else
			++i;
	}

	Container* container = NULL;
	while(listContainer.size() > 0)
	{
		container = listContainer.front();
		listContainer.pop_front();
		for(int32_t i = 0; i < (int32_t)container->size(); )
		{
			if((item = container->getItem(i)))
			{
				if(item->getID() == itemId && (subType == -1 || subType == item->getSubType()))
					return item;

				++i;
				if((tmpContainer = item->getContainer()))
					listContainer.push_back(tmpContainer);
			}
			else
				++i;
		}
	}

	return NULL;
}

bool Game::removeItemOfType(Cylinder* cylinder, uint16_t itemId, int32_t count, int32_t subType/* = -1*/, bool onlyContainers/* = false*/)
{
	if(!cylinder || ((int32_t)cylinder->__getItemTypeCount(itemId, subType) < count))
		return false;

	if(count <= 0)
		return true;

	std::list<Container*> listContainer;
	Container* tmpContainer = NULL;
	Item* item = NULL;

	Thing* thing = NULL;
	for(int32_t i = cylinder->__getFirstIndex(); i < cylinder->__getLastIndex() && count > 0; )
	{
		if((thing = cylinder->__getThing(i)) && (item = thing->getItem()))
		{
			if(!onlyContainers && item->getID() == itemId)
			{
				if(item->isStackable())
				{
					if(item->getItemCount() > count)
					{
						internalRemoveItem(NULL, item, count);
						count = 0;
					}
					else
					{
						count -= item->getItemCount();
						internalRemoveItem(NULL, item);
					}
				}
				else if(subType == -1 || subType == item->getSubType())
				{
					--count;
					internalRemoveItem(NULL, item);
				}
				else
					++i;
			}
			else
			{
				++i;
				if((tmpContainer = item->getContainer()))
					listContainer.push_back(tmpContainer);
			}
		}
		else
			++i;
	}

	Container* container = NULL;
	while(listContainer.size() > 0 && count > 0)
	{
		container = listContainer.front();
		listContainer.pop_front();
		for(int32_t i = 0; i < (int32_t)container->size() && count > 0; )
		{
			if((item = container->getItem(i)))
			{
				if(item->getID() == itemId)
				{
					if(item->isStackable())
					{
						if(item->getItemCount() > count)
						{
							internalRemoveItem(NULL, item, count);
							count = 0;
						}
						else
						{
							count-= item->getItemCount();
							internalRemoveItem(NULL, item);
						}
					}
					else if(subType == -1 || subType == item->getSubType())
					{
						--count;
						internalRemoveItem(NULL, item);
					}
					else
						++i;
				}
				else
				{
					++i;
					if((tmpContainer = item->getContainer()))
						listContainer.push_back(tmpContainer);
				}
			}
			else
				++i;
		}
	}

	return !count;
}

uint64_t Game::getMoney(const Cylinder* cylinder)
{
	if(!cylinder)
		return 0;

	std::list<Container*> listContainer;
	Container* tmpContainer = NULL;

	Thing* thing = NULL;
	Item* item = NULL;

	uint64_t moneyCount = 0;
	for(int32_t i = cylinder->__getFirstIndex(); i < cylinder->__getLastIndex(); ++i)
	{
		if(!(thing = cylinder->__getThing(i)) || !(item = thing->getItem()))
			continue;

		if((tmpContainer = item->getContainer()))
			listContainer.push_back(tmpContainer);
		else if(item->getWorth())
			moneyCount += item->getWorth();
	}

	Container* container = NULL;
	while(listContainer.size() > 0)
	{
		container = listContainer.front();
		listContainer.pop_front();
		for(ItemList::const_iterator it = container->getItems(); it != container->getEnd(); ++it)
		{
			item = *it;
			if((tmpContainer = item->getContainer()))
				listContainer.push_back(tmpContainer);
			else if(item->getWorth())
				moneyCount += item->getWorth();
		}
	}

	return moneyCount;
}

bool Game::removeMoney(Cylinder* cylinder, int64_t money, uint32_t flags /*= 0*/)
{
	if(!cylinder)
		return false;

	if(money <= 0)
		return true;

	typedef std::multimap<int32_t, Item*, std::less<int32_t> > MoneyMultiMap;
	MoneyMultiMap moneyMap;

	std::list<Container*> listContainer;
	Container* tmpContainer = NULL;

	Thing* thing = NULL;
	Item* item = NULL;

	int64_t moneyCount = 0;
	for(int32_t i = cylinder->__getFirstIndex(); i < cylinder->__getLastIndex(); ++i)
	{
		if(!(thing = cylinder->__getThing(i)) || !(item = thing->getItem()))
			continue;

		if((tmpContainer = item->getContainer()))
			listContainer.push_back(tmpContainer);
		else if(item->getWorth())
		{
			moneyCount += item->getWorth();
			moneyMap.insert(std::make_pair(item->getWorth(), item));
		}
	}

	while(listContainer.size() > 0)
	{
		Container* container = listContainer.front();
		listContainer.pop_front();
		for(int32_t i = 0; i < (int32_t)container->size(); ++i)
		{
			Item* item = container->getItem(i);
			if((tmpContainer = item->getContainer()))
				listContainer.push_back(tmpContainer);
			else if(item->getWorth())
			{
				moneyCount += item->getWorth();
				moneyMap.insert(std::make_pair(item->getWorth(), item));
			}
		}
	}

	// Not enough money
	if(moneyCount < money)
		return false;

	for(MoneyMultiMap::iterator mit = moneyMap.begin(); mit != moneyMap.end() && money > 0; ++mit)
	{
		if(!(item = mit->second))
			continue;

		internalRemoveItem(NULL, item);
		if(mit->first > money)
		{
			// Remove a monetary value from an item
			addMoney(cylinder, (int64_t)(item->getWorth() - money), flags);
			money = 0;
		}
		else
			money -= mit->first;

		mit->second = NULL;
	}

	moneyMap.clear();
	return !money;
}

void Game::addMoney(Cylinder* cylinder, int64_t money, uint32_t flags /*= 0*/)
{
	IntegerMap moneyMap = Item::items.getMoneyMap();
	for(IntegerMap::reverse_iterator it = moneyMap.rbegin(); it != moneyMap.rend(); ++it)
	{
		int64_t tmp = money / it->first;
		money -= tmp * it->first;
		if(!tmp)
			continue;

		do
		{
			uint32_t remainderCount = 0;
			Item* item = Item::CreateItem(it->second, std::min<uint16_t>(100, tmp));
			if(internalAddItem(NULL, cylinder, item, INDEX_WHEREEVER, flags, false, remainderCount) != RET_NOERROR)
			{
				if(remainderCount)
				{
					delete item;
					item = Item::CreateItem(it->second, remainderCount);
				}

				if(internalAddItem(NULL, cylinder->getTile(), item, INDEX_WHEREEVER, FLAG_NOLIMIT) != RET_NOERROR)
					delete item;
			}

			tmp -= std::min((int64_t)100, tmp);
		}
		while(tmp > 0);
	}
}

Item* Game::transformItem(Item* item, uint16_t newId, int32_t newCount/* = -1*/)
{
	if(item->getID() == newId && (newCount == -1 || (newCount == item->getSubType() && newCount)))
		return item;

	Cylinder* cylinder = item->getParent();
	if(!cylinder)
		return NULL;

	int32_t itemIndex = cylinder->__getIndexOfThing(item);
	if(itemIndex == -1)
	{
#ifdef __DEBUG__
		std::clog << "Error: transformItem, itemIndex == -1" << std::endl;
#endif
		return item;
	}

	if(!item->canTransform())
		return item;

	const ItemType &curType = Item::items[item->getID()], &newType = Item::items[newId];
	if(curType.alwaysOnTop != newType.alwaysOnTop)
	{
		// This only occurs when you transform items on tiles from a downItem to a topItem (or vice versa)
		// Remove the old, and add the new
		ReturnValue ret = internalRemoveItem(NULL, item);
		if(ret != RET_NOERROR)
			return item;

		Item* newItem = NULL;
		if(newCount == -1)
			newItem = Item::CreateItem(newId);
		else
			newItem = Item::CreateItem(newId, newCount);

		if(!newItem)
			return NULL;

		newItem->copyAttributes(item);
		if(internalAddItem(NULL, cylinder, newItem, INDEX_WHEREEVER, FLAG_NOLIMIT) != RET_NOERROR)
		{
			delete newItem;
			return NULL;
		}

		newItem->makeUnique(item);
		return newItem;
	}

	if(curType.type == newType.type)
	{
		//Both items has the same type so we can safely change id/subtype
		if(!newCount && (item->isStackable() || item->hasCharges()))
		{
			if(!item->isStackable() && (!item->getDefaultDuration() || item->getDuration() <= 0))
			{
				int16_t tmp = newId;
				if(curType.id == newId)
					tmp = curType.decayTo;

				if(tmp != -1)
					return transformItem(item, tmp);
			}

			internalRemoveItem(NULL, item);
			return NULL;
		}

		cylinder->postRemoveNotification(NULL, item, cylinder, itemIndex, false);
		uint16_t tmp = item->getID();
		if(curType.id != newId)
		{
			tmp = newId;
			if(newType.group != curType.group)
				item->setDefaultSubtype();

			if(curType.hasSubType() && !newType.hasSubType())
			{
				item->resetFluidType();
				item->resetCharges();
			}
		}

		int32_t count = item->getSubType();
		if(newCount != -1 && newType.hasSubType())
			count = newCount;

		cylinder->__updateThing(item, tmp, count);
		cylinder->postAddNotification(NULL, item, cylinder, itemIndex);
		return item;
	}

	//Replacing the the old item with the new while maintaining the old position
	Item* newItem = NULL;
	if(newCount == -1)
		newItem = Item::CreateItem(newId);
	else
		newItem = Item::CreateItem(newId, newCount);

	if(!newItem)
		return NULL;

	newItem->copyAttributes(item);
	newItem->makeUnique(item);
	cylinder->__replaceThing(itemIndex, newItem);

	cylinder->postAddNotification(NULL, newItem, cylinder, itemIndex);
	item->setParent(NULL);
	cylinder->postRemoveNotification(NULL, item, cylinder, itemIndex, true);

	freeThing(item);
	return newItem;
}

ReturnValue Game::internalTeleport(Thing* thing, const Position& newPos, bool forceTeleport, uint32_t flags/* = 0*/, bool fullTeleport/* = true*/)
{
	if(newPos == thing->getPosition())
		return RET_NOERROR;

	if(thing->isRemoved())
		return RET_NOTPOSSIBLE;

	if(Tile* toTile = map->getTile(newPos))
	{
		if(Creature* creature = thing->getCreature())
		{
			if(fullTeleport)
				return internalMoveCreature(NULL, creature, creature->getParent(), toTile, flags, forceTeleport);

			creature->getTile()->moveCreature(NULL, creature, toTile, forceTeleport);
			return RET_NOERROR;
		}

		if(Item* item = thing->getItem())
			return internalMoveItem(NULL, item->getParent(), toTile, INDEX_WHEREEVER, item, item->getItemCount(), NULL, flags);
	}

	return RET_NOTPOSSIBLE;
}

bool Game::playerMove(uint32_t playerId, Direction dir)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	player->setIdleTime(0);
	if(player->getNoMove())
	{
		player->sendCancelWalk();
		return false;
	}

	std::list<Direction> dirs;
	dirs.push_back(dir);

	player->setNextWalkActionTask(NULL);
	return player->startAutoWalk(dirs);
}

bool Game::playerBroadcastMessage(Player* player, MessageClasses type, const std::string& text, uint32_t statementId)
{
	if(!player->hasFlag(PlayerFlag_CanBroadcast) || !((type >= MSG_SPEAK_FIRST && type <= MSG_SPEAK_LAST) ||
			(type >= MSG_SPEAK_MONSTER_FIRST && type <= MSG_SPEAK_MONSTER_LAST)))
		return false;

	Logger::getInstance()->eFile("talkactions/" + player->getName() + ".log", "#b " + text, true);
	for(AutoList<Player>::iterator it = Player::autoList.begin(); it != Player::autoList.end(); ++it)
		it->second->sendCreatureSay(player, type, text, NULL, statementId);

	std::clog << "> " << player->getName() << " broadcasted: \"" << text << "\"." << std::endl;
	return true;
}

bool Game::playerCreatePrivateChannel(uint32_t playerId)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved() || !player->isPremium())
		return false;

	ChatChannel* channel = g_chat.createChannel(player, 0xFFFF);
	if(!channel || !channel->addUser(player))
		return false;

	player->sendCreatePrivateChannel(channel->getId(), channel->getName());
	return true;
}

bool Game::playerChannelInvite(uint32_t playerId, const std::string& name)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	PrivateChatChannel* channel = g_chat.getPrivateChannel(player);
	if(!channel)
		return false;

	Player* invitePlayer = getPlayerByName(name);
	if(!invitePlayer)
		return false;

	channel->invitePlayer(player, invitePlayer);
	return true;
}

bool Game::playerChannelExclude(uint32_t playerId, const std::string& name)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	PrivateChatChannel* channel = g_chat.getPrivateChannel(player);
	if(!channel)
		return false;

	Player* excludePlayer = getPlayerByName(name);
	if(!excludePlayer)
		return false;

	channel->excludePlayer(player, excludePlayer);
	return true;
}

bool Game::playerRequestChannels(uint32_t playerId)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	player->sendChannelsDialog(g_chat.getChannelList(player));
	player->setSentChat(true);
	return true;
}

bool Game::playerOpenChannel(uint32_t playerId, uint16_t channelId)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	bool deny = false;
	CreatureEventList openEvents = player->getCreatureEvents(CREATURE_EVENT_CHANNEL_REQUEST);
	for(CreatureEventList::iterator it = openEvents.begin(); it != openEvents.end(); ++it)
	{
		if(!(*it)->executeChannelRequest(player, asString(channelId), false, !player->hasSentChat()) && !deny)
			deny = true;
	}

	player->setSentChat(false);
	if(deny)
		return false;

	ChatChannel* channel = g_chat.addUserToChannel(player, channelId);
	if(!channel)
	{
		#ifdef __DEBUG_CHAT__
		std::clog << "Game::playerOpenChannel - failed adding user to channel." << std::endl;
		#endif
		return false;
	}

	if(channel->getId() != CHANNEL_RVR)
		player->sendChannel(channel->getId(), channel->getName());
	else
		player->sendRuleViolationsChannel(channel->getId());

	return true;
}

bool Game::playerCloseChannel(uint32_t playerId, uint16_t channelId)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	g_chat.removeUserFromChannel(player, channelId);
	return true;
}

bool Game::playerProcessRuleViolation(uint32_t playerId, const std::string& name)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	if(!player->hasFlag(PlayerFlag_CanAnswerRuleViolations))
		return false;

	Player* reporter = getPlayerByName(name);
	if(!reporter)
		return false;

	RuleViolationsMap::iterator it = ruleViolations.find(reporter->getID());
	if(it == ruleViolations.end())
		return false;

	RuleViolation& rvr = *it->second;
	if(!rvr.isOpen)
		return false;

	rvr.isOpen = false;
	rvr.gamemaster = player;
	if(ChatChannel* channel = g_chat.getChannelById(CHANNEL_RVR))
	{
		UsersMap tmpMap = channel->getUsers();
		for(UsersMap::iterator tit = tmpMap.begin(); tit != tmpMap.end(); ++tit)
			tit->second->sendRemoveReport(reporter->getName());
	}

	return true;
}

bool Game::playerCloseRuleViolation(uint32_t playerId, const std::string& name)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	Player* reporter = getPlayerByName(name);
	if(!reporter)
		return false;

	return closeRuleViolation(reporter);
}

bool Game::playerCancelRuleViolation(uint32_t playerId)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	return cancelRuleViolation(player);
}

bool Game::playerOpenPrivateChannel(uint32_t playerId, std::string& receiver)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	bool deny = false;
	CreatureEventList openEvents = player->getCreatureEvents(CREATURE_EVENT_CHANNEL_REQUEST);
	for(CreatureEventList::iterator it = openEvents.begin(); it != openEvents.end(); ++it)
	{
		if(!(*it)->executeChannelRequest(player, receiver, true, !player->hasSentChat()) && !deny)
			deny = true;
	}

	player->setSentChat(false);
	if(deny)
		return false;

	if(IOLoginData::getInstance()->playerExists(receiver))
		player->sendOpenPrivateChannel(receiver);
	else
		player->sendCancel("A player with this name does not exist.");

	return true;
}

bool Game::playerReceivePing(uint32_t playerId)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	player->receivePing();
	return true;
}

bool Game::playerAutoWalk(uint32_t playerId, std::list<Direction>& listDir)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	player->setIdleTime(0);
	if(player->getNoMove())
	{
		player->sendCancelWalk();
		return false;
	}

	if(player->hasCondition(CONDITION_GAMEMASTER, GAMEMASTER_TELEPORT))
	{
		Position pos = player->getPosition();
		for(std::list<Direction>::iterator it = listDir.begin(); it != listDir.end(); ++it)
			pos = getNextPosition((*it), pos);

		pos = getClosestFreeTile(player, pos, true, false);
		if(!pos.x || !pos.y)
		{
			player->sendCancelWalk();
			return false;
		}

		internalCreatureTurn(player, getDirectionTo(player->getPosition(), pos, false));
		internalTeleport(player, pos, true);
		return true;
	}

	player->setNextWalkTask(NULL);
	return player->startAutoWalk(listDir);
}

bool Game::playerStopAutoWalk(uint32_t playerId)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	player->stopWalk();
	return true;
}

bool Game::playerUseItemEx(uint32_t playerId, const Position& fromPos, int16_t fromStackpos, uint16_t fromSpriteId,
	const Position& toPos, int16_t toStackpos, uint16_t toSpriteId)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	Thing* thing = internalGetThing(player, fromPos, fromStackpos, fromSpriteId, STACKPOS_USEITEM);
	if(!thing)
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		return false;
	}

	Item* item = thing->getItem();
	if(!item || !item->isUsable())
	{
		player->sendCancelMessage(RET_CANNOTUSETHISOBJECT);
		return false;
	}

	Position walkToPos = fromPos;
	ReturnValue ret = g_actions->canUse(player, fromPos);
	if(ret == RET_NOERROR)
	{
		ret = g_actions->canUseEx(player, toPos, item);
		if(ret == RET_TOOFARAWAY)
		{
			if(!player->hasCustomFlag(PlayerCustomFlag_CanUseFar))
				walkToPos = toPos;
			else
				ret = RET_NOERROR;
		}
	}

	if(ret != RET_NOERROR)
	{
		if(ret == RET_TOOFARAWAY)
		{
			Position itemPos = fromPos;
			if(fromPos.x != 0xFFFF && toPos.x != 0xFFFF && Position::areInRange<1,1,0>(fromPos,
				player->getPosition()) && !Position::areInRange<1,1,0>(fromPos, toPos))
			{
				Item* moveItem = NULL;
				ReturnValue retTmp = internalMoveItem(player, item->getParent(), player,
					INDEX_WHEREEVER, item, item->getItemCount(), &moveItem);
				if(retTmp != RET_NOERROR)
				{
					player->sendCancelMessage(retTmp);
					return false;
				}

				//changing the position since its now in the inventory of the player
				internalGetPosition(moveItem, itemPos, fromStackpos);
			}

			if(player->getNoMove())
			{
				player->sendCancelMessage(RET_NOTPOSSIBLE);
				return false;
			}

			std::list<Direction> listDir;
			if(getPathToEx(player, walkToPos, listDir, 0, 1, true, true, 10))
			{
				Dispatcher::getInstance().addTask(createTask(boost::bind(&Game::playerAutoWalk,
					this, player->getID(), listDir)));

				SchedulerTask* task = createSchedulerTask(std::max((int32_t)SCHEDULER_MINTICKS, player->getStepDuration()),
					boost::bind(&Game::playerUseItemEx, this, playerId, itemPos, fromStackpos, fromSpriteId, toPos, toStackpos, toSpriteId));

				player->setNextWalkActionTask(task);
				return true;
			}

			ret = RET_THEREISNOWAY;
		}

		player->sendCancelMessage(ret);
		return false;
	}

	if(!player->canDoAction())
	{
		SchedulerTask* task = createSchedulerTask(player->getNextActionTime(),
			boost::bind(&Game::playerUseItemEx, this, playerId, fromPos, fromStackpos, fromSpriteId, toPos, toStackpos, toSpriteId));
		player->setNextActionTask(task);
		return false;
	}

	player->setIdleTime(0);
	player->setNextActionTask(NULL);
	return g_actions->useItemEx(player, fromPos, toPos, toStackpos, item);
}

bool Game::playerUseItem(uint32_t playerId, const Position& pos, int16_t stackpos,
	uint8_t index, uint16_t spriteId)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	Thing* thing = internalGetThing(player, pos, stackpos, spriteId, STACKPOS_USEITEM);
	if(!thing)
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		return false;
	}

	Item* item = thing->getItem();
	if(!item || item->isUsable())
	{
		player->sendCancelMessage(RET_CANNOTUSETHISOBJECT);
		return false;
	}

	ReturnValue ret = g_actions->canUse(player, pos);
	if(ret == RET_TOOFARAWAY && player->hasCustomFlag(PlayerCustomFlag_CanUseFar))
		ret = RET_NOERROR;

	if(ret != RET_NOERROR)
	{
		if(ret == RET_TOOFARAWAY)
		{
			if(player->getNoMove())
			{
				player->sendCancelMessage(RET_NOTPOSSIBLE);
				return false;
			}

			std::list<Direction> listDir;
			if(getPathToEx(player, pos, listDir, 0, 1, true, true))
			{
				Dispatcher::getInstance().addTask(createTask(boost::bind(&Game::playerAutoWalk,
					this, player->getID(), listDir)));

				SchedulerTask* task = createSchedulerTask(std::max((int32_t)SCHEDULER_MINTICKS, player->getStepDuration()),
					boost::bind(&Game::playerUseItem, this, playerId, pos, stackpos, index, spriteId));

				player->setNextWalkActionTask(task);
				return true;
			}

			ret = RET_THEREISNOWAY;
		}

		player->sendCancelMessage(ret);
		return false;
	}

	if(!player->canDoAction())
	{
		SchedulerTask* task = createSchedulerTask(player->getNextActionTime(),
			boost::bind(&Game::playerUseItem, this, playerId, pos, stackpos, index, spriteId));
		player->setNextActionTask(task);
		return false;
	}

	player->setIdleTime(0);
	player->setNextActionTask(NULL);
	return g_actions->useItem(player, pos, index, item);
}

bool Game::playerUseBattleWindow(uint32_t playerId, const Position& pos, int16_t stackpos,
	uint32_t creatureId, uint16_t spriteId)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	Creature* creature = getCreatureByID(creatureId);
	if(!creature)
		return false;

	if(!Position::areInRange<7,5,0>(creature->getPosition(), player->getPosition()))
		return false;

	Thing* thing = internalGetThing(player, pos, stackpos, spriteId, STACKPOS_USE);
	if(!thing)
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		return false;
	}

	Item* item = thing->getItem();
	if(!item || item->getClientID() != spriteId)
	{
		player->sendCancelMessage(RET_CANNOTUSETHISOBJECT);
		return false;
	}

	ReturnValue ret = g_actions->canUse(player, pos);
	if(ret != RET_NOERROR)
	{
		if(ret == RET_TOOFARAWAY)
		{
			if(player->getNoMove())
			{
				player->sendCancelMessage(RET_NOTPOSSIBLE);
				return false;
			}

			std::list<Direction> listDir;
			if(getPathToEx(player, item->getPosition(), listDir, 0, 1, true, true))
			{
				Dispatcher::getInstance().addTask(createTask(boost::bind(&Game::playerAutoWalk,
					this, player->getID(), listDir)));

				SchedulerTask* task = createSchedulerTask(std::max((int32_t)SCHEDULER_MINTICKS, player->getStepDuration()),
					boost::bind(&Game::playerUseBattleWindow, this, playerId, pos, stackpos, creatureId, spriteId));

				player->setNextWalkActionTask(task);
				return true;
			}

			ret = RET_THEREISNOWAY;
		}

		player->sendCancelMessage(ret);
		return false;
	}

	if(!player->canDoAction())
	{
		SchedulerTask* task = createSchedulerTask(player->getNextActionTime(),
			boost::bind(&Game::playerUseBattleWindow, this, playerId, pos, stackpos, creatureId, spriteId));
		player->setNextActionTask(task);
		return false;
	}

	player->setIdleTime(0);
	player->setNextActionTask(NULL);
	return g_actions->useItemEx(player, pos, creature->getPosition(),
		creature->getParent()->__getIndexOfThing(creature), item, creatureId);
}

bool Game::playerCloseContainer(uint32_t playerId, uint8_t cid)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	player->closeContainer(cid);
	player->sendCloseContainer(cid);
	return true;
}

bool Game::playerMoveUpContainer(uint32_t playerId, uint8_t cid)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	Container* container = player->getContainer(cid);
	if(!container)
		return false;

	Container* parentContainer = dynamic_cast<Container*>(container->getParent());
	if(!parentContainer)
		return false;

	player->addContainer(cid, parentContainer);
	player->sendContainer(cid, parentContainer, (dynamic_cast<const Container*>(parentContainer->getParent()) != NULL));
	return true;
}

bool Game::playerUpdateTile(uint32_t playerId, const Position& pos)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	if(!player->canSee(pos))
		return false;

	if(Tile* tile = getTile(pos))
		player->sendUpdateTile(tile, pos);

	return true;
}

bool Game::playerUpdateContainer(uint32_t playerId, uint8_t cid)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	Container* container = player->getContainer(cid);
	if(!container)
		return false;

	player->sendContainer(cid, container, (dynamic_cast<const Container*>(container->getParent()) != NULL));
	return true;
}

bool Game::playerRotateItem(uint32_t playerId, const Position& pos, int16_t stackpos, const uint16_t spriteId)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	Thing* thing = internalGetThing(player, pos, stackpos);
	if(!thing)
		return false;

	Item* item = thing->getItem();
	if(!item || item->getClientID() != spriteId || !item->isRoteable() || (item->isLoadedFromMap() &&
		(item->getUniqueId() || (item->getActionId() && item->getContainer()))))
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		return false;
	}

	if(pos.x != 0xFFFF && !Position::areInRange<1,1,0>(pos, player->getPosition()))
	{
		if(player->getNoMove())
		{
			player->sendCancelMessage(RET_NOTPOSSIBLE);
			return false;
		}

		std::list<Direction> listDir;
		if(getPathToEx(player, pos, listDir, 0, 1, true, true))
		{
			Dispatcher::getInstance().addTask(createTask(boost::bind(&Game::playerAutoWalk,
				this, player->getID(), listDir)));

			SchedulerTask* task = createSchedulerTask(std::max((int32_t)SCHEDULER_MINTICKS, player->getStepDuration()),
				boost::bind(&Game::playerRotateItem, this, playerId, pos, stackpos, spriteId));

			player->setNextWalkActionTask(task);
			return true;
		}

		player->sendCancelMessage(RET_THEREISNOWAY);
		return false;
	}

	uint16_t newId = Item::items[item->getID()].rotateTo;
	if(newId)
		transformItem(item, newId);

	return true;
}

bool Game::playerWriteItem(uint32_t playerId, uint32_t windowTextId, const std::string& text)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	uint16_t maxTextLength = 0;
	uint32_t internalWindowTextId = 0;

	Item* writeItem = player->getWriteItem(internalWindowTextId, maxTextLength);
	if(text.length() > maxTextLength || windowTextId != internalWindowTextId)
		return false;

	if(!writeItem || writeItem->isRemoved())
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		return false;
	}

	Cylinder* topParent = writeItem->getTopParent();
	Player* owner = dynamic_cast<Player*>(topParent);
	if(owner && owner != player)
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		return false;
	}

	if(!Position::areInRange<1,1,0>(writeItem->getPosition(), player->getPosition()))
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		return false;
	}

	bool deny = false;
	CreatureEventList textEditEvents = player->getCreatureEvents(CREATURE_EVENT_TEXTEDIT);
	for(CreatureEventList::iterator it = textEditEvents.begin(); it != textEditEvents.end(); ++it)
	{
		if(!(*it)->executeTextEdit(player, writeItem, text))
			deny = true;
	}

	player->setWriteItem(NULL);
	if((Container*)writeItem->getParent() == &player->transferContainer)
	{
		player->transferContainer.setParent(NULL);
		player->transferContainer.__removeThing(writeItem, writeItem->getItemCount());

		freeThing(writeItem);
		return true;
	}

	if(deny)
		return false;

	if(!text.empty())
	{
		if(writeItem->getText() != text)
		{
			writeItem->setText(text);
			writeItem->setWriter(player->getName());
			writeItem->setDate(std::time(NULL));
		}
	}
	else
	{
		writeItem->resetText();
		writeItem->resetWriter();
		writeItem->resetDate();
	}

	uint16_t newId = Item::items[writeItem->getID()].writeOnceItemId;
	if(newId)
		transformItem(writeItem, newId);

	return true;
}

bool Game::playerUpdateHouseWindow(uint32_t playerId, uint8_t listId, uint32_t windowTextId, const std::string& text)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	uint32_t internalWindowTextId = 0, internalListId = 0;
	House* house = player->getEditHouse(internalWindowTextId, internalListId);
	if(!house || internalWindowTextId != windowTextId || listId)
		return true;

	bool deny = false;
	CreatureEventList houseEditEvents = player->getCreatureEvents(CREATURE_EVENT_HOUSEEDIT);
	for(CreatureEventList::iterator it = houseEditEvents.begin(); it != houseEditEvents.end(); ++it)
	{
		if(!(*it)->executeHouseEdit(player, house->getId(), internalListId, text))
			deny = true;
	}

	player->setEditHouse(NULL);
	if(deny)
		return false;

	house->setAccessList(internalListId, text);
	return true;
}

bool Game::playerRequestTrade(uint32_t playerId, const Position& pos, int16_t stackpos,
	uint32_t tradePlayerId, uint16_t spriteId)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	if(player->hasCondition(CONDITION_EXHAUST, 8))
	{
		player->sendTextMessage(MSG_STATUS_SMALL, "You have to wait a while.");
		return false;
	}

	if(Condition* conditiontrade = Condition::createCondition(CONDITIONID_DEFAULT, CONDITION_EXHAUST, 300, 0, false, 8))
		player->addCondition(conditiontrade);

	Player* tradePartner = getPlayerByID(tradePlayerId);
	if(!tradePartner || tradePartner == player)
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		return false;
	}

	if(!Position::areInRange<2,2,0>(tradePartner->getPosition(), player->getPosition()))
	{
		player->sendCancelMessage(RET_DESTINATIONOUTOFREACH);
		return false;
	}

	if(!canThrowObjectTo(tradePartner->getPosition(), player->getPosition())
		&& !player->hasCustomFlag(PlayerCustomFlag_CanThrowAnywhere))
	{
		player->sendCancelMessage(RET_CANNOTTHROW);
		return false;
	}

	Item* tradeItem = dynamic_cast<Item*>(internalGetThing(player, pos, stackpos, spriteId, STACKPOS_USE));
	if(!tradeItem || tradeItem->getClientID() != spriteId || !tradeItem->isPickupable() || (tradeItem->isLoadedFromMap()
		&& (tradeItem->getUniqueId() || (tradeItem->getActionId() && tradeItem->getContainer()))))
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		return false;
	}

	if(player->getPosition().z > tradeItem->getPosition().z)
	{
		player->sendCancelMessage(RET_FIRSTGOUPSTAIRS);
		return false;
	}

	if(player->getPosition().z < tradeItem->getPosition().z)
	{
		player->sendCancelMessage(RET_FIRSTGODOWNSTAIRS);
		return false;
	}

	HouseTile* houseTile = dynamic_cast<HouseTile*>(tradeItem->getParent());
	if(houseTile && houseTile->getHouse() && !houseTile->getHouse()->isInvited(player))
	{
		player->sendCancelMessage(RET_PLAYERISNOTINVITED);
		return false;
	}

	if(!Position::areInRange<1,1,0>(tradeItem->getPosition(), player->getPosition()))
	{
		if(player->getNoMove())
		{
			player->sendCancelMessage(RET_NOTPOSSIBLE);
			return false;
		}

		std::list<Direction> listDir;
		if(getPathToEx(player, pos, listDir, 0, 1, true, true))
		{
			Dispatcher::getInstance().addTask(createTask(boost::bind(&Game::playerAutoWalk,
				this, player->getID(), listDir)));

			SchedulerTask* task = createSchedulerTask(std::max((int32_t)SCHEDULER_MINTICKS, player->getStepDuration()),
				boost::bind(&Game::playerRequestTrade, this, playerId, pos, stackpos, tradePlayerId, spriteId));

			player->setNextWalkActionTask(task);
			return true;
		}

		player->sendCancelMessage(RET_THEREISNOWAY);
		return false;
	}

	const Container* container = NULL;
	for(std::map<Item*, uint32_t>::const_iterator it = tradeItems.begin(); it != tradeItems.end(); ++it)
	{
		if(tradeItem == it->first ||
			((container = dynamic_cast<const Container*>(tradeItem)) && container->isHoldingItem(it->first)) ||
			((container = dynamic_cast<const Container*>(it->first)) && container->isHoldingItem(tradeItem)))
		{
			player->sendCancelMessage(RET_YOUAREALREADYTRADING);
			return false;
		}
	}

	Container* tradeContainer = tradeItem->getContainer();
	if(tradeContainer && tradeContainer->getItemHoldingCount() + 1 > (uint32_t)g_config.getNumber(ConfigManager::TRADE_LIMIT))
	{
		player->sendCancelMessage(RET_YOUCANONLYTRADEUPTOX);
		return false;
	}

	bool deny = false;
	CreatureEventList tradeEvents = player->getCreatureEvents(CREATURE_EVENT_TRADE_REQUEST);
	for(CreatureEventList::iterator it = tradeEvents.begin(); it != tradeEvents.end(); ++it)
	{
		if(!(*it)->executeTradeRequest(player, tradePartner, tradeItem))
			deny = true;
	}

	if(deny)
		return false;

	return internalStartTrade(player, tradePartner, tradeItem);
}

bool Game::internalStartTrade(Player* player, Player* tradePartner, Item* tradeItem)
{
	if(player->tradeState != TRADE_NONE && !(player->tradeState == TRADE_ACKNOWLEDGE && player->tradePartner == tradePartner))
	{
		player->sendCancelMessage(RET_YOUAREALREADYTRADING);
		return false;
	}
	else if(tradePartner->tradeState != TRADE_NONE && tradePartner->tradePartner != player)
	{
		player->sendCancelMessage(RET_THISPLAYERISALREADYTRADING);
		return false;
	}

	player->tradePartner = tradePartner;
	player->tradeItem = tradeItem;
	player->tradeState = TRADE_INITIATED;

	tradeItem->addRef();
	tradeItems[tradeItem] = player->getID();

	player->sendTradeItemRequest(player, tradeItem, true);
	if(tradePartner->tradeState == TRADE_NONE)
	{
		char buffer[100];
		sprintf(buffer, "%s wants to trade with you", player->getName().c_str());
		tradePartner->sendTextMessage(MSG_EVENT_ADVANCE, buffer);

		tradePartner->tradeState = TRADE_ACKNOWLEDGE;
		tradePartner->tradePartner = player;
	}
	else
	{
		Item* counterItem = tradePartner->tradeItem;
		player->sendTradeItemRequest(tradePartner, counterItem, false);
		tradePartner->sendTradeItemRequest(player, tradeItem, false);
	}

	return true;
}

bool Game::playerAcceptTrade(uint32_t playerId)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved() || (player->getTradeState() != TRADE_ACKNOWLEDGE
		&& player->getTradeState() != TRADE_INITIATED))
		return false;

	Player* tradePartner = player->tradePartner;
	if(!tradePartner)
		return false;

	if(!canThrowObjectTo(tradePartner->getPosition(), player->getPosition())
		&& !player->hasCustomFlag(PlayerCustomFlag_CanThrowAnywhere))
	{
		player->sendCancelMessage(RET_CREATUREISNOTREACHABLE);
		return false;
	}

	player->setTradeState(TRADE_ACCEPT);
	if(tradePartner->getTradeState() != TRADE_ACCEPT)
		return false;

	Item* tradeItem1 = player->tradeItem;
	Item* tradeItem2 = tradePartner->tradeItem;

	bool deny = false;
	CreatureEventList tradeEvents = player->getCreatureEvents(CREATURE_EVENT_TRADE_ACCEPT);
	for(CreatureEventList::iterator it = tradeEvents.begin(); it != tradeEvents.end(); ++it)
	{
		if(!(*it)->executeTradeAccept(player, tradePartner, tradeItem1, tradeItem2))
			deny = true;
	}

	if(deny)
		return false;

	player->setTradeState(TRADE_TRANSFER);
	tradePartner->setTradeState(TRADE_TRANSFER);

	std::map<Item*, uint32_t>::iterator it = tradeItems.find(tradeItem1);
	if(it != tradeItems.end())
	{
		freeThing(it->first);
		tradeItems.erase(it);
	}

	it = tradeItems.find(tradeItem2);
	if(it != tradeItems.end())
	{
		freeThing(it->first);
		tradeItems.erase(it);
	}

	ReturnValue ret1 = internalAddItem(player, tradePartner, tradeItem1, INDEX_WHEREEVER, FLAG_IGNOREAUTOSTACK, true);
	ReturnValue ret2 = internalAddItem(tradePartner, player, tradeItem2, INDEX_WHEREEVER, FLAG_IGNOREAUTOSTACK, true);

	bool success = false;
	if(ret1 == RET_NOERROR && ret2 == RET_NOERROR)
	{
		ret1 = internalRemoveItem(tradePartner, tradeItem1, tradeItem1->getItemCount(), true);
		ret2 = internalRemoveItem(player, tradeItem2, tradeItem2->getItemCount(), true);
		if(ret1 == RET_NOERROR && ret2 == RET_NOERROR)
		{
			internalMoveItem(player, tradeItem1->getParent(), tradePartner, INDEX_WHEREEVER,
				tradeItem1, tradeItem1->getItemCount(), NULL, FLAG_IGNOREAUTOSTACK);
			internalMoveItem(tradePartner, tradeItem2->getParent(), player, INDEX_WHEREEVER,
				tradeItem2, tradeItem2->getItemCount(), NULL, FLAG_IGNOREAUTOSTACK);

			tradeItem1->onTradeEvent(ON_TRADE_TRANSFER, tradePartner, player);
			tradeItem2->onTradeEvent(ON_TRADE_TRANSFER, player, tradePartner);
			success = true;
		}
	}

	if(!success)
	{
		std::string error;
		if(tradeItem2)
		{
			error = getTradeErrorDescription(ret1, tradeItem1);
			tradePartner->sendTextMessage(MSG_EVENT_ADVANCE, error);
			tradeItem2->onTradeEvent(ON_TRADE_CANCEL, tradePartner, player);
		}

		if(tradeItem1)
		{
			error = getTradeErrorDescription(ret2, tradeItem2);
			player->sendTextMessage(MSG_EVENT_ADVANCE, error);
			tradeItem1->onTradeEvent(ON_TRADE_CANCEL, player, tradePartner);
		}
	}

	player->setTradeState(TRADE_NONE);
	player->tradeItem = NULL;
	player->tradePartner = NULL;

	tradePartner->setTradeState(TRADE_NONE);
	tradePartner->tradeItem = NULL;
	tradePartner->tradePartner = NULL;

	player->sendTradeClose();
	tradePartner->sendTradeClose();
	return success;
}

std::string Game::getTradeErrorDescription(ReturnValue ret, Item* item)
{
	if(!item)
		return std::string();

	std::stringstream ss;
	if(ret == RET_NOTENOUGHCAPACITY)
	{
		ss << "You do not have enough capacity to carry";
		if(item->isStackable() && item->getItemCount() > 1)
			ss << " these objects.";
		else
			ss << " this object." ;

		ss << std::endl << " " << item->getWeightDescription();
	}
	else if(ret == RET_NOTENOUGHROOM || ret == RET_CONTAINERNOTENOUGHROOM)
	{
		ss << "You do not have enough room to carry";
		if(item->isStackable() && item->getItemCount() > 1)
			ss << " these objects.";
		else
			ss << " this object.";
	}
	else
		ss << "Trade could not be completed.";

	return ss.str().c_str();
}

bool Game::playerLookInTrade(uint32_t playerId, bool lookAtCounterOffer, int32_t index)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	Player* tradePartner = player->tradePartner;
	if(!tradePartner)
		return false;

	Item* tradeItem = NULL;
	if(lookAtCounterOffer)
		tradeItem = tradePartner->getTradeItem();
	else
		tradeItem = player->getTradeItem();

	if(!tradeItem)
		return false;

	std::stringstream ss;
	ss << "You see ";

	int32_t lookDistance = std::max(std::abs(player->getPosition().x - tradeItem->getPosition().x),
		std::abs(player->getPosition().y - tradeItem->getPosition().y));
	if(!index)
	{
		ss << tradeItem->getDescription(lookDistance);
		if(player->hasCustomFlag(PlayerCustomFlag_CanSeeItemDetails))
		{
			ss << std::endl << "ItemID: [" << tradeItem->getID() << "]";
			if(tradeItem->getActionId() > 0)
				ss << ", ActionID: [" << tradeItem->getActionId() << "]";

			if(tradeItem->getUniqueId() > 0)
				ss << ", UniqueID: [" << tradeItem->getUniqueId() << "]";

			ss << ".";
			const ItemType& it = Item::items[tradeItem->getID()];
			if(it.transformEquipTo)
				ss << std::endl << "TransformTo (onEquip): [" << it.transformEquipTo << "]";
			else if(it.transformDeEquipTo)
				ss << std::endl << "TransformTo (onDeEquip): [" << it.transformDeEquipTo << "]";
			else if(it.decayTo != -1)
				ss << std::endl << "DecayTo: [" << it.decayTo << "]";
		}

		player->sendTextMessage(MSG_INFO_DESCR, ss.str());
		return false;
	}

	Container* tradeContainer = tradeItem->getContainer();
	if(!tradeContainer || index > (int32_t)tradeContainer->getItemHoldingCount())
		return false;

	std::list<const Container*> listContainer;
	listContainer.push_back(tradeContainer);
	ItemList::const_iterator it;

	Container* tmpContainer = NULL;
	while(listContainer.size() > 0)
	{
		const Container* container = listContainer.front();
		listContainer.pop_front();
		for(it = container->getItems(); it != container->getEnd(); ++it)
		{
			if((tmpContainer = (*it)->getContainer()))
				listContainer.push_back(tmpContainer);

			--index;
			if(index)
				continue;

			ss << (*it)->getDescription(lookDistance);
			if(player->hasCustomFlag(PlayerCustomFlag_CanSeeItemDetails))
			{
				ss << std::endl << "ItemID: [" << (*it)->getID() << "]";
				if((*it)->getActionId() > 0)
					ss << ", ActionID: [" << (*it)->getActionId() << "]";

				if((*it)->getUniqueId() > 0)
					ss << ", UniqueID: [" << (*it)->getUniqueId() << "]";

				ss << ".";
				const ItemType& iit = Item::items[(*it)->getID()];
				if(iit.transformEquipTo)
					ss << std::endl << "TransformTo: [" << iit.transformEquipTo << "] (onEquip).";
				else if(iit.transformDeEquipTo)
					ss << std::endl << "TransformTo: [" << iit.transformDeEquipTo << "] (onDeEquip).";
				else if(iit.decayTo != -1)
					ss << std::endl << "DecayTo: [" << iit.decayTo << "].";
			}

			player->sendTextMessage(MSG_INFO_DESCR, ss.str());
			return true;
		}
	}

	return false;
}

bool Game::playerCloseTrade(uint32_t playerId)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	return internalCloseTrade(player);
}

bool Game::internalCloseTrade(Player* player)
{
	Player* tradePartner = player->tradePartner;
	if((tradePartner && tradePartner->getTradeState() == TRADE_TRANSFER) || player->getTradeState() == TRADE_TRANSFER)
	{
		std::clog << "[Warning - Game::internalCloseTrade] TradeState == TRADE_TRANSFER, " <<
			player->getName() << " " << player->getTradeState() << ", " <<
			tradePartner->getName() << " " << tradePartner->getTradeState() << std::endl;
		return true;
	}

	std::vector<Item*>::iterator it;
	if(player->getTradeItem())
	{
		std::map<Item*, uint32_t>::iterator it = tradeItems.find(player->getTradeItem());
		if(it != tradeItems.end())
		{
			freeThing(it->first);
			tradeItems.erase(it);
		}

		player->tradeItem->onTradeEvent(ON_TRADE_CANCEL, player, tradePartner);
		player->tradeItem = NULL;
	}

	player->setTradeState(TRADE_NONE);
	player->tradePartner = NULL;

	player->sendTextMessage(MSG_STATUS_SMALL, "Trade cancelled.");
	player->sendTradeClose();
	if(tradePartner)
	{
		if(tradePartner->getTradeItem())
		{
			std::map<Item*, uint32_t>::iterator it = tradeItems.find(tradePartner->getTradeItem());
			if(it != tradeItems.end())
			{
				freeThing(it->first);
				tradeItems.erase(it);
			}

			tradePartner->tradeItem->onTradeEvent(ON_TRADE_CANCEL, tradePartner, player);
			tradePartner->tradeItem = NULL;
		}

		tradePartner->setTradeState(TRADE_NONE);
		tradePartner->tradePartner = NULL;

		tradePartner->sendTextMessage(MSG_STATUS_SMALL, "Trade cancelled.");
		tradePartner->sendTradeClose();
	}

	return true;
}

bool Game::playerLookAt(uint32_t playerId, const Position& pos, uint16_t spriteId, int16_t stackpos)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	if(player->hasCondition(CONDITION_EXHAUST, 7))
	{
		player->sendTextMessage(MSG_STATUS_SMALL, "You have to wait a while.");
		return false;
	}

	if(Condition* conditionlook = Condition::createCondition(CONDITIONID_DEFAULT, CONDITION_EXHAUST, 300, 0, false, 7))
		player->addCondition(conditionlook);

	Thing* thing = internalGetThing(player, pos, stackpos, spriteId, STACKPOS_LOOK);
	if(!thing)
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		return false;
	}

	Position thingPos = pos;
	if(pos.x == 0xFFFF)
		thingPos = thing->getPosition();

	if(!player->canSee(thingPos))
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		return false;
	}

	Position playerPos = player->getPosition();
	int32_t lookDistance = -1;
	if(thing != player)
	{
		lookDistance = std::max(std::abs(playerPos.x - thingPos.x), std::abs(playerPos.y - thingPos.y));
		if(playerPos.z != thingPos.z)
			lookDistance += 15;
	}

	bool deny = false;
	CreatureEventList lookEvents = player->getCreatureEvents(CREATURE_EVENT_LOOK);
	for(CreatureEventList::iterator it = lookEvents.begin(); it != lookEvents.end(); ++it)
	{
		if(!(*it)->executeLook(player, thing, thingPos, stackpos, lookDistance))
			deny = true;
	}

	if(deny)
		return false;

	std::stringstream ss;
	ss << "You see " << thing->getDescription(lookDistance);
	if(player->hasCustomFlag(PlayerCustomFlag_CanSeeItemDetails))
	{
		if(Item* item = thing->getItem())
		{
			ss << std::endl << "ItemID: [" << item->getID() << "]";
			if(item->getActionId() > 0)
				ss << ", ActionID: [" << item->getActionId() << "]";

			if(item->getUniqueId() > 0)
				ss << ", UniqueID: [" << item->getUniqueId() << "]";

			ss << ".";
			const ItemType& it = Item::items[item->getID()];
			if(it.transformEquipTo)
				ss << std::endl << "TransformTo: [" << it.transformEquipTo << "] (onEquip).";
			else if(it.transformDeEquipTo)
				ss << std::endl << "TransformTo: [" << it.transformDeEquipTo << "] (onDeEquip).";

			if(it.transformUseTo)
				ss << std::endl << "TransformTo: [" << it.transformUseTo << "] (onUse).";

			if(it.decayTo != -1)
				ss << std::endl << "DecayTo: [" << it.decayTo << "].";
		}
	}

	if(player->hasCustomFlag(PlayerCustomFlag_CanSeeCreatureDetails))
	{
		if(const Creature* creature = thing->getCreature())
		{
			if(!player->hasFlag(PlayerFlag_HideHealth))
			{
				ss << std::endl << "Health: [" << creature->getHealth() << " / " << creature->getMaxHealth() << "]";
				if(creature->getMaxMana() > 0)
					ss << ", Mana: [" << creature->getMana() << " / " << creature->getMaxMana() << "]";

				ss << ".";
			}

			if(const Player* target = creature->getPlayer())
			{
				ss << std::endl << "IP: " << convertIPAddress(target->getIP());
#if CLIENT_VERSION_MIN != CLIENT_VERSION_MAX
				ss << ", Client: " << target->getClientVersion();
#endif
				ss << ".";
			}

			if(creature->isGhost())
				ss << std::endl << "* Ghost mode *";
		}
	}

	if(player->hasCustomFlag(PlayerCustomFlag_CanSeePosition))
	{
		ss << std::endl << "Position: [X: " << thingPos.x << "] [Y: " << thingPos.y << "] [Z: " << thingPos.z << "]";
		if(Tile* tile = getTile(thingPos))
		{
			if(House* house = tile->getHouse())
				ss << " [House: " << house->getId() << "]";
		}

		ss << ".";
	}

	player->sendTextMessage(MSG_INFO_DESCR, ss.str());
	return true;
}

bool Game::playerLookInBattleList(uint32_t playerId, uint32_t creatureId)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	Creature* creature = getCreatureByID(creatureId);
	if(!creature || creature->isRemoved())
		return false;

	if(!player->canSeeCreature(creature))
		return false;

	const Position& creaturePos = creature->getPosition();
	if(!player->canSee(creaturePos))
		return false;

	int32_t lookDistance;
	if(creature != player)
	{
		const Position& playerPos = player->getPosition();
		lookDistance = std::max(std::abs(playerPos.x - creaturePos.x), std::abs(playerPos.y - creaturePos.y));
		if(playerPos.z != creaturePos.z)
			lookDistance += 15;
	}
	else
		lookDistance = -1;

	std::ostringstream ss;
	ss << "You see " << creature->getDescription(lookDistance);
	if(player->hasCustomFlag(PlayerCustomFlag_CanSeeCreatureDetails))
	{
		if(!player->hasFlag(PlayerFlag_HideHealth))
		{
			ss << std::endl << "Health: [" << creature->getHealth() << " / " << creature->getMaxHealth() << "]";
			if(creature->getMaxMana() > 0)
				ss << ", Mana: [" << creature->getMana() << " / " << creature->getMaxMana() << "]";

			ss << ".";
		}

		if(const Player* target = creature->getPlayer())
		{
			ss << std::endl << "IP: " << convertIPAddress(target->getIP());
#if CLIENT_VERSION_MIN != CLIENT_VERSION_MAX
			ss << ", Client: " << target->getClientVersion();
#endif
			ss << ".";
		}

		if(creature->isGhost())
			ss << std::endl << "* Ghost mode *";
	}

	if(player->hasCustomFlag(PlayerCustomFlag_CanSeePosition))
	{
		ss << std::endl << "Position: [X: " << creaturePos.x << "] [Y: " << creaturePos.y << "] [Z: " << creaturePos.z << "]";
		ss << ".";
	}

	player->sendTextMessage(MSG_INFO_DESCR, ss.str());
	return true;
}

bool Game::playerCancelAttackAndFollow(uint32_t playerId)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	playerSetAttackedCreature(playerId, 0);
	playerFollowCreature(playerId, 0);

	player->stopWalk();
	return true;
}

bool Game::playerSetAttackedCreature(uint32_t playerId, uint32_t creatureId)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	if(player->getAttackedCreature() && !creatureId)
	{
		player->setAttackedCreature(NULL);
		player->sendCancelTarget();
		return true;
	}

	Creature* attackCreature = getCreatureByID(creatureId);
	if(!attackCreature)
	{
		player->setAttackedCreature(NULL);
		player->sendCancelTarget();
		return false;
	}

	ReturnValue ret = Combat::canTargetCreature(player, attackCreature);
	if(ret != RET_NOERROR)
	{
		if(ret != RET_NEEDEXCHANGE)
			player->sendCancelMessage(ret);

		player->sendCancelTarget();
		player->setAttackedCreature(NULL);
		return false;
	}

	player->setAttackedCreature(attackCreature);
	Dispatcher::getInstance().addTask(createTask(boost::bind(
		&Game::updateCreatureWalk, this, player->getID())));
	return true;
}

bool Game::playerFollowCreature(uint32_t playerId, uint32_t creatureId)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	Creature* followCreature = NULL;
	if(creatureId)
	{
		if(player->getNoMove())
		{
			playerCancelAttackAndFollow(playerId);
			player->sendCancelMessage(RET_NOTPOSSIBLE);
			return false;
		}

		followCreature = getCreatureByID(creatureId);
	}

	player->setAttackedCreature(NULL);
	Dispatcher::getInstance().addTask(createTask(boost::bind(
		&Game::updateCreatureWalk, this, player->getID())));
	return player->setFollowCreature(followCreature);
}

bool Game::playerSetFightModes(uint32_t playerId, fightMode_t fightMode, chaseMode_t chaseMode, secureMode_t secureMode)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	player->setFightMode(fightMode);
	player->setChaseMode(chaseMode);
	player->setSecureMode(secureMode);

	player->setLastAttack(OTSYS_TIME());
	return true;
}

bool Game::playerRequestAddVip(uint32_t playerId, const std::string& vipName)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved() || !player->canDoExAction())
		return false;

	uint32_t guid;
	bool specialVip;
	std::string name = vipName;

	player->setNextExAction(OTSYS_TIME() + g_config.getNumber(ConfigManager::CUSTOM_ACTIONS_DELAY_INTERVAL) - 10);
	if(!IOLoginData::getInstance()->getGuidByNameEx(guid, specialVip, name))
	{
		player->sendTextMessage(MSG_STATUS_SMALL, "A player with that name does not exist.");
		return false;
	}

	if(specialVip && !player->hasFlag(PlayerFlag_SpecialVIP))
	{
		player->sendTextMessage(MSG_STATUS_SMALL, "You cannot add this player.");
		return false;
	}

	if(player->hasCondition(CONDITION_EXHAUST, 1))
	{
		player->sendTextMessage(MSG_STATUS_SMALL, "Please wait few seconds before adding new player to your vip list.");
		return false;
	}

	bool online = false;
	if(Player* target = getPlayerByName(name))
		online = player->canSeeCreature(target);

	if(Condition* condition = Condition::createCondition(CONDITIONID_DEFAULT, CONDITION_EXHAUST, 3000, 0, false, 1))
		player->addCondition(condition);

	return player->addVIP(guid, name, online);
}

bool Game::playerRequestRemoveVip(uint32_t playerId, uint32_t guid)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	if(player->hasCondition(CONDITION_EXHAUST, 1))
	{
		player->sendTextMessage(MSG_STATUS_SMALL, "Please wait few seconds before deleting next player from your vip list.");
		return false;
	}

	if(Condition* condition = Condition::createCondition(CONDITIONID_DEFAULT, CONDITION_EXHAUST, 3000, 0, false, 1))
		player->addCondition(condition);

	player->removeVIP(guid);
	return true;
}

bool Game::playerTurn(uint32_t playerId, Direction dir)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	if(internalCreatureTurn(player, dir))
	{
		player->setIdleTime(0);
		return true;
	}

	if(player->getDirection() != dir || !player->hasCustomFlag(PlayerCustomFlag_CanTurnhop))
		return false;

	Position pos = getNextPosition(dir, player->getPosition());
	Tile* tile = map->getTile(pos);
	if(!tile || !tile->ground)
		return false;

	player->setIdleTime(0);
	ReturnValue ret = tile->__queryAdd(0, player, 1, FLAG_IGNOREBLOCKITEM);
	if(ret != RET_NOTENOUGHROOM && (ret != RET_NOTPOSSIBLE || player->hasCustomFlag(PlayerCustomFlag_CanMoveAnywhere)))
		return (internalTeleport(player, pos, false, FLAG_NOLIMIT, false) != RET_NOERROR);

	player->sendCancelMessage(ret);
	return false;
}

bool Game::playerRequestOutfit(uint32_t playerId)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	if(player->hasCondition(CONDITION_EXHAUST, 4))
	{
		player->sendTextMessage(MSG_STATUS_SMALL, "You have to wait a while.");
		return false;
	}

	if(Condition* conditionoutfit = Condition::createCondition(CONDITIONID_DEFAULT, CONDITION_EXHAUST, 300, 0, false, 4))
		player->addCondition(conditionoutfit);

	player->sendOutfitWindow();
	return true;
}

bool Game::playerChangeOutfit(uint32_t playerId, Outfit_t outfit)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	if(player->hasCondition(CONDITION_EXHAUST, 4))
	{
		player->sendTextMessage(MSG_STATUS_SMALL, "You have to wait a while.");
		return false;
	}

	if(Condition* conditionoutfit = Condition::createCondition(CONDITIONID_DEFAULT, CONDITION_EXHAUST, 1000, 0, false, 4))
		player->addCondition(conditionoutfit);

	if(!player->changeOutfit(outfit, true))
		return false;

	player->setIdleTime(0);
	if(!player->hasCondition(CONDITION_OUTFIT, -1))
		internalCreatureChangeOutfit(player, outfit);

	return true;
}

bool Game::playerSay(uint32_t playerId, uint16_t channelId, MessageClasses type, const std::string& receiver, const std::string& text)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	player->setIdleTime(0);

	int32_t muted = 0;
	bool mute = player->isMuted(channelId, type, muted);
	if(muted && mute)
	{
		if(muted > 0)
		{
			char buffer[75];
			sprintf(buffer, "You are still muted for %d seconds.", muted);
			player->sendTextMessage(MSG_STATUS_SMALL, buffer);
		}
		else
			player->sendTextMessage(MSG_STATUS_SMALL, "You are muted permanently.");

		return false;
	}

	if(player->isAccountManager())
	{
		if(mute)
			player->removeMessageBuffer();

		return internalCreatureSay(player, MSG_SPEAK_SAY, text, false);
	}

	if(g_talkActions->onPlayerSay(player, type == MSG_SPEAK_SAY ? (unsigned)CHANNEL_DEFAULT : channelId, text, false))
		return true;

	ReturnValue ret = RET_NOERROR;
	if(!muted)
	{
		ret = g_spells->onPlayerSay(player, text);
		if(ret == RET_NOERROR || (ret == RET_NEEDEXCHANGE &&
			!g_config.getBool(ConfigManager::BUFFER_SPELL_FAILURE)))
			return true;
	}

	if(mute)
		player->removeMessageBuffer();

	if(ret == RET_NEEDEXCHANGE)
		return true;

	uint32_t statementId = 0;
	if(g_config.getBool(ConfigManager::SAVE_STATEMENT))
		IOLoginData::getInstance()->playerStatement(player, channelId, text, statementId);

	switch(type)
	{
		case MSG_SPEAK_SAY:
			return internalCreatureSay(player, MSG_SPEAK_SAY, text, false, NULL, NULL, statementId);
		case MSG_SPEAK_WHISPER:
			return playerWhisper(player, text, statementId);
		case MSG_SPEAK_YELL:
			return playerYell(player, text, statementId);
		case MSG_PRIVATE:
		case MSG_GAMEMASTER_PRIVATE:
		case MSG_RVR_ANSWER:
			return playerSpeakTo(player, type, receiver, text, statementId);
		case MSG_CHANNEL:
		case MSG_CHANNEL_HIGHLIGHT:
		case MSG_GAMEMASTER_CHANNEL:
		case MSG_GAMEMASTER_ANONYMOUS:
		{
			if(playerSpeakToChannel(player, type, text, channelId, statementId))
				return true;

			return internalCreatureSay(player, MSG_SPEAK_SAY, text, false, NULL, NULL, statementId);
		}
		case MSG_GAMEMASTER_BROADCAST:
			return playerBroadcastMessage(player, MSG_GAMEMASTER_BROADCAST, text, statementId);
		case MSG_RVR_CHANNEL:
			return playerReportRuleViolation(player, text);
			break;
		case MSG_RVR_CONTINUE:
			return playerContinueReport(player, text);
			break;

		default:
			break;
	}

	return false;
}

bool Game::playerWhisper(Player* player, const std::string& text, uint32_t statementId)
{
	SpectatorVec list;
	getSpectators(list, player->getPosition(), false, false, 1, 1);
	internalCreatureSay(player, MSG_SPEAK_WHISPER, text, false, &list, NULL, statementId);
	return true;
}

bool Game::playerYell(Player* player, const std::string& text, uint32_t statementId)
{
	if(player->getLevel() < 20 && !player->hasFlag(PlayerFlag_CannotBeMuted) && player->getPremiumDays() < 1)
	{
		player->sendTextMessage(MSG_STATUS_SMALL, "You may not yell unless you heave reached level 20 or your account has premium status.");
		return true;
	}

	if(player->hasCondition(CONDITION_MUTED, 1))
	{
		player->sendCancelMessage(RET_YOUAREEXHAUSTED);
		return true;
	}

	if(!player->hasFlag(PlayerFlag_CannotBeMuted))
	{
		if(Condition* condition = Condition::createCondition(CONDITIONID_DEFAULT, CONDITION_MUTED, 30000, 0, false, 1))
			player->addCondition(condition);
	}

	internalCreatureSay(player, MSG_SPEAK_YELL, asUpperCaseString(text), false, NULL, NULL, statementId);
	return true;
}

bool Game::playerSpeakTo(Player* player, MessageClasses type, const std::string& receiver,
	const std::string& text, uint32_t statementId)
{
	Player* toPlayer = getPlayerByName(receiver);
	if(!toPlayer || toPlayer->isRemoved())
	{
		player->sendTextMessage(MSG_STATUS_SMALL, "A player with this name is not online.");
		return false;
	}

	bool canSee = player->canSeeCreature(toPlayer);
	if(toPlayer->hasCondition(CONDITION_GAMEMASTER, GAMEMASTER_IGNORE)
		&& !player->hasFlag(PlayerFlag_CannotBeMuted))
	{
		char buffer[70];
		if(!canSee)
			sprintf(buffer, "A player with this name is not online.");
		else
			sprintf(buffer, "Sorry, %s is currently ignoring private messages.", toPlayer->getName().c_str());

		player->sendTextMessage(MSG_STATUS_SMALL, buffer);
		return false;
	}

	if(type == MSG_GAMEMASTER_PRIVATE && (player->hasFlag(PlayerFlag_CanTalkRedPrivate) || player->hasFlag(PlayerFlag_CannotBeMuted)))
		type = MSG_PRIVATE;

	toPlayer->sendCreatureSay(player, type, text, NULL, statementId);
	toPlayer->onCreatureSay(player, type, text);
	if(!canSee)
	{
		player->sendTextMessage(MSG_STATUS_SMALL, "A player with this name is not online.");
		return false;
	}

	char buffer[80];
	sprintf(buffer, "Message sent to %s.", toPlayer->getName().c_str());
	player->sendTextMessage(MSG_STATUS_SMALL, buffer);
	return true;
}

bool Game::playerSpeakToChannel(Player* player, MessageClasses type, const std::string& text, uint16_t channelId, uint32_t statementId)
{
	if(player->hasCondition(CONDITION_EXHAUST, 6))
	{
		player->sendTextMessage(MSG_STATUS_SMALL, "You have to wait a while.");
		return false;
	}

	if(Condition* conditionchannel = Condition::createCondition(CONDITIONID_DEFAULT, CONDITION_EXHAUST, 300, 0, false, 6))
		player->addCondition(conditionchannel);

	switch(type)
	{
		case MSG_CHANNEL:
		{
			if(channelId == CHANNEL_HELP)
			{
				if(player->hasFlag(PlayerFlag_TalkOrangeHelpChannel))
					type = MSG_CHANNEL_HIGHLIGHT;

				if(player->hasFlag(PlayerFlag_CanTalkRedChannel))
					type = MSG_GAMEMASTER_CHANNEL;
			}

			break;
		}

		case MSG_GAMEMASTER_ANONYMOUS:
		{
			if(player->hasFlag(PlayerFlag_CanTalkRedChannelAnonymous))
			{
				if(text.length() < 251)
					return g_chat.talk(player, type, text, channelId, statementId, true);
			}
			else
				type = MSG_CHANNEL;
		}

		case MSG_GAMEMASTER_CHANNEL:
		{
			if(!player->hasFlag(PlayerFlag_CanTalkRedChannel))
				type = MSG_CHANNEL;
			break;
		}

		case MSG_GAMEMASTER_BROADCAST:
		{
			if(!player->hasFlag(PlayerFlag_CanBroadcast))
				type = MSG_CHANNEL;
			break;
		}

		default:
			break;
	}

	if(text.length() < 251)
		return g_chat.talk(player, type, text, channelId, statementId, false);

	player->sendCancelMessage(RET_NOTPOSSIBLE);
	return false;
}

bool Game::canThrowObjectTo(const Position& fromPos, const Position& toPos, bool checkLineOfSight /*= true*/,
	int32_t rangex/* = Map::maxClientViewportX*/, int32_t rangey/* = Map::maxClientViewportY*/)
{
	return map->canThrowObjectTo(fromPos, toPos, checkLineOfSight, rangex, rangey);
}

bool Game::playerReportRuleViolation(Player* player, const std::string& text)
{
	//Do not allow reports on multiclones worlds since reports are name-based
	if(g_config.getNumber(ConfigManager::ALLOW_CLONES))
	{
		player->sendTextMessage(MSG_INFO_DESCR, "Rule violation reports are disabled.");
		return false;
	}

	cancelRuleViolation(player);
	boost::shared_ptr<RuleViolation> rvr(new RuleViolation(player, text, time(NULL)));
	ruleViolations[player->getID()] = rvr;

	ChatChannel* channel = g_chat.getChannelById(CHANNEL_RVR);
	if(!channel)
		return false;

	for(UsersMap::const_iterator it = channel->getUsers().begin(); it != channel->getUsers().end(); ++it)
		it->second->sendCreatureChannelSay(player, MSG_RVR_CHANNEL, text, CHANNEL_RVR, rvr->time);

	return true;
}

bool Game::playerContinueReport(Player* player, const std::string& text)
{
	RuleViolationsMap::iterator it = ruleViolations.find(player->getID());
	if(it == ruleViolations.end())
		return false;

	RuleViolation& rvr = *it->second;
	Player* toPlayer = rvr.gamemaster;
	if(!toPlayer)
		return false;

	toPlayer->sendCreatureSay(player, MSG_RVR_CONTINUE, text);
	player->sendTextMessage(MSG_STATUS_SMALL, "Message sent to Gamemaster.");
	return true;
}

bool Game::isSightClear(const Position& fromPos, const Position& toPos, bool floorCheck)
{
	return map->isSightClear(fromPos, toPos, floorCheck);
}

bool Game::internalCreatureTurn(Creature* creature, Direction dir)
{
	bool deny = false;
	CreatureEventList directionEvents = creature->getCreatureEvents(CREATURE_EVENT_DIRECTION);
	for(CreatureEventList::iterator it = directionEvents.begin(); it != directionEvents.end(); ++it)
	{
		if(!(*it)->executeDirection(creature, creature->getDirection(), dir) && !deny)
			deny = true;
	}

	if(deny || creature->getDirection() == dir)
		return false;

	creature->setDirection(dir);
	const SpectatorVec& list = getSpectators(creature->getPosition());
	SpectatorVec::const_iterator it;

	//send to client
	Player* tmpPlayer = NULL;
	for(it = list.begin(); it != list.end(); ++it)
	{
		if((tmpPlayer = (*it)->getPlayer()))
			tmpPlayer->sendCreatureTurn(creature);
	}

	//event method
	for(it = list.begin(); it != list.end(); ++it)
		(*it)->onCreatureTurn(creature);

	return true;
}

bool Game::internalCreatureSay(Creature* creature, MessageClasses type, const std::string& text,
	bool ghostMode, SpectatorVec* spectators/* = NULL*/, Position* pos/* = NULL*/, uint32_t statementId/* = 0*/)
{
	Player* player = creature->getPlayer();
	if(player && player->isAccountManager() && !ghostMode)
	{
		player->manageAccount(text);
		return true;
	}

	Position destPos = creature->getPosition();
	if(pos)
		destPos = (*pos);

	SpectatorVec list;
	SpectatorVec::const_iterator it;
	if(!spectators || !spectators->size())
	{
		// This somewhat complex construct ensures that the cached SpectatorVec
		// is used if available and if it can be used, else a local vector is
		// used (hopefully the compiler will optimize away the construction of
		// the temporary when it's not used).
		if(type != MSG_SPEAK_YELL && type != MSG_SPEAK_MONSTER_YELL)
			getSpectators(list, destPos, false, false,
				Map::maxClientViewportX, Map::maxClientViewportX,
				Map::maxClientViewportY, Map::maxClientViewportY);
		else
			getSpectators(list, destPos, false, true, 18, 18, 14, 14);
	}
	else
		list = (*spectators);

	//send to client
	Player* tmpPlayer = NULL;
	for(it = list.begin(); it != list.end(); ++it)
	{
		if(!(tmpPlayer = (*it)->getPlayer()))
			continue;

		if(!ghostMode || tmpPlayer->canSeeCreature(creature))
			tmpPlayer->sendCreatureSay(creature, type, text, &destPos, statementId);
	}

	//event method
	for(it = list.begin(); it != list.end(); ++it)
		(*it)->onCreatureSay(creature, type, text, &destPos);

	return true;
}

bool Game::getPathTo(const Creature* creature, const Position& destPos,
	std::list<Direction>& listDir, int32_t maxSearchDist /*= -1*/)
{
	return map->getPathTo(creature, destPos, listDir, maxSearchDist);
}

bool Game::getPathToEx(const Creature* creature, const Position& targetPos,
	std::list<Direction>& dirList, const FindPathParams& fpp)
{
	return map->getPathMatching(creature, dirList, FrozenPathingConditionCall(targetPos), fpp);
}

bool Game::getPathToEx(const Creature* creature, const Position& targetPos, std::list<Direction>& dirList,
	uint32_t minTargetDist, uint32_t maxTargetDist, bool fullPathSearch/* = true*/,
	bool clearSight/* = true*/, int32_t maxSearchDist/* = -1*/)
{
	FindPathParams fpp;
	fpp.fullPathSearch = fullPathSearch;
	fpp.maxSearchDist = maxSearchDist;
	fpp.clearSight = clearSight;
	fpp.minTargetDist = minTargetDist;
	fpp.maxTargetDist = maxTargetDist;
	return getPathToEx(creature, targetPos, dirList, fpp);
}

bool Game::steerCreature(Creature* creature, const Position& position, uint16_t maxNodes/* = 100*/)
{
	FindPathParams fpp;
	fpp.maxClosedNodes = maxNodes;

	fpp.fullPathSearch = true;
	fpp.maxSearchDist = -1;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 1;

	std::list<Direction> dirList;
	if(!getPathToEx(creature, position, dirList, fpp))
		return false;

	if(Player* player = creature->getPlayer())
		player->setNextWalkTask(NULL);

	creature->startAutoWalk(dirList);
	return true;
}

void Game::checkCreatureWalk(uint32_t creatureId)
{
	Creature* creature = getCreatureByID(creatureId);
	if(!creature || creature->getHealth() < 1)
		return;

	creature->onWalk();
	cleanup();
}

void Game::updateCreatureWalk(uint32_t creatureId)
{
	Creature* creature = getCreatureByID(creatureId);
	if(creature && creature->getHealth() > 0)
		creature->goToFollowCreature();
}

void Game::checkCreatureAttack(uint32_t creatureId)
{
	Creature* creature = getCreatureByID(creatureId);
	if(creature && creature->getHealth() > 0)
		creature->onAttacking(0);
}

void Game::addCreatureCheck(Creature* creature)
{
	if(creature->isRemoved())
		return;

	creature->checked = true;
	if(creature->checkVector >= 0) //already in a vector, or about to be added
		return;

	toAddCheckCreatureVector.push_back(creature);
	creature->checkVector = random_range(0, EVENT_CREATURECOUNT - 1);
	creature->addRef();
}

void Game::removeCreatureCheck(Creature* creature)
{
	if(creature->checkVector == -1) //not in any vector
		return;

	creature->checked = false;
}

void Game::checkCreatures()
{
	checkCreatureEvent = Scheduler::getInstance().addEvent(createSchedulerTask(
		EVENT_CHECK_CREATURE_INTERVAL, boost::bind(&Game::checkCreatures, this)));
	if(++checkCreatureLastIndex == EVENT_CREATURECOUNT)
		checkCreatureLastIndex = 0;

	std::vector<Creature*>::iterator it;
#ifndef __GROUPED_ATTACKS__
	std::vector<Creature*> creatureVector;
	for(uint16_t i = 0; i < EVENT_CREATURECOUNT; ++i)
	{
		if(i == checkCreatureLastIndex)
			continue;

		creatureVector = checkCreatureVectors[i];
		for(it = creatureVector.begin(); it != creatureVector.end(); ++it)
		{
			if((*it)->checked && (*it)->getHealth() > 0)
				(*it)->onAttacking(EVENT_CHECK_CREATURE_INTERVAL);
		}
	}
#endif

	for(it = toAddCheckCreatureVector.begin(); it != toAddCheckCreatureVector.end(); ++it)
		checkCreatureVectors[(*it)->checkVector].push_back(*it);

	toAddCheckCreatureVector.clear();
	std::vector<Creature*>& checkCreatureVector = checkCreatureVectors[checkCreatureLastIndex];
	for(it = checkCreatureVector.begin(); it != checkCreatureVector.end(); )
	{
		if((*it)->checked)
		{
			if((*it)->getHealth() > 0 || !(*it)->onDeath())
				(*it)->onThink(EVENT_CREATURE_THINK_INTERVAL);

			++it;
		}
		else
		{
			(*it)->checkVector = -1;
			freeThing(*it);
			it = checkCreatureVector.erase(it);
		}
	}

	cleanup();
}

void Game::changeSpeed(Creature* creature, int32_t varSpeed)
{
	if(!creature)
		return;

	creature->setSpeed(creature->getSpeed() - creature->getBaseSpeed() + varSpeed);
	const SpectatorVec& list = getSpectators(creature->getPosition());
	SpectatorVec::const_iterator it;

	//send to client
	Player* tmpPlayer = NULL;
	for(it = list.begin(); it != list.end(); ++it)
	{
		if((*it) && (tmpPlayer = (*it)->getPlayer()))
			tmpPlayer->sendChangeSpeed(creature, creature->getStepSpeed());
	}
}

void Game::internalCreatureChangeOutfit(Creature* creature, const Outfit_t& outfit, bool forced/* = false*/)
{
	if(!forced)
	{
		bool deny = false;
		CreatureEventList outfitEvents = creature->getCreatureEvents(CREATURE_EVENT_OUTFIT);
		for(CreatureEventList::iterator it = outfitEvents.begin(); it != outfitEvents.end(); ++it)
		{
			if(!(*it)->executeOutfit(creature, creature->getCurrentOutfit(), outfit) && !deny)
				deny = true;
		}

		if(deny || creature->getCurrentOutfit() == outfit)
			return;
	}

	creature->setCurrentOutfit(outfit);
	const SpectatorVec& list = getSpectators(creature->getPosition());
	SpectatorVec::const_iterator it;

	//send to client
	Player* tmpPlayer = NULL;
	for(it = list.begin(); it != list.end(); ++it)
	{
		if((tmpPlayer = (*it)->getPlayer()))
			tmpPlayer->sendCreatureChangeOutfit(creature, outfit);
	}

	//event method
	for(it = list.begin(); it != list.end(); ++it)
		(*it)->onCreatureChangeOutfit(creature, outfit);
}

void Game::internalCreatureChangeVisible(Creature* creature, Visible_t visible)
{
	const SpectatorVec& list = getSpectators(creature->getPosition());
	SpectatorVec::const_iterator it;

	//send to client
	Player* tmpPlayer = NULL;
	for(it = list.begin(); it != list.end(); ++it)
	{
		if((tmpPlayer = (*it)->getPlayer()))
			tmpPlayer->sendCreatureChangeVisible(creature, visible);
	}

	//event method
	for(it = list.begin(); it != list.end(); ++it)
		(*it)->onCreatureChangeVisible(creature, visible);
}


void Game::changeLight(const Creature* creature)
{
	const SpectatorVec& list = getSpectators(creature->getPosition());

	//send to client
	Player* tmpPlayer = NULL;
	for(SpectatorVec::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		if((tmpPlayer = (*it)->getPlayer()) && !tmpPlayer->hasCustomFlag(PlayerCustomFlag_HasFullLight))
			tmpPlayer->sendCreatureLight(creature);
	}
}

bool Game::combatBlockHit(CombatType_t combatType, Creature* attacker, Creature* target,
	int32_t& healthChange, bool checkDefense, bool checkArmor, bool field/* = false*/, bool element/* = false*/)
{
	if(healthChange > 0)
		return false;

	const Position& targetPos = target->getPosition();
	const SpectatorVec& list = getSpectators(targetPos);
	if(Combat::canDoCombat(attacker, target, true) != RET_NOERROR)
	{
		if(!element)
			addMagicEffect(list, targetPos, MAGIC_EFFECT_POFF, target->isGhost());

		return true;
	}

	int32_t damage = -healthChange;
	BlockType_t blockType = target->blockHit(attacker, combatType,
		damage, checkDefense, checkArmor, !field, field, element);

	healthChange = -damage;
	if(blockType == BLOCK_DEFENSE)
	{
		if(!element)
			addMagicEffect(list, targetPos, MAGIC_EFFECT_POFF);

		return true;
	}

	if(blockType == BLOCK_ARMOR)
	{
		if(!element)
			addMagicEffect(list, targetPos, MAGIC_EFFECT_BLOCKHIT);

		return true;
	}

	if(blockType != BLOCK_IMMUNITY)
		return false;

	if(element)
		return true;

	MagicEffect_t effect = MAGIC_EFFECT_NONE;
	switch(combatType)
	{
		case COMBAT_UNDEFINEDDAMAGE:
			break;

		case COMBAT_ENERGYDAMAGE:
		case COMBAT_FIREDAMAGE:
		case COMBAT_PHYSICALDAMAGE:
		case COMBAT_ICEDAMAGE:
		case COMBAT_DEATHDAMAGE:
		case COMBAT_EARTHDAMAGE:
		case COMBAT_HOLYDAMAGE:
		{
			effect = MAGIC_EFFECT_BLOCKHIT;
			break;
		}

		default:
		{
			effect = MAGIC_EFFECT_POFF;
			break;
		}
	}

	addMagicEffect(list, targetPos, effect);
	return true;
}

bool Game::combatChangeHealth(CombatType_t combatType, Creature* attacker, Creature* target, int32_t healthChange,
	MagicEffect_t hitEffect/* = MAGIC_EFFECT_UNKNOWN*/, Color_t hitColor/* = COLOR_UNKNOWN*/, bool force/* = false*/)
{
	CombatParams params;
	params.effects.hit =  hitEffect;
	params.effects.color = hitColor;

	params.combatType = combatType;
	return combatChangeHealth(params, attacker, target, healthChange, force);
}

bool Game::combatChangeHealth(const CombatParams& params, Creature* attacker, Creature* target, int32_t healthChange, bool force)
{
	const Position& targetPos = target->getPosition();
	if(healthChange > 0)
	{
		if(!force && target->getHealth() <= 0)
			return false;

		bool deny = false;
		CreatureEventList statsChangeEvents = target->getCreatureEvents(CREATURE_EVENT_STATSCHANGE);
		for(CreatureEventList::iterator it = statsChangeEvents.begin(); it != statsChangeEvents.end(); ++it)
		{
			if(!(*it)->executeStatsChange(target, attacker, STATSCHANGE_HEALTHGAIN, params.combatType, healthChange))
				deny = true;
		}

		if(deny)
			return false;

		int32_t oldHealth = target->getHealth();
		target->gainHealth(attacker, healthChange);
		if(oldHealth != target->getHealth() && g_config.getBool(ConfigManager::SHOW_HEALTH_CHANGE) && !target->isGhost() &&
			(g_config.getBool(ConfigManager::SHOW_HEALTH_CHANGE_MONSTER) || !target->getMonster()))
		{
			char buffer[20];
			sprintf(buffer, "+%d", healthChange);

			const SpectatorVec& list = getSpectators(targetPos);
			if(params.combatType != COMBAT_HEALING)
				addMagicEffect(list, targetPos, MAGIC_EFFECT_WRAPS_BLUE);

			addAnimatedText(list, targetPos, COLOR_GREEN, buffer);
		}
	}
	else
	{
		const SpectatorVec& list = getSpectators(targetPos);
		if(target->getHealth() < 1 || Combat::canDoCombat(attacker, target, true) != RET_NOERROR)
		{
			addMagicEffect(list, targetPos, MAGIC_EFFECT_POFF);
			return true;
		}

		int32_t elementDamage = 0;
		if(params.element.damage && params.element.type != COMBAT_NONE)
			elementDamage = -params.element.damage;

		int32_t damage = -healthChange;
		if(damage > 0)
		{
			if(target->hasCondition(CONDITION_MANASHIELD) && params.combatType != COMBAT_UNDEFINEDDAMAGE)
			{
				int32_t manaDamage = std::min(target->getMana(), damage + elementDamage);
				damage = std::max((int32_t)0, damage + elementDamage - manaDamage);

				elementDamage = 0; // TODO: I don't know how it works ;(
				if(manaDamage && combatChangeMana(attacker, target, -manaDamage, params.combatType, true))
					addMagicEffect(list, targetPos, MAGIC_EFFECT_LOSE_ENERGY);
			}

			damage = std::min(target->getHealth(), damage);
			if(damage > 0)
			{
				bool deny = false;
				CreatureEventList statsChangeEvents = target->getCreatureEvents(CREATURE_EVENT_STATSCHANGE);
				for(CreatureEventList::iterator it = statsChangeEvents.begin(); it != statsChangeEvents.end(); ++it)
				{
					if(!(*it)->executeStatsChange(target, attacker, STATSCHANGE_HEALTHLOSS, params.combatType, damage))
						deny = true;
				}

				if(deny)
					return false;

				target->drainHealth(attacker, params.combatType, damage);
				if(elementDamage)
					target->drainHealth(attacker, params.element.type, elementDamage);

				Color_t textColor = COLOR_NONE;
				MagicEffect_t magicEffect = MAGIC_EFFECT_NONE;

				addCreatureHealth(list, target);
				if(params.combatType == COMBAT_PHYSICALDAMAGE)
				{
					Item* splash = NULL;
					switch(target->getRace())
					{
						case RACE_VENOM:
							textColor = COLOR_LIGHTGREEN;
							magicEffect = MAGIC_EFFECT_POISON;
							splash = Item::CreateItem(ITEM_SMALLSPLASH, FLUID_GREEN);
							break;

						case RACE_BLOOD:
							textColor = COLOR_RED;
							magicEffect = MAGIC_EFFECT_DRAW_BLOOD;
							splash = Item::CreateItem(ITEM_SMALLSPLASH, FLUID_BLOOD);
							break;

						case RACE_UNDEAD:
							textColor = COLOR_GREY;
							magicEffect = MAGIC_EFFECT_HIT_AREA;
							break;

						case RACE_FIRE:
							textColor = COLOR_ORANGE;
							magicEffect = MAGIC_EFFECT_DRAW_BLOOD;
							break;

						case RACE_ENERGY:
							textColor = COLOR_PURPLE;
							magicEffect = MAGIC_EFFECT_LOSE_ENERGY;
							break;

						default:
							break;
					}

					if(splash)
					{
						internalAddItem(NULL, target->getTile(), splash, INDEX_WHEREEVER, FLAG_NOLIMIT);
						startDecay(splash);
					}
				}
				else
					getCombatDetails(params.combatType, magicEffect, textColor);

				if(params.effects.hit != MAGIC_EFFECT_UNKNOWN)
					magicEffect = params.effects.hit;

				if(params.effects.color != COLOR_UNKNOWN)
					textColor = params.effects.color;

				if(textColor < COLOR_NONE && magicEffect < MAGIC_EFFECT_NONE)
				{
					char buffer[20];
					addMagicEffect(list, targetPos, magicEffect);
					if(elementDamage)
					{
						sprintf(buffer, "%d+%d", damage, elementDamage);
						getCombatDetails(params.element.type, magicEffect, textColor);
						addMagicEffect(list, targetPos, magicEffect);
					}
					else
						sprintf(buffer, "%d", damage);
		
					addAnimatedText(list, targetPos, textColor, buffer);
					std::stringstream ss;
					uint16_t totalDamage = damage + elementDamage;
					Player* player = NULL;
					if((player = target->getPlayer()) && attacker != target)
					{
						if(attacker)
							ss << "You lose " << totalDamage << " hitpoint" << (totalDamage != 1 ? "s" : "") << " due to an attack by " << attacker->getNameDescription() << ".";
						else
							ss << "You lose " << totalDamage << " hitpoint" << (totalDamage != 1 ? "s" : "") << ".";

						player->sendTextMessage(MSG_EVENT_DEFAULT, ss.str());
					}
				}
			}
		}
	}

	return true;
}

bool Game::combatChangeMana(Creature* attacker, Creature* target, int32_t manaChange,
	CombatType_t combatType/* = COMBAT_MANADRAIN*/, bool inherited/* = false*/)
{
	const Position& targetPos = target->getPosition();
	if(manaChange > 0)
	{
		bool deny = false;
		CreatureEventList statsChangeEvents = target->getCreatureEvents(CREATURE_EVENT_STATSCHANGE);
		for(CreatureEventList::iterator it = statsChangeEvents.begin(); it != statsChangeEvents.end(); ++it)
		{
			if(!(*it)->executeStatsChange(target, attacker, STATSCHANGE_MANAGAIN, COMBAT_HEALING, manaChange))
				deny = true;
		}

		if(deny)
			return false;

		target->changeMana(manaChange);
		if(g_config.getBool(ConfigManager::SHOW_MANA_CHANGE) && !target->isGhost() && (g_config.getBool(ConfigManager::SHOW_MANA_CHANGE_MONSTER) || !target->getMonster()))
		{
			char buffer[20];
			sprintf(buffer, "+%d", manaChange);

			const SpectatorVec& list = getSpectators(targetPos);
			addAnimatedText(list, targetPos, COLOR_DARKPURPLE, buffer);
		}
	}
	else if(!inherited && Combat::canDoCombat(attacker, target, true) != RET_NOERROR)
	{
		const SpectatorVec& list = getSpectators(targetPos);
		addMagicEffect(list, targetPos, MAGIC_EFFECT_POFF);
		return false;
	}
	else
	{
		int32_t manaLoss = std::min(target->getMana(), -manaChange);
		if(manaLoss > 0)
		{
			bool deny = false;
			CreatureEventList statsChangeEvents = target->getCreatureEvents(CREATURE_EVENT_STATSCHANGE);
			for(CreatureEventList::iterator it = statsChangeEvents.begin(); it != statsChangeEvents.end(); ++it)
			{
				if(!(*it)->executeStatsChange(target, attacker, STATSCHANGE_MANALOSS, combatType, manaLoss))
					deny = true;
			}

			if(deny)
				return false;

			const SpectatorVec& list = getSpectators(targetPos);
			target->drainMana(attacker, combatType, manaLoss);
			char buffer[20];
			sprintf(buffer, "%d", manaLoss);
			addAnimatedText(list, targetPos, COLOR_BLUE, buffer);
		}
	}

	return true;
}

void Game::addCreatureHealth(const Creature* target)
{
	const SpectatorVec& list = getSpectators(target->getPosition());
	addCreatureHealth(list, target);
}

void Game::addCreatureHealth(const SpectatorVec& list, const Creature* target)
{
	Player* player = NULL;
	for(SpectatorVec::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		if((player = (*it)->getPlayer()))
			player->sendCreatureHealth(target);
	}
}

void Game::addCreatureSquare(const Creature* target, uint8_t squareColor)
{
	const SpectatorVec& list = getSpectators(target->getPosition());
	addCreatureSquare(list, target, squareColor);
}

void Game::addCreatureSquare(const SpectatorVec& list, const Creature* target, uint8_t squareColor)
{
	Player* player = NULL;
	for(SpectatorVec::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		if((player = (*it)->getPlayer()))
			player->sendCreatureSquare(target, squareColor);
	}
}

void Game::addAnimatedText(const Position& pos, uint8_t textColor, const std::string& text)
{
	const SpectatorVec& list = getSpectators(pos);
	addAnimatedText(list, pos, textColor, text);
}

void Game::addAnimatedText(const SpectatorVec& list, const Position& pos, uint8_t textColor,
	const std::string& text)
{
	Player* player = NULL;
	for(SpectatorVec::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		if((player = (*it)->getPlayer()))
			player->sendAnimatedText(pos, textColor, text);
	}
}

void Game::addMagicEffect(const Position& pos, uint8_t effect, bool ghostMode/* = false*/)
{
	if(ghostMode)
		return;

	const SpectatorVec& list = getSpectators(pos);
	addMagicEffect(list, pos, effect);
}

void Game::addMagicEffect(const SpectatorVec& list, const Position& pos, uint8_t effect,
	bool ghostMode/* = false*/)
{
	if(ghostMode)
		return;

	Player* player = NULL;
	for(SpectatorVec::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		if((player = (*it)->getPlayer()))
			player->sendMagicEffect(pos, effect);
	}
}

void Game::addDistanceEffect(const Position& fromPos, const Position& toPos, uint8_t effect)
{
	SpectatorVec list;
	getSpectators(list, fromPos, false);
	getSpectators(list, toPos, true);
	addDistanceEffect(list, fromPos, toPos, effect);
}

void Game::addDistanceEffect(const SpectatorVec& list, const Position& fromPos,
	const Position& toPos, uint8_t effect)
{
	Player* player = NULL;
	for(SpectatorVec::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		if((player = (*it)->getPlayer()))
			player->sendDistanceShoot(fromPos, toPos, effect);
	}
}

void Game::startDecay(Item* item)
{
	if(!item || !item->canDecay() || item->getDecaying() == DECAYING_TRUE)
		return;

	if(item->getDuration() > 0)
	{
		item->addRef();
		item->setDecaying(DECAYING_TRUE);
		toDecayItems.push_back(item);
	}
	else
		internalDecayItem(item);
}

void Game::internalDecayItem(Item* item)
{
	const ItemType& it = Item::items.getItemType(item->getID());
	if(it.decayTo)
	{
		Item* newItem = transformItem(item, it.decayTo);
		startDecay(newItem);
	}
	else
	{
		ReturnValue ret = internalRemoveItem(NULL, item);
		if(ret != RET_NOERROR)
			std::clog << "> DEBUG: internalDecayItem failed, error code: " << (int32_t)ret << ", item id: " << item->getID() << std::endl;
	}
}

void Game::checkDecay()
{
	checkDecayEvent = Scheduler::getInstance().addEvent(createSchedulerTask(EVENT_DECAYINTERVAL,
		boost::bind(&Game::checkDecay, this)));

	size_t bucket = (lastBucket + 1) % EVENT_DECAYBUCKETS;
	for(DecayList::iterator it = decayItems[bucket].begin(); it != decayItems[bucket].end();)
	{
		Item* item = *it;
		if(!item->canDecay())
		{
			item->setDecaying(DECAYING_FALSE);
			freeThing(item);
			it = decayItems[bucket].erase(it);
			continue;
		}

		int32_t decreaseTime = EVENT_DECAYINTERVAL * EVENT_DECAYBUCKETS;
		if((int32_t)item->getDuration() - decreaseTime < 0)
			decreaseTime = item->getDuration();

		item->decreaseDuration(decreaseTime);

		int32_t dur = item->getDuration();
		if(dur <= 0)
		{
			it = decayItems[bucket].erase(it);
			internalDecayItem(item);
			freeThing(item);
		}
		else if(dur < EVENT_DECAYINTERVAL * EVENT_DECAYBUCKETS)
		{
			it = decayItems[bucket].erase(it);
			size_t newBucket = (bucket + ((dur + EVENT_DECAYINTERVAL / 2) / 1000)) % EVENT_DECAYBUCKETS;
			if(newBucket == bucket)
			{
				internalDecayItem(item);
				freeThing(item);
			}
			else
				decayItems[newBucket].push_back(item);
		}
		else
			++it;
	}

	lastBucket = bucket;
	cleanup();
}

void Game::checkLight()
{
	checkLightEvent = Scheduler::getInstance().addEvent(createSchedulerTask(EVENT_LIGHTINTERVAL,
		boost::bind(&Game::checkLight, this)));

	lightHour = lightHour + lightHourDelta;
	if(lightHour > 1440)
		lightHour -= 1440;

	if(std::abs(lightHour - SUNRISE) < 2 * lightHourDelta)
		lightState = LIGHT_STATE_SUNRISE;
	else if(std::abs(lightHour - SUNSET) < 2 * lightHourDelta)
		lightState = LIGHT_STATE_SUNSET;

	int32_t newLightLevel = lightLevel;
	bool lightChange = false;
	switch(lightState)
	{
		case LIGHT_STATE_SUNRISE:
		{
			newLightLevel += (LIGHT_LEVEL_DAY - LIGHT_LEVEL_NIGHT) / 30;
			lightChange = true;
			break;
		}
		case LIGHT_STATE_SUNSET:
		{
			newLightLevel -= (LIGHT_LEVEL_DAY - LIGHT_LEVEL_NIGHT) / 30;
			lightChange = true;
			break;
		}
		default:
			break;
	}

	if(newLightLevel <= LIGHT_LEVEL_NIGHT)
	{
		lightLevel = LIGHT_LEVEL_NIGHT;
		lightState = LIGHT_STATE_NIGHT;
	}
	else if(newLightLevel >= LIGHT_LEVEL_DAY)
	{
		lightLevel = LIGHT_LEVEL_DAY;
		lightState = LIGHT_STATE_DAY;
	}
	else
		lightLevel = newLightLevel;

	if(lightChange)
	{
		LightInfo lightInfo;
		getWorldLightInfo(lightInfo);
		for(AutoList<Player>::iterator it = Player::autoList.begin(); it != Player::autoList.end(); ++it)
		{
			if(!it->second->hasCustomFlag(PlayerCustomFlag_HasFullLight))
				it->second->sendWorldLight(lightInfo);
		}
	}
}

void Game::checkWars()
{
	//executes every EVENT_WARSINTERVAL
	IOGuild::getInstance()->checkWars();
	if(checkEndingWars)
	{
		//executes every EVENT_WARSINTERVAL*2
		checkEndingWars = false;
		IOGuild::getInstance()->checkEndingWars();
	}
	else
		checkEndingWars = true;

	checkWarsEvent = Scheduler::getInstance().addEvent(createSchedulerTask(EVENT_WARSINTERVAL,
		boost::bind(&Game::checkWars, this)));
}

void Game::getWorldLightInfo(LightInfo& lightInfo)
{
	lightInfo.level = lightLevel;
	lightInfo.color = 0xD7;
}

bool Game::cancelRuleViolation(Player* player)
{
	RuleViolationsMap::iterator it = ruleViolations.find(player->getID());
	if(it == ruleViolations.end())
		return false;

	Player* gamemaster = it->second->gamemaster;
	if(!it->second->isOpen && gamemaster) //Send to the responser
		gamemaster->sendRuleViolationCancel(player->getName());
	else if(ChatChannel* channel = g_chat.getChannelById(CHANNEL_RVR))
	{
		UsersMap tmpMap = channel->getUsers();
		for(UsersMap::iterator tit = tmpMap.begin(); tit != tmpMap.end(); ++tit)
			tit->second->sendRemoveReport(player->getName());
	}

	//Now erase it
	ruleViolations.erase(it);
	return true;
}

bool Game::closeRuleViolation(Player* player)
{
	RuleViolationsMap::iterator it = ruleViolations.find(player->getID());
	if(it == ruleViolations.end())
		return false;

	ruleViolations.erase(it);
	player->sendLockRuleViolation();
	if(ChatChannel* channel = g_chat.getChannelById(CHANNEL_RVR))
	{
		UsersMap tmpMap = channel->getUsers();
		for(UsersMap::iterator tit = tmpMap.begin(); tit != tmpMap.end(); ++tit)
			tit->second->sendRemoveReport(player->getName());
	}

	return true;
}

void Game::updateCreatureSkull(Creature* creature)
{
	const SpectatorVec& list = getSpectators(creature->getPosition());

	//send to client
	Player* tmpPlayer = NULL;
	for(SpectatorVec::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		if((tmpPlayer = (*it)->getPlayer()))
			tmpPlayer->sendCreatureSkull(creature);
	}
}

void Game::updateCreatureShield(Creature* creature)
{
	const SpectatorVec& list = getSpectators(creature->getPosition());

	//send to client
	Player* tmpPlayer = NULL;
	for(SpectatorVec::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		if((tmpPlayer = (*it)->getPlayer()))
			tmpPlayer->sendCreatureShield(creature);
	}
}

bool Game::playerInviteToParty(uint32_t playerId, uint32_t invitedId)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	if(player->hasCondition(CONDITION_EXHAUST, 5))
	{
		player->sendTextMessage(MSG_STATUS_SMALL, "You have to wait a while.");
		return false;
	}

	if(Condition* conditionparty = Condition::createCondition(CONDITIONID_DEFAULT, CONDITION_EXHAUST, 1000, 0, false, 5))
		player->addCondition(conditionparty);

	Player* invitedPlayer = getPlayerByID(invitedId);
	if(!invitedPlayer || invitedPlayer->isRemoved() || invitedPlayer->isInviting(player))
		return false;

	if(invitedPlayer->getParty())
	{
		char buffer[90];
		sprintf(buffer, "%s is already in a party.", invitedPlayer->getName().c_str());
		player->sendTextMessage(MSG_PARTY, buffer);
		return false;
	}

	Party* party = player->getParty();
	if(!party)
		party = new Party(player);

	return party->getLeader() == player && party->invitePlayer(invitedPlayer);
}

bool Game::playerJoinParty(uint32_t playerId, uint32_t leaderId)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	if(player->hasCondition(CONDITION_EXHAUST, 5))
	{
		player->sendTextMessage(MSG_STATUS_SMALL, "You have to wait a while.");
		return false;
	}

	if(Condition* conditionparty = Condition::createCondition(CONDITIONID_DEFAULT, CONDITION_EXHAUST, 1000, 0, false, 5))
		player->addCondition(conditionparty);

	Player* leader = getPlayerByID(leaderId);
	if(!leader || leader->isRemoved() || !leader->isInviting(player))
		return false;

	if(!player->getParty())
		return leader->getParty()->join(player);

	player->sendTextMessage(MSG_PARTY, "You are already in a party.");
	return false;
}

bool Game::playerRevokePartyInvitation(uint32_t playerId, uint32_t invitedId)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved() || !player->getParty() || player->getParty()->getLeader() != player)
		return false;

	Player* invitedPlayer = getPlayerByID(invitedId);
	if(!invitedPlayer || invitedPlayer->isRemoved() || !player->isInviting(invitedPlayer))
		return false;

	player->getParty()->revokeInvitation(invitedPlayer);
	return true;
}

bool Game::playerPassPartyLeadership(uint32_t playerId, uint32_t newLeaderId)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved() || !player->getParty() || player->getParty()->getLeader() != player)
		return false;

	if(player->hasCondition(CONDITION_EXHAUST, 5))
	{
		player->sendTextMessage(MSG_STATUS_SMALL, "You have to wait a while.");
		return false;
	}

	if(Condition* conditionparty = Condition::createCondition(CONDITIONID_DEFAULT, CONDITION_EXHAUST, 1000, 0, false, 5))
		player->addCondition(conditionparty);

	Player* newLeader = getPlayerByID(newLeaderId);
	if(!newLeader || newLeader->isRemoved() || !newLeader->getParty() || newLeader->getParty() != player->getParty())
		return false;

	return player->getParty()->passLeadership(newLeader);
}

bool Game::playerLeaveParty(uint32_t playerId, bool forced/* = false*/)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved() || !player->getParty() || (player->hasCondition(CONDITION_INFIGHT) && !forced))
		return false;

	return player->getParty()->leave(player);
}

bool Game::playerSharePartyExperience(uint32_t playerId, bool activate)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	if(!player->getParty() || (!player->hasFlag(PlayerFlag_NotGainInFight)
		&& player->hasCondition(CONDITION_INFIGHT)))
		return false;

	return player->getParty()->setSharedExperience(player, activate);
}

bool Game::playerReportBug(uint32_t playerId, std::string comment)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	if(!player->hasFlag(PlayerFlag_CanReportBugs))
		return false;

	CreatureEventList reportBugEvents = player->getCreatureEvents(CREATURE_EVENT_REPORTBUG);
	for(CreatureEventList::iterator it = reportBugEvents.begin(); it != reportBugEvents.end(); ++it)
		(*it)->executeReportBug(player, comment);

	return true;
}

bool Game::playerViolationWindow(uint32_t playerId, std::string name, uint8_t reason, ViolationAction_t action,
	std::string comment, std::string statement, uint32_t statementId, bool ipBanishment)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	Group* group = player->getGroup();
	if(!group)
		return false;

	time_t length[3] = {0, 0, 0};
	int32_t pos = 0, start = comment.find("{");
	while((start = comment.find("{")) > 0 && pos < 4)
	{
		std::string::size_type end = comment.find("}", start);
		if(end == std::string::npos)
			break;

		std::string data = comment.substr(start + 1, end - 1);
		comment = comment.substr(end + 1);

		++pos;
		if(data.empty())
			continue;

		if(data == "delete")
		{
			action = ACTION_DELETION;
			continue;
		}

		time_t banTime = time(NULL);
		StringVec vec = explodeString(";", data);
		for(StringVec::iterator it = vec.begin(); it != vec.end(); ++it)
		{
			StringVec tmp = explodeString(",", *it);
			uint32_t count = 1;
			if(tmp.size() > 1)
			{
				count = atoi(tmp[1].c_str());
				if(!count)
					count = 1;
			}

			if(tmp[0][0] == 's')
				banTime += count;
			if(tmp[0][0] == 'm')
				banTime += count * 60;
			if(tmp[0][0] == 'h')
				banTime += count * 3600;
			if(tmp[0][0] == 'd')
				banTime += count * 86400;
			if(tmp[0][0] == 'w')
				banTime += count * 604800;
			if(tmp[0][0] == 'm')
				banTime += count * 2592000;
			if(tmp[0][0] == 'y')
				banTime += count * 31536000;
		}

		if(action == ACTION_DELETION)
			length[pos - 2] = banTime;
		else
			length[pos - 1] = banTime;
	}

	int16_t nameFlags = group->getNameViolationFlags(), statementFlags = group->getStatementViolationFlags();
	if((ipBanishment && ((nameFlags & IPBAN_FLAG) != IPBAN_FLAG || (statementFlags & IPBAN_FLAG) != IPBAN_FLAG)) ||
		!(nameFlags & (1 << action) || statementFlags & (1 << action)) || reason > group->getViolationReasons())
	{
		player->sendCancel("You do not have authorization for this action.");
		return false;
	}

	uint32_t commentSize = g_config.getNumber(ConfigManager::MAX_VIOLATIONCOMMENT_SIZE);
	if(comment.size() > commentSize)
	{
		char buffer[90];
		sprintf(buffer, "The comment may not exceed limit of %d characters.", commentSize);

		player->sendCancel(buffer);
		return false;
	}

	toLowerCaseString(name);
	Player* target = getPlayerByNameEx(name);
	if(!target || name == "account manager")
	{
		player->sendCancel("A player with this name does not exist.");
		return false;
	}

	if(target->hasFlag(PlayerFlag_CannotBeBanned))
	{
		player->sendCancel("You do not have authorization for this action.");
		return false;
	}

	Account account = IOLoginData::getInstance()->loadAccount(target->getAccount(), true);
	enum KickAction {
		NONE = 1,
		KICK = 2,
		FULL_KICK = 3,
	} kickAction = FULL_KICK;

	pos = 1;
	switch(action)
	{
		case ACTION_STATEMENT:
		{
			StatementMap::iterator it = g_chat.statementMap.find(statementId);
			if(it == g_chat.statementMap.end())
			{
				player->sendCancel("Statement has been already reported.");
				return false;
			}

			IOBan::getInstance()->addStatement(target->getGUID(), reason, comment,
				player->getGUID(), -1, statement);
			g_chat.statementMap.erase(it);

			kickAction = NONE;
			break;
		}

		case ACTION_NAMEREPORT:
		{
			int64_t banTime = -1;
			PlayerBan_t tmp = (PlayerBan_t)g_config.getNumber(ConfigManager::NAME_REPORT_TYPE);
			if(tmp == PLAYERBAN_BANISHMENT)
			{
				if(!length[0])
					banTime = time(NULL) + g_config.getNumber(ConfigManager::BAN_LENGTH);
				else
					banTime = length[0];
			}

			if(!IOBan::getInstance()->addPlayerBanishment(target->getGUID(), banTime, reason, action,
				comment, player->getGUID(), tmp))
			{
				player->sendCancel("Player has been already reported.");
				return false;
			}
			else if(tmp == PLAYERBAN_BANISHMENT)
				account.warnings++;

			kickAction = (KickAction)tmp;
			break;
		}

		case ACTION_NOTATION:
		{
			if(!IOBan::getInstance()->addNotation(account.number, reason,
				comment, player->getGUID(), target->getGUID()))
			{
				player->sendCancel("Unable to perform action.");
				return false;
			}

			if(IOBan::getInstance()->getNotationsCount(account.number) < (uint32_t)
				g_config.getNumber(ConfigManager::NOTATIONS_TO_BAN))
			{
				kickAction = NONE;
				break;
			}

			action = ACTION_BANISHMENT;
		}

		case ACTION_BANISHMENT:
		case ACTION_BANREPORT:
		{
			bool deny = action != ACTION_BANREPORT;
			int64_t banTime = -1;
			pos = 2;

			account.warnings++;
			if(account.warnings >= g_config.getNumber(ConfigManager::WARNINGS_TO_DELETION))
				action = ACTION_DELETION;
			else if(length[0])
				banTime = length[0];
			else if(account.warnings >= g_config.getNumber(ConfigManager::WARNINGS_TO_FINALBAN))
				banTime = time(NULL) + g_config.getNumber(ConfigManager::FINALBAN_LENGTH);
			else
				banTime = time(NULL) + g_config.getNumber(ConfigManager::BAN_LENGTH);

			if(!IOBan::getInstance()->addAccountBanishment(account.number, banTime, reason, action,
				comment, player->getGUID(), target->getGUID()))
			{
				account.warnings--;
				player->sendCancel("Account is already banned.");
				return false;
			}

			if(deny)
				break;

			banTime = -1;
			PlayerBan_t tmp = (PlayerBan_t)g_config.getNumber(ConfigManager::NAME_REPORT_TYPE);
			if(tmp == PLAYERBAN_BANISHMENT)
			{
				if(!length[1])
					banTime = time(NULL) + g_config.getNumber(ConfigManager::FINALBAN_LENGTH);
				else
					banTime = length[1];
			}

			IOBan::getInstance()->addPlayerBanishment(target->getGUID(), banTime, reason, action, comment,
				player->getGUID(), tmp);
			break;
		}

		case ACTION_BANFINAL:
		case ACTION_BANREPORTFINAL:
		{
			bool allow = action == ACTION_BANREPORTFINAL;
			int64_t banTime = -1;

			account.warnings++;
			if(account.warnings >= g_config.getNumber(ConfigManager::WARNINGS_TO_DELETION))
				action = ACTION_DELETION;
			else if(length[0])
				banTime = length[0];
			else
				banTime = time(NULL) + g_config.getNumber(ConfigManager::FINALBAN_LENGTH);

			if(!IOBan::getInstance()->addAccountBanishment(account.number, banTime, reason, action,
				comment, player->getGUID(), target->getGUID()))
			{
				account.warnings--;
				player->sendCancel("Account is already banned.");
				return false;
			}

			if(action != ACTION_DELETION)
				account.warnings += (g_config.getNumber(ConfigManager::WARNINGS_TO_FINALBAN) - 1);

			if(allow)
				IOBan::getInstance()->addPlayerBanishment(target->getGUID(), -1, reason, action, comment,
					player->getGUID(), (PlayerBan_t)g_config.getNumber(
					ConfigManager::NAME_REPORT_TYPE));

			break;
		}

		case ACTION_DELETION:
		{
			//completely internal
			account.warnings++;
			if(!IOBan::getInstance()->addAccountBanishment(account.number, -1, reason, ACTION_DELETION,
				comment, player->getGUID(), target->getGUID()))
			{
				account.warnings--;
				player->sendCancel("Account is currently banned or already deleted.");
				return false;
			}

			break;
		}

		default:
			// these just shouldn't occur in rvw
			return false;
	}

	if(ipBanishment && target->getIP())
	{
		if(!length[pos])
			length[pos] = time(NULL) + g_config.getNumber(ConfigManager::IPBAN_LENGTH);

		IOBan::getInstance()->addIpBanishment(target->getIP(), length[pos], reason, comment, player->getGUID(), 0xFFFFFFFF);
	}

	if(kickAction == FULL_KICK)
		IOBan::getInstance()->removeNotations(account.number);

	std::stringstream ss;
	if(g_config.getBool(ConfigManager::BROADCAST_BANISHMENTS))
		ss << player->getName() << " has";
	else
		ss << "You have";

	ss << " taken the action \"" << getAction(action, ipBanishment) << "\"";
	switch(action)
	{
		case ACTION_NOTATION:
		{
			ss << " (" << (g_config.getNumber(ConfigManager::NOTATIONS_TO_BAN) - IOBan::getInstance()->getNotationsCount(
				account.number)) << " left to banishment)";
			break;
		}
		case ACTION_STATEMENT:
		{
			ss << " for the statement: \"" << statement << "\"";
			break;
		}
		default:
			break;
	}

	ss << " against: " << name << " (Warnings: " << account.warnings << "), with reason: \"" << getReason(
		reason) << "\", and comment: \"" << comment << "\".";
	if(g_config.getBool(ConfigManager::BROADCAST_BANISHMENTS))
		broadcastMessage(ss.str(), MSG_STATUS_WARNING);
	else
		player->sendTextMessage(MSG_STATUS_CONSOLE_RED, ss.str());

	if(target->isVirtual())
	{
		delete target;
		target = NULL;
	}
	else if(kickAction > NONE)
	{
		char buffer[30];
		sprintf(buffer, "You have been %s.", (kickAction > KICK ? "banished" : "namelocked"));
		target->sendTextMessage(MSG_INFO_DESCR, buffer);

		addMagicEffect(target->getPosition(), MAGIC_EFFECT_WRAPS_GREEN);
		Scheduler::getInstance().addEvent(createSchedulerTask(1000, boost::bind(&Game::kickPlayer, this, target->getID(), false)));
	}

	IOLoginData::getInstance()->saveAccount(account);
	return true;
}

void Game::kickPlayer(uint32_t playerId, bool displayEffect)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return;

	player->kick(displayEffect, true);
}

bool Game::broadcastMessage(const std::string& text, MessageClasses type)
{
	std::clog << "> Broadcasted message: \"" << text << "\"." << std::endl;
	for(AutoList<Player>::iterator it = Player::autoList.begin(); it != Player::autoList.end(); ++it)
		it->second->sendTextMessage(type, text);

	return true;
}

Position Game::getClosestFreeTile(Creature* creature, Position pos, bool extended/* = false*/, bool ignoreHouse/* = true*/)
{
	PairVector relList;
	relList.push_back(PositionPair(0, 0));
	relList.push_back(PositionPair(-1, -1));
	relList.push_back(PositionPair(-1, 0));
	relList.push_back(PositionPair(-1, 1));
	relList.push_back(PositionPair(0, -1));
	relList.push_back(PositionPair(0, 1));
	relList.push_back(PositionPair(1, -1));
	relList.push_back(PositionPair(1, 0));
	relList.push_back(PositionPair(1, 1));

	if(extended)
	{
		relList.push_back(PositionPair(-2, 0));
		relList.push_back(PositionPair(0, -2));
		relList.push_back(PositionPair(0, 2));
		relList.push_back(PositionPair(2, 0));
	}

	std::random_shuffle(relList.begin() + 1, relList.end());
	if(Player* player = creature->getPlayer())
	{
		for(PairVector::iterator it = relList.begin(); it != relList.end(); ++it)
		{
			Tile* tile = map->getTile(Position((pos.x + it->first), (pos.y + it->second), pos.z));
			if(!tile || !tile->ground)
				continue;

			ReturnValue ret = tile->__queryAdd(0, player, 1, FLAG_IGNOREBLOCKITEM);
			if(ret == RET_NOTENOUGHROOM || (ret == RET_NOTPOSSIBLE && !player->hasCustomFlag(
				PlayerCustomFlag_CanMoveAnywhere)) || (ret == RET_PLAYERISNOTINVITED && !ignoreHouse))
				continue;

			return tile->getPosition();
		}
	}
	else
	{
		for(PairVector::iterator it = relList.begin(); it != relList.end(); ++it)
		{
			Tile* tile = NULL;
			if((tile = map->getTile(Position((pos.x + it->first), (pos.y + it->second), pos.z)))
				&& tile->__queryAdd(0, creature, 1, FLAG_IGNOREBLOCKITEM) == RET_NOERROR)
				return tile->getPosition();
		}
	}

	return Position(0, 0, 0);
}

std::string Game::getSearchString(const Position& fromPos, const Position& toPos, bool fromIsCreature/* = false*/, bool toIsCreature/* = false*/)
{
	/*
	 * When the position is on same level and 0 to 4 squares away, they are "[toIsCreature: standing] next to you"
	 * When the position is on same level and 5 to 100 squares away they are "to the north/west/south/east."
	 * When the position is on any level and 101 to 274 squares away they are "far to the north/west/south/east."
	 * When the position is on any level and 275+ squares away they are "very far to the north/west/south/east."
	 * When the position is not directly north/west/south/east of you they are "((very) far) to the north-west/south-west/south-east/north-east."
	 * When the position is on a lower or higher level and 5 to 100 squares away they are "on a lower (or) higher level to the north/west/south/east."
	 * When the position is on a lower or higher level and 0 to 4 squares away they are "below (or) above you."
	 */

	enum direction_t
	{
		DIR_N, DIR_S, DIR_E, DIR_W,
		DIR_NE, DIR_NW, DIR_SE, DIR_SW
	};

	enum distance_t
	{
		DISTANCE_BESIDE,
		DISTANCE_CLOSE,
		DISTANCE_FAR,
		DISTANCE_VERYFAR
	};

	enum level_t
	{
		LEVEL_HIGHER,
		LEVEL_LOWER,
		LEVEL_SAME
	};

	direction_t direction;
	distance_t distance;
	level_t level;

	int32_t dx = fromPos.x - toPos.x, dy = fromPos.y - toPos.y, dz = fromPos.z - toPos.z;
	if(dz > 0)
		level = LEVEL_HIGHER;
	else if(dz < 0)
		level = LEVEL_LOWER;
	else
		level = LEVEL_SAME;

	if(std::abs(dx) < 5 && std::abs(dy) < 5)
		distance = DISTANCE_BESIDE;
	else
	{
		int32_t tmp = dx * dx + dy * dy;
		if(tmp < 10000)
			distance = DISTANCE_CLOSE;
		else if(tmp < 75625)
			distance = DISTANCE_FAR;
		else
			distance = DISTANCE_VERYFAR;
	}

	float tan;
	if(dx)
		tan = (float)dy / (float)dx;
	else
		tan = 10.;

	if(std::abs(tan) < 0.4142)
	{
		if(dx > 0)
			direction = DIR_W;
		else
			direction = DIR_E;
	}
	else if(std::abs(tan) < 2.4142)
	{
		if(tan > 0)
		{
			if(dy > 0)
				direction = DIR_NW;
			else
				direction = DIR_SE;
		}
		else
		{
			if(dx > 0)
				direction = DIR_SW;
			else
				direction = DIR_NE;
		}
	}
	else
	{
		if(dy > 0)
			direction = DIR_N;
		else
			direction = DIR_S;
	}

	std::stringstream ss;
	switch(distance)
	{
		case DISTANCE_BESIDE:
		{
			switch(level)
			{
				case LEVEL_SAME:
				{
					ss << "is ";
					if(toIsCreature)
						ss << "standing ";

					ss << "next to you";
					break;
				}

				case LEVEL_HIGHER:
				{
					ss << "is above ";
					if(fromIsCreature)
						ss << "you";

					break;
				}

				case LEVEL_LOWER:
				{
					ss << "is below ";
					if(fromIsCreature)
						ss << "you";

					break;
				}

				default:
					break;
			}

			break;
		}

		case DISTANCE_CLOSE:
		{
			switch(level)
			{
				case LEVEL_SAME:
					ss << "is to the";
					break;
				case LEVEL_HIGHER:
					ss << "is on a higher level to the";
					break;
				case LEVEL_LOWER:
					ss << "is on a lower level to the";
					break;
				default:
					break;
			}

			break;
		}

		case DISTANCE_FAR:
			ss << "is far to the";
			break;

		case DISTANCE_VERYFAR:
			ss << "is very far to the";
			break;

		default:
			break;
	}

	if(distance != DISTANCE_BESIDE)
	{
		ss << " ";
		switch(direction)
		{
			case DIR_N:
				ss << "north";
				break;

			case DIR_S:
				ss << "south";
				break;

			case DIR_E:
				ss << "east";
				break;

			case DIR_W:
				ss << "west";
				break;

			case DIR_NE:
				ss << "north-east";
				break;

			case DIR_NW:
				ss << "north-west";
				break;

			case DIR_SE:
				ss << "south-east";
				break;

			case DIR_SW:
				ss << "south-west";
				break;

			default:
				break;
		}
	}

	return ss.str();
}

double Game::getExperienceStage(uint32_t level, double divider/* = 1.*/)
{
	if(!g_config.getBool(ConfigManager::EXPERIENCE_STAGES))
		return g_config.getDouble(ConfigManager::RATE_EXPERIENCE) * divider;

	if(lastStageLevel && level >= lastStageLevel)
		return stages[lastStageLevel] * divider;

	return stages[level] * divider;
}

bool Game::loadExperienceStages()
{
	if(!g_config.getBool(ConfigManager::EXPERIENCE_STAGES))
	{
		if(!stages.empty())
			stages.clear();

		return true;
	}

	xmlDocPtr doc = xmlParseFile(getFilePath(FILE_TYPE_XML, "stages.xml").c_str());
	if(!doc)
	{
		std::clog << "[Warning - Game::loadExperienceStages] Cannot load stages file."
			<< std::endl << getLastXMLError() << std::endl;
		return false;
	}

	xmlNodePtr q, p, root = xmlDocGetRootElement(doc);
	if(xmlStrcmp(root->name, (const xmlChar*)"stages"))
	{
		std::clog << "[Error - Game::loadExperienceStages] Malformed stages file" << std::endl;
		xmlFreeDoc(doc);
		return false;
	}

	int32_t intValue, low = 0, high = 0;
	float floatValue, mul = 1.0f, defStageMultiplier;
	std::string strValue;

	lastStageLevel = 0;
	stages.clear();

	q = root->children;
	while(q)
	{
		if(!xmlStrcmp(q->name, (const xmlChar*)"world"))
		{
			if(readXMLString(q, "id", strValue))
			{
				IntegerVec intVector;
				if(!parseIntegerVec(strValue, intVector) || std::find(intVector.begin(),
					intVector.end(), g_config.getNumber(ConfigManager::WORLD_ID)) == intVector.end())
				{
					q = q->next;
					continue;
				}
			}

			defStageMultiplier = 1.0f;
			if(readXMLFloat(q, "multiplier", floatValue))
				defStageMultiplier = floatValue;

			p = q->children;
			while(p)
			{
				if(!xmlStrcmp(p->name, (const xmlChar*)"stage"))
				{
					low = 1;
					if(readXMLInteger(p, "minlevel", intValue) || readXMLInteger(p, "minLevel", intValue))
						low = intValue;

					high = 0;
					if(readXMLInteger(p, "maxlevel", intValue) || readXMLInteger(p, "maxLevel", intValue))
						high = intValue;
					else
						lastStageLevel = low;

					mul = 1.0f;
					if(readXMLFloat(p, "multiplier", floatValue))
						mul = floatValue;

					mul *= defStageMultiplier;
					if(lastStageLevel && lastStageLevel == (uint32_t)low)
						stages[lastStageLevel] = mul;
					else
					{
						for(int32_t i = low; i <= high; ++i)
							stages[i] = mul;
					}
				}

				p = p->next;
			}
		}

		if(!xmlStrcmp(q->name, (const xmlChar*)"stage"))
		{
			low = 1;
			if(readXMLInteger(q, "minlevel", intValue))
				low = intValue;
			else

			high = 0;
			if(readXMLInteger(q, "maxlevel", intValue))
				high = intValue;
			else
				lastStageLevel = low;

			mul = 1.0f;
			if(readXMLFloat(q, "multiplier", floatValue))
				mul = floatValue;

			if(lastStageLevel && lastStageLevel == (uint32_t)low)
				stages[lastStageLevel] = mul;
			else
			{
				for(int32_t i = low; i <= high; ++i)
					stages[i] = mul;
			}
		}

		q = q->next;
	}

	xmlFreeDoc(doc);
	return true;
}

int32_t Game::getMotdId()
{
	if(lastMotd.length() == g_config.getString(ConfigManager::MOTD).length())
	{
		if(lastMotd == g_config.getString(ConfigManager::MOTD))
			return lastMotdId;
	}

	lastMotd = g_config.getString(ConfigManager::MOTD);
	Database* db = Database::getInstance();

	DBQuery query;
	query << "INSERT INTO `server_motd` (`id`, `world_id`, `text`) VALUES (" << lastMotdId + 1 << ", " << g_config.getNumber(ConfigManager::WORLD_ID) << ", " << db->escapeString(lastMotd) << ")";
	if(db->query(query.str()))
		++lastMotdId;

	return lastMotdId;
}

void Game::loadMotd()
{
	Database* db = Database::getInstance();
	DBQuery query;
	query << "SELECT `id`, `text` FROM `server_motd` WHERE `world_id` = " << g_config.getNumber(ConfigManager::WORLD_ID) << " ORDER BY `id` DESC LIMIT 1";

	DBResult* result;
	if(!(result = db->storeQuery(query.str())))
	{
		std::clog << "> ERROR: Failed to load motd!" << std::endl;
		lastMotdId = random_range(5, 500);
		return;
	}

	lastMotdId = result->getDataInt("id");
	lastMotd = result->getDataString("text");
	result->free();
}

void Game::checkPlayersRecord(Player* player)
{
	uint32_t count = getPlayersOnline();
	if(count <= playersRecord)
		return;

	GlobalEventMap recordEvents = g_globalEvents->getEventMap(GLOBALEVENT_RECORD);
	for(GlobalEventMap::iterator it = recordEvents.begin(); it != recordEvents.end(); ++it)
		it->second->executeRecord(count, playersRecord, player);

	playersRecord = count;
}

void Game::loadPlayersRecord()
{
	Database* db = Database::getInstance();
	DBQuery query;
	query << "SELECT `record` FROM `server_record` WHERE `world_id` = " << g_config.getNumber(ConfigManager::WORLD_ID) << " ORDER BY `timestamp` DESC LIMIT 1";

	DBResult* result;
	if(!(result = db->storeQuery(query.str())))
	{
		std::clog << "> ERROR: Failed to load players record!" << std::endl;
		return;
	}

	playersRecord = result->getDataInt("record");
	result->free();
}

bool Game::reloadInfo(ReloadInfo_t reload, uint32_t playerId/* = 0*/, bool completeReload/* = false*/)
{
	bool done = false;
	switch(reload)
	{
		case RELOAD_ACTIONS:
		{
			if(g_actions->reload())
				done = true;
			else
				std::clog << "[Error - Game::reloadInfo] Failed to reload actions." << std::endl;

			break;
		}

		case RELOAD_CHAT:
		{
			if(g_chat.reload())
				done = true;
			else
				std::clog << "[Error - Game::reloadInfo] Failed to reload chat." << std::endl;

			break;
		}

		case RELOAD_CONFIG:
		{
			if(g_config.reload())
				done = true;
			else
				std::clog << "[Error - Game::reloadInfo] Failed to reload config." << std::endl;

			break;
		}

		case RELOAD_CREATUREEVENTS:
		{
			if(g_creatureEvents->reload())
				done = true;
			else
				std::clog << "[Error - Game::reloadInfo] Failed to reload creature events." << std::endl;

			break;
		}

		case RELOAD_GAMESERVERS:
		{
			#ifdef __LOGIN_SERVER__
			if(GameServers::getInstance()->reload())
				done = true;
			else
				std::clog << "[Error - Game::reloadInfo] Failed to reload game servers." << std::endl;

			#endif
			break;
		}

		case RELOAD_GLOBALEVENTS:
		{
			if(g_globalEvents->reload())
				done = true;
			else
				std::clog << "[Error - Game::reloadInfo] Failed to reload global events." << std::endl;

			break;
		}

		case RELOAD_GROUPS:
		{
			break;
		}

		case RELOAD_ITEMS:
		{
			std::clog << "[Notice - Game::reloadInfo] Reload type does not work." << std::endl;
			done = true;
			break;
		}

		case RELOAD_MODS:
		{
			if(ScriptManager::getInstance()->reloadMods())
				done = true;
			else
				std::clog << "[Error - Game::reloadInfo] Failed to reload mods." << std::endl;

			break;
		}

		case RELOAD_MONSTERS:
		{
			if(g_monsters.reload())
				done = true;
			else
				std::clog << "[Error - Game::reloadInfo] Failed to reload monsters." << std::endl;

			break;
		}

		case RELOAD_MOVEEVENTS:
		{
			if(g_moveEvents->reload())
				done = true;
			else
				std::clog << "[Error - Game::reloadInfo] Failed to reload move events." << std::endl;

			break;
		}

		case RELOAD_NPCS:
		{
			g_npcs.reload();
			done = true;
			break;
		}

		case RELOAD_OUTFITS:
		{
			std::clog << "[Notice - Game::reloadInfo] Reload type does not work." << std::endl;
			done = true;
			break;
		}

		case RELOAD_RAIDS:
		{
			if(!Raids::getInstance()->reload())
				std::clog << "[Error - Game::reloadInfo] Failed to reload raids." << std::endl;
			else if(!Raids::getInstance()->startup())
				std::clog << "[Error - Game::reloadInfo] Failed to startup raids when reloading." << std::endl;
			else
				done = true;

			break;
		}

		case RELOAD_SPELLS:
		{
			if(!g_spells->reload())
				std::clog << "[Error - Game::reloadInfo] Failed to reload spells." << std::endl;
			else if(!g_monsters.reload())
				std::clog << "[Error - Game::reloadInfo] Failed to reload monsters when reloading spells." << std::endl;
			else
				done = true;

			break;
		}

		case RELOAD_STAGES:
		{
			if(loadExperienceStages())
				done = true;
			else
				std::clog << "[Error - Game::reloadInfo] Failed to reload stages." << std::endl;

			break;
		}

		case RELOAD_TALKACTIONS:
		{
			if(g_talkActions->reload())
				done = true;
			else
				std::clog << "[Error - Game::reloadInfo] Failed to reload talk actions." << std::endl;

			break;
		}

		case RELOAD_VOCATIONS:
		{
			break;
		}

		case RELOAD_WEAPONS:
		{
			std::clog << "[Notice - Game::reloadInfo] Reload type does not work." << std::endl;
			done = true;
			break;
		}

		case RELOAD_ALL:
		{
			done = true;
			for(int32_t i = RELOAD_FIRST; i <= RELOAD_LAST; ++i)
			{
				if(!reloadInfo((ReloadInfo_t)i, 0, true) && done)
					done = false;
			}

			if(!ScriptManager::getInstance()->reloadMods() && done)
				done = false;

			break;
		}

		default:
		{
			std::clog << "[Warning - Game::reloadInfo] Reload type not found." << std::endl;
			break;
		}
	}

	if(reload != RELOAD_CONFIG && reload != RELOAD_MODS && !completeReload && !ScriptManager::getInstance()->reloadMods())
		std::clog << "[Error - Game::reloadInfo] Failed to reload mods." << std::endl;

	if(!playerId)
		return done;

	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return done;

	if(done)
	{
		player->sendTextMessage(MSG_STATUS_CONSOLE_BLUE, "Reloaded successfully.");
		return true;
	}

	if(reload == RELOAD_ALL)
		player->sendTextMessage(MSG_STATUS_CONSOLE_BLUE, "Failed to reload some parts.");
	else
		player->sendTextMessage(MSG_STATUS_CONSOLE_BLUE, "Failed to reload.");

	return false;
}

void Game::prepareGlobalSave(uint8_t minutes)
{
	std::clog << "Game::prepareGlobalSave in " << (uint32_t)minutes << " minutes" << std::endl;
	switch(minutes)
	{
		case 5:
			setGameState(GAMESTATE_CLOSING);
			broadcastMessage("Server is going down for a global save within 5 minutes. Please logout.", MSG_STATUS_WARNING);
			Scheduler::getInstance().addEvent(createSchedulerTask(2 * 60000, boost::bind(&Game::prepareGlobalSave, this, 3)));
			break;

		case 3:
			broadcastMessage("Server is going down for a global save within 3 minutes. Please logout.", MSG_STATUS_WARNING);
			Scheduler::getInstance().addEvent(createSchedulerTask(2 * 60000, boost::bind(&Game::prepareGlobalSave, this, 1)));
			break;

		case 1:
			broadcastMessage("Server is going down for a global save in one minute, please logout!", MSG_STATUS_WARNING);
			Scheduler::getInstance().addEvent(createSchedulerTask(60000, boost::bind(&Game::prepareGlobalSave, this, 0)));
			break;

		case 0:
			globalSave();
			break;

		default:
			if(minutes > 5)
				Scheduler::getInstance().addEvent(createSchedulerTask((minutes - 5) * 1000, boost::bind(&Game::prepareGlobalSave, this, 5)));
			break;
	}
}

void Game::globalSave()
{
	bool close = g_config.getBool(ConfigManager::SHUTDOWN_AT_GLOBALSAVE);
	if(!close) // check are we're going to close the server
		Dispatcher::getInstance().addTask(createTask(boost::bind(&Game::setGameState, this, GAMESTATE_CLOSED)));

	// call the global event
	g_globalEvents->execute(GLOBALEVENT_GLOBALSAVE);
	if(close)
	{
		//shutdown server
		Dispatcher::getInstance().addTask(createTask(boost::bind(&Game::setGameState, this, GAMESTATE_SHUTDOWN)));
		return;
	}

	//pay houses
	Houses::getInstance()->check();
	//clean map if configured to
	if(g_config.getBool(ConfigManager::CLEAN_MAP_AT_GLOBALSAVE))
		cleanMap();

	//remove premium days globally if configured to
	if(g_config.getBool(ConfigManager::INIT_PREMIUM_UPDATE))
		IOLoginData::getInstance()->updatePremiumDays();

	//reload everything
	reloadInfo(RELOAD_ALL);
	//prepare for next global save after 24 hours
	Scheduler::getInstance().addEvent(createSchedulerTask(((24 * 60 * 60) - (5 * 60)) * 1000, boost::bind(&Game::prepareGlobalSave, this, 5)));
	//open server
	Dispatcher::getInstance().addTask(createTask(boost::bind(&Game::setGameState, this, GAMESTATE_NORMAL)));
}

void Game::shutdown()
{
	std::clog << "Preparing";
	Scheduler::getInstance().shutdown();
	std::clog << " to";
	Dispatcher::getInstance().shutdown();
	std::clog << " shutdown";
	Spawns::getInstance()->clear();
	std::clog << " the";
	Raids::getInstance()->clear();
	std::clog << " server... ";
	cleanup();
	std::clog << "(done)." << std::endl;
	if(services)
		services->stop();

#if defined(WINDOWS) && !defined(_CONSOLE)
	std::exit(-1);
#endif
}

void Game::cleanup()
{
	//free memory
	for(std::vector<Thing*>::iterator it = releaseThings.begin(); it != releaseThings.end(); ++it)
		(*it)->unRef();

	releaseThings.clear();
	for(DecayList::iterator it = toDecayItems.begin(); it != toDecayItems.end(); ++it)
	{
		int32_t dur = (*it)->getDuration();
		if(dur >= EVENT_DECAYINTERVAL * EVENT_DECAYBUCKETS)
			decayItems[lastBucket].push_back(*it);
		else
			decayItems[(lastBucket + 1 + (*it)->getDuration() / 1000) % EVENT_DECAYBUCKETS].push_back(*it);
	}

	toDecayItems.clear();
}

void Game::freeThing(Thing* thing)
{
	releaseThings.push_back(thing);
}

void Game::parsePlayerExtendedOpcode(uint32_t playerId, uint8_t opcode, const std::string& buffer)
{
	Player* player = getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return;

	CreatureEventList extendedOpcodeEvents = player->getCreatureEvents(CREATURE_EVENT_EXTENDED_OPCODE);
	for(CreatureEventList::iterator it = extendedOpcodeEvents.begin(); it != extendedOpcodeEvents.end(); ++it)
		(*it)->executeExtendedOpcode(player, opcode, buffer);
}
