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

#include "creature.h"
#include "player.h"
#include "npc.h"
#include "monster.h"

#include "condition.h"
#include "combat.h"

#include "container.h"
#if defined __EXCEPTION_TRACER__
#include "exception.h"
#endif

#include "configmanager.h"
#include "game.h"

boost::recursive_mutex AutoId::lock;
uint32_t AutoId::count = 1000;
AutoId::List AutoId::list;

extern Game g_game;
extern ConfigManager g_config;
extern CreatureEvents* g_creatureEvents;

Creature::Creature()
{
	id = 0;
	_tile = NULL;
	direction = SOUTH;
	master = NULL;
	lootDrop = LOOT_DROP_FULL;
	skillLoss = true;
	hideName = hideHealth = cannotMove = false;
	speakType = MSG_NONE;
	skull = SKULL_NONE;
	partyShield = SHIELD_NONE;
	guildEmblem = GUILDEMBLEM_NONE;

	health = 1000;
	healthMax = 1000;
	mana = 0;
	manaMax = 0;

	lastStep = 0;
	lastStepCost = 1;
	baseSpeed = 220;
	varSpeed = 0;

	masterRadius = -1;
	masterPosition = Position();

	followCreature = NULL;
	hasFollowPath = false;
	removed = false;
	eventWalk = 0;
	cancelNextWalk = false;
	forceUpdateFollowPath = false;
	isMapLoaded = false;
	isUpdatingPath = false;
	checked = false;
	memset(localMapCache, false, sizeof(localMapCache));

	attackedCreature = NULL;
	lastHitCreature = 0;
	lastDamageSource = COMBAT_NONE;
	blockCount = 0;
	blockTicks = 0;
	walkUpdateTicks = 0;
	checkVector = -1;
	lastFailedFollow = 0;

	onIdleStatus();
}

Creature::~Creature()
{
	attackedCreature = NULL;
	removeConditions(CONDITIONEND_CLEANUP, false);
	for(std::list<Creature*>::iterator cit = summons.begin(); cit != summons.end(); ++cit)
	{
		(*cit)->setAttackedCreature(NULL);
		(*cit)->setMaster(NULL);
		(*cit)->unRef();
	}

	summons.clear();
	conditions.clear();
	eventsList.clear();
}

bool Creature::canSee(const Position& myPos, const Position& pos, uint32_t viewRangeX, uint32_t viewRangeY)
{
	if(myPos.z <= 7)
	{
		//we are on ground level or above (7 -> 0)
		//view is from 7 -> 0
		if(pos.z > 7)
			return false;
	}
	else if(myPos.z >= 8)
	{
		//we are underground (8 -> 15)
		//view is +/- 2 from the floor we stand on
		if(std::abs(myPos.z - pos.z) > 2)
			return false;
	}

	int32_t offsetz = myPos.z - pos.z;
	return (((uint32_t)pos.x >= myPos.x - viewRangeX + offsetz) && ((uint32_t)pos.x <= myPos.x + viewRangeX + offsetz) &&
		((uint32_t)pos.y >= myPos.y - viewRangeY + offsetz) && ((uint32_t)pos.y <= myPos.y + viewRangeY + offsetz));
}

bool Creature::canSee(const Position& pos) const
{
	return canSee(getPosition(), pos, Map::maxViewportX, Map::maxViewportY);
}

bool Creature::canSeeCreature(const Creature* creature) const
{
	return creature == this || (!creature->isGhost() && (!creature->isInvisible() || canSeeInvisibility()));
}

bool Creature::canWalkthrough(const Creature* creature) const
{
	if(creature == this)
		return true;

	if(const Creature* _master = creature->getMaster())
	{
		if(_master != this && canWalkthrough(_master))
			return true;
	}

	return creature->isGhost() || creature->isWalkable() || (master &&
		master != creature && master->canWalkthrough(creature));
}

int64_t Creature::getTimeSinceLastMove() const
{
	if(lastStep)
		return OTSYS_TIME() - lastStep;

	return 0x7FFFFFFFFFFFFFFFLL;
}

int32_t Creature::getWalkDelay(Direction dir) const
{
	if(lastStep)
		return getStepDuration(dir) - (OTSYS_TIME() - lastStep);

	return 0;
}

int32_t Creature::getWalkDelay() const
{
	if(lastStep)
		return getStepDuration() - (OTSYS_TIME() - lastStep);

	return 0;
}

void Creature::onThink(uint32_t interval)
{
	if(!isMapLoaded && useCacheMap())
	{
		isMapLoaded = true;
		updateMapCache();
	}

	if(followCreature && master != followCreature && !canSeeCreature(followCreature))
		internalCreatureDisappear(followCreature, false);

	if(attackedCreature && master != attackedCreature && !canSeeCreature(attackedCreature))
		internalCreatureDisappear(attackedCreature, false);

	blockTicks += interval;
	if(blockTicks >= 1000)
	{
		blockCount = std::min((uint32_t)blockCount + 1, (uint32_t)2);
		blockTicks = 0;
	}

	if(followCreature)
	{
		walkUpdateTicks += interval;
		if(forceUpdateFollowPath || walkUpdateTicks >= 2000)
		{
			walkUpdateTicks = 0;
			forceUpdateFollowPath = false;
			isUpdatingPath = true;
		}
	}

	if(isUpdatingPath)
	{
		isUpdatingPath = false;
		goToFollowCreature();
	}

#ifndef __GROUPED_ATTACKS__
	onAttacking(interval / EVENT_CREATURECOUNT);
#else
	onAttacking(interval);
#endif
	executeConditions(interval);

	CreatureEventList thinkEvents = getCreatureEvents(CREATURE_EVENT_THINK);
	for(CreatureEventList::iterator it = thinkEvents.begin(); it != thinkEvents.end(); ++it)
		(*it)->executeThink(this, interval);
}

void Creature::onAttacking(uint32_t interval)
{
	if(!attackedCreature || attackedCreature->getHealth() < 1 || interval < 100)
		return;

	bool deny = false;
	CreatureEventList attackEvents = getCreatureEvents(CREATURE_EVENT_ATTACK);
	for(CreatureEventList::iterator it = attackEvents.begin(); it != attackEvents.end(); ++it)
	{
		if(!(*it)->executeAction(this, attackedCreature) && !deny)
			deny = true;
	}

	if(deny)
		setAttackedCreature(NULL);

	if(!attackedCreature)
		return;

	onAttacked();
	attackedCreature->onAttacked();
	if(g_game.isSightClear(getPosition(), attackedCreature->getPosition(), true))
		doAttacking(interval);
}

void Creature::onWalk()
{
	if(getWalkDelay() <= 0)
	{
		Direction dir;
		uint32_t flags = FLAG_IGNOREFIELDDAMAGE;
		if(!getNextStep(dir, flags))
		{
			if(listWalkDir.empty())
				onWalkComplete();

			stopEventWalk();
		}
		else if(g_game.internalMoveCreature(this, dir, flags) != RET_NOERROR)
			forceUpdateFollowPath = true;
	}

	if(cancelNextWalk)
	{
		cancelNextWalk = false;
		listWalkDir.clear();
		onWalkAborted();
	}

	if(eventWalk)
	{
		eventWalk = 0;
		addEventWalk();
	}
}

void Creature::onWalk(Direction& dir)
{
	int32_t drunk = -1;
	if(!isSuppress(CONDITION_DRUNK))
	{
		Condition* condition = NULL;
		for(ConditionList::const_iterator it = conditions.begin(); it != conditions.end(); ++it)
		{
			if(!(condition = *it) || condition->getType() != CONDITION_DRUNK)
				continue;

			int32_t subId = condition->getSubId();
			if((!condition->getEndTime() || condition->getEndTime() >= OTSYS_TIME()) && subId > drunk)
				drunk = subId;
		}
	}

	if(drunk < 0)
		return;

	drunk += 25;
	int32_t r = random_range(1, 100);
	if(r > drunk)
		return;

	int32_t tmp = (drunk / 5);
	if(r <= tmp)
		dir = NORTH;
	else if(r <= (tmp * 2))
		dir = WEST;
	else if(r <= (tmp * 3))
		dir = SOUTH;
	else if(r <= (tmp * 4))
		dir = EAST;

	g_game.internalCreatureSay(this, MSG_SPEAK_MONSTER_SAY, "Hicks!", isGhost());
}

bool Creature::getNextStep(Direction& dir, uint32_t&)
{
	if(listWalkDir.empty())
		return false;

	dir = listWalkDir.front();
	listWalkDir.pop_front();
	onWalk(dir);
	return true;
}

bool Creature::startAutoWalk(std::list<Direction>& listDir)
{
	listWalkDir = listDir;
	addEventWalk(listDir.size() == 1);
	return true;
}

void Creature::addEventWalk(bool firstStep/* = false*/)
{
	cancelNextWalk = false;
	if(getStepSpeed() < 1 || eventWalk)
		return;

	int64_t ticks = getEventStepTicks(firstStep);
	if(ticks < 1)
		return;

	if(ticks == 1)
		g_game.checkCreatureWalk(getID());

	eventWalk = Scheduler::getInstance().addEvent(createSchedulerTask(std::max((int64_t)SCHEDULER_MINTICKS, ticks),
		boost::bind(&Game::checkCreatureWalk, &g_game, id)));
}

void Creature::stopEventWalk()
{
	if(!eventWalk)
		return;

	Scheduler::getInstance().stopEvent(eventWalk);
	eventWalk = 0;
}

void Creature::updateMapCache()
{
	const Position& pos = getPosition();
	Position dest(0, 0, pos.z);

	Tile* tile = NULL;
	for(int32_t y = -((mapWalkHeight - 1) / 2); y <= ((mapWalkHeight - 1) / 2); ++y)
	{
		for(int32_t x = -((mapWalkWidth - 1) / 2); x <= ((mapWalkWidth - 1) / 2); ++x)
		{
			dest.x = pos.x + x;
			dest.y = pos.y + y;
			if((tile = g_game.getTile(dest)))
				updateTileCache(tile, dest);
		}
	}
}

#ifdef __DEBUG__
void Creature::validateMapCache()
{
	const Position& myPos = getPosition();
	for(int32_t y = -((mapWalkHeight - 1) / 2); y <= ((mapWalkHeight - 1) / 2); ++y)
	{
		for(int32_t x = -((mapWalkWidth - 1) / 2); x <= ((mapWalkWidth - 1) / 2); ++x)
			getWalkCache(Position(myPos.x + x, myPos.y + y, myPos.z));
	}
}
#endif

void Creature::updateTileCache(const Tile* tile, int32_t dx, int32_t dy)
{
	if((std::abs(dx) <= (mapWalkWidth - 1) / 2) && (std::abs(dy) <= (mapWalkHeight - 1) / 2))
	{
		int32_t x = (mapWalkWidth - 1) / 2 + dx, y = (mapWalkHeight - 1) / 2 + dy;
		localMapCache[y][x] = (tile && tile->__queryAdd(0, this, 1,
			FLAG_PATHFINDING | FLAG_IGNOREFIELDDAMAGE) == RET_NOERROR);
	}
#ifdef __DEBUG__
	else
		std::clog << "Creature::updateTileCache out of range." << std::endl;
#endif
}

void Creature::updateTileCache(const Tile* tile, const Position& pos)
{
	const Position& myPos = getPosition();
	if(pos.z == myPos.z)
		updateTileCache(tile, pos.x - myPos.x, pos.y - myPos.y);
}

void Creature::updateTileCache(const Tile* tile)
{
	if(isMapLoaded && tile->getPosition().z == getPosition().z)
		updateTileCache(tile, tile->getPosition());
}

int32_t Creature::getWalkCache(const Position& pos) const
{
	if(!useCacheMap())
		return 2;

	const Position& myPos = getPosition();
	if(myPos.z != pos.z)
		return 0;

	if(pos == myPos)
		return 1;

	int32_t dx = pos.x - myPos.x, dy = pos.y - myPos.y;
	if((std::abs(dx) <= (mapWalkWidth - 1) / 2) && (std::abs(dy) <= (mapWalkHeight - 1) / 2))
	{
		int32_t x = (mapWalkWidth - 1) / 2 + dx, y = (mapWalkHeight - 1) / 2 + dy;
#ifdef __DEBUG__
		//testing
		Tile* tile = g_game.getTile(pos);
		if(tile && (tile->__queryAdd(0, this, 1, FLAG_PATHFINDING | FLAG_IGNOREFIELDDAMAGE) == RET_NOERROR))
		{
			if(!localMapCache[y][x])
				std::clog << "Wrong cache value" << std::endl;
		}
		else if(localMapCache[y][x])
			std::clog << "Wrong cache value" << std::endl;

#endif
		if(localMapCache[y][x])
			return 1;

		return 0;
	}

	//out of range
	return 2;
}

void Creature::onAddTileItem(const Tile* tile, const Position& pos, const Item*)
{
	if(isMapLoaded && pos.z == getPosition().z)
		updateTileCache(tile, pos);
}

void Creature::onUpdateTileItem(const Tile* tile, const Position& pos, const Item*,
	const ItemType& oldType, const Item*, const ItemType& newType)
{
	if(isMapLoaded && (oldType.blockSolid || oldType.blockPathFind || newType.blockPathFind
		|| newType.blockSolid) && pos.z == getPosition().z)
		updateTileCache(tile, pos);
}

void Creature::onRemoveTileItem(const Tile* tile, const Position& pos, const ItemType& iType, const Item*)
{
	if(isMapLoaded && (iType.blockSolid || iType.blockPathFind ||
		iType.isGroundTile()) && pos.z == getPosition().z)
		updateTileCache(tile, pos);
}

void Creature::onCreatureAppear(const Creature* creature)
{
	if(creature == this)
	{
		if(useCacheMap())
		{
			isMapLoaded = true;
			updateMapCache();
		}
	}
	else if(isMapLoaded && creature->getPosition().z == getPosition().z)
		updateTileCache(creature->getTile(), creature->getPosition());
}

void Creature::internalCreatureDisappear(const Creature* creature, bool isLogout)
{
	if(attackedCreature == creature)
	{
		setAttackedCreature(NULL);
		onTargetDisappear(isLogout);
	}

	if(followCreature == creature)
	{
		setFollowCreature(NULL);
		onFollowCreatureDisappear(isLogout);
	}
}

void Creature::onRemovedCreature()
{
	setRemoved();
	removeList();
	if(master && !master->isRemoved())
		master->removeSummon(this);
}

void Creature::onChangeZone(ZoneType_t zone)
{
	if(attackedCreature && zone == ZONE_PROTECTION)
		internalCreatureDisappear(attackedCreature, false);
}

void Creature::onTargetChangeZone(ZoneType_t zone)
{
	if(zone == ZONE_PROTECTION)
		internalCreatureDisappear(attackedCreature, false);
}

void Creature::onCreatureMove(const Creature* creature, const Tile* newTile, const Position& newPos,
	const Tile* oldTile, const Position& oldPos, bool teleport)
{
	if(creature == this)
	{
		if(!oldTile->floorChange() && !oldTile->positionChange())
			setLastPosition(oldPos);

		lastStep = OTSYS_TIME();
		lastStepCost = 1;
		if(!teleport)
		{
			if(oldPos.z != newPos.z)
				lastStepCost = 2;
			else if(std::abs(newPos.x - oldPos.x) >= 1 && std::abs(newPos.y - oldPos.y) >= 1)
				lastStepCost = 3;
		}
		else
			stopEventWalk();

		if(!summons.empty() && (!g_config.getBool(ConfigManager::TELEPORT_SUMMONS) ||
			(g_config.getBool(ConfigManager::TELEPORT_PLAYER_SUMMONS) && !getPlayer())))
		{
			std::list<Creature*>::iterator cit;
			std::list<Creature*> despawnList;
			for(cit = summons.begin(); cit != summons.end(); ++cit)
			{
				const Position pos = (*cit)->getPosition();
				if((std::abs(pos.z - newPos.z) > 2) || (std::max(std::abs((
					newPos.x) - pos.x), std::abs((newPos.y - 1) - pos.y)) > 30))
					despawnList.push_back(*cit);
			}

			for(cit = despawnList.begin(); cit != despawnList.end(); ++cit)
				g_game.removeCreature((*cit), true);
		}

		if(newTile->getZone() != oldTile->getZone())
			onChangeZone(getZone());

		//update map cache
		if(isMapLoaded)
		{
			if(!teleport && oldPos.z == newPos.z)
			{
				Tile* tile = NULL;
				const Position& myPos = getPosition();
				if(oldPos.y > newPos.y) //north
				{
					//shift y south
					for(int32_t y = mapWalkHeight - 1 - 1; y >= 0; --y)
						memcpy(localMapCache[y + 1], localMapCache[y], sizeof(localMapCache[y]));

					//update 0
					for(int32_t x = -((mapWalkWidth - 1) / 2); x <= ((mapWalkWidth - 1) / 2); ++x)
					{
						tile = g_game.getTile(myPos.x + x, myPos.y - ((mapWalkHeight - 1) / 2), myPos.z);
						updateTileCache(tile, x, -((mapWalkHeight - 1) / 2));
					}
				}
				else if(oldPos.y < newPos.y) // south
				{
					//shift y north
					for(int32_t y = 0; y <= mapWalkHeight - 1 - 1; ++y)
						memcpy(localMapCache[y], localMapCache[y + 1], sizeof(localMapCache[y]));

					//update mapWalkHeight - 1
					for(int32_t x = -((mapWalkWidth - 1) / 2); x <= ((mapWalkWidth - 1) / 2); ++x)
					{
						tile = g_game.getTile(myPos.x + x, myPos.y + ((mapWalkHeight - 1) / 2), myPos.z);
						updateTileCache(tile, x, (mapWalkHeight - 1) / 2);
					}
				}

				if(oldPos.x < newPos.x) // east
				{
					//shift y west
					int32_t starty = 0, endy = mapWalkHeight - 1, dy = (oldPos.y - newPos.y);
					if(dy < 0)
						endy = endy + dy;
					else if(dy > 0)
						starty = starty + dy;

					for(int32_t y = starty; y <= endy; ++y)
					{
						for(int32_t x = 0; x <= mapWalkWidth - 1 - 1; ++x)
							localMapCache[y][x] = localMapCache[y][x + 1];
					}

					//update mapWalkWidth - 1
					for(int32_t y = -((mapWalkHeight - 1) / 2); y <= ((mapWalkHeight - 1) / 2); ++y)
					{
						tile = g_game.getTile(myPos.x + ((mapWalkWidth - 1) / 2), myPos.y + y, myPos.z);
						updateTileCache(tile, (mapWalkWidth - 1) / 2, y);
					}
				}
				else if(oldPos.x > newPos.x) // west
				{
					//shift y east
					int32_t starty = 0, endy = mapWalkHeight - 1, dy = (oldPos.y - newPos.y);
					if(dy < 0)
						endy = endy + dy;
					else if(dy > 0)
						starty = starty + dy;

					for(int32_t y = starty; y <= endy; ++y)
					{
						for(int32_t x = mapWalkWidth - 1 - 1; x >= 0; --x)
							localMapCache[y][x + 1] = localMapCache[y][x];
					}

					//update 0
					for(int32_t y = -((mapWalkHeight - 1) / 2); y <= ((mapWalkHeight - 1) / 2); ++y)
					{
						tile = g_game.getTile(myPos.x - ((mapWalkWidth - 1) / 2), myPos.y + y, myPos.z);
						updateTileCache(tile, -((mapWalkWidth - 1) / 2), y);
					}
				}

				updateTileCache(oldTile, oldPos);
#ifdef __DEBUG__
				validateMapCache();
#endif
			}
			else
				updateMapCache();
		}
	}
	else if(isMapLoaded)
	{
		const Position& myPos = getPosition();
		if(newPos.z == myPos.z)
			updateTileCache(newTile, newPos);

		if(oldPos.z == myPos.z)
			updateTileCache(oldTile, oldPos);
	}

	if(creature == followCreature || (creature == this && followCreature))
	{
		if(hasFollowPath)
		{
			isUpdatingPath = true;
			Dispatcher::getInstance().addTask(createTask(
				boost::bind(&Game::updateCreatureWalk, &g_game, getID())));
		}

		if(newPos.z != oldPos.z || !canSee(followCreature->getPosition()))
			internalCreatureDisappear(followCreature, false);
	}

	if(creature == attackedCreature || (creature == this && attackedCreature))
	{
		if(newPos.z == oldPos.z && canSee(attackedCreature->getPosition()))
		{
			if(hasExtraSwing()) //our target is moving lets see if we can get in hit
				Dispatcher::getInstance().addTask(createTask(
					boost::bind(&Game::checkCreatureAttack, &g_game, getID())));

			if(newTile->getZone() != oldTile->getZone())
				onTargetChangeZone(attackedCreature->getZone());
		}
		else
			internalCreatureDisappear(attackedCreature, false);
	}
}

bool Creature::onDeath()
{
	DeathList deathList = getKillers();
	bool deny = false;

	CreatureEventList prepareDeathEvents = getCreatureEvents(CREATURE_EVENT_PREPAREDEATH);
	for(CreatureEventList::iterator it = prepareDeathEvents.begin(); it != prepareDeathEvents.end(); ++it)
	{
		if(!(*it)->executePrepareDeath(this, deathList) && !deny)
			deny = true;
	}

	if(deny)
		return false;

	int32_t i = 0, size = deathList.size(), limit = g_config.getNumber(ConfigManager::DEATH_ASSISTS) + 1;
	if(limit > 0 && size > limit)
		size = limit;

	Creature* tmp = NULL;
	CreatureVector justifyVec;
	for(DeathList::iterator it = deathList.begin(); it != deathList.end(); ++it, ++i)
	{
		if(it->isNameKill())
			continue;

		if(it == deathList.begin())
			it->setLast();

		if(i < size)
		{
			if(it->getKillerCreature()->getPlayer())
				tmp = it->getKillerCreature();
			else if(it->getKillerCreature()->getPlayerMaster())
				tmp = it->getKillerCreature()->getMaster();
		}

		if(tmp)
		{
			if(std::find(justifyVec.begin(), justifyVec.end(), tmp) == justifyVec.end())
			{
				it->setJustify();
				justifyVec.push_back(tmp);
			}

			tmp = NULL;
		}

		if(!it->getKillerCreature()->onKilledCreature(this, (*it)) && it->isLast())
			return false;
	}

	for(CountMap::iterator it = damageMap.begin(); it != damageMap.end(); ++it)
	{
		if((tmp = g_game.getCreatureByID(it->first)))
			tmp->onTargetKilled(this);
	}

	dropCorpse(deathList);
	if(master)
		master->removeSummon(this);

	return true;
}

void Creature::dropCorpse(DeathList deathList)
{
	if(master)
	{
		g_game.addMagicEffect(getPosition(), MAGIC_EFFECT_POFF);
		return;
	}

	Item* corpse = createCorpse(deathList);
	if(corpse)
		corpse->setParent(VirtualCylinder::virtualCylinder);

	bool deny = false;
	CreatureEventList deathEvents = getCreatureEvents(CREATURE_EVENT_DEATH);
	for(CreatureEventList::iterator it = deathEvents.begin(); it != deathEvents.end(); ++it)
	{
		if(!(*it)->executeDeath(this, corpse, deathList) && !deny)
			deny = true;
	}

	if(!corpse)
		return;

	corpse->setParent(NULL);
	if(deny)
		return;

	Tile* tile = getTile();
	if(!tile)
		return;

	Item* splash = NULL;
	switch(getRace())
	{
		case RACE_VENOM:
			splash = Item::CreateItem(ITEM_FULLSPLASH, FLUID_GREEN);
			break;

		case RACE_BLOOD:
			splash = Item::CreateItem(ITEM_FULLSPLASH, FLUID_BLOOD);
			break;

		default:
			break;
	}

	if(splash)
	{
		g_game.internalAddItem(NULL, tile, splash, INDEX_WHEREEVER, FLAG_NOLIMIT);
		g_game.startDecay(splash);
	}

	g_game.internalAddItem(NULL, tile, corpse, INDEX_WHEREEVER, FLAG_NOLIMIT);
	dropLoot(corpse->getContainer());
	g_game.startDecay(corpse);
}

DeathList Creature::getKillers()
{
	DeathList list;
	CountMap::const_iterator it;

	Creature* lhc = NULL;
	if((lhc = g_game.getCreatureByID(lastHitCreature)))
	{
		int32_t damage = 0;
		if((it = damageMap.find(lastHitCreature)) != damageMap.end())
			damage = it->second.total;

		list.push_back(DeathEntry(lhc, damage));
	}
	else
		list.push_back(DeathEntry(getCombatName(lastDamageSource), 0));

	int32_t requiredTime = g_config.getNumber(ConfigManager::DEATHLIST_REQUIRED_TIME);
	int64_t now = OTSYS_TIME();

	CountBlock_t cb;
	for(it = damageMap.begin(); it != damageMap.end(); ++it)
	{
		cb = it->second;
		if((now - cb.ticks) > requiredTime)
			continue;

		Creature* mdc = g_game.getCreatureByID(it->first);
		if(!mdc || mdc == lhc || (lhc && (mdc->getMaster() == lhc || lhc->getMaster() == mdc)))
			continue;

		bool deny = false;
		for(DeathList::iterator fit = list.begin(); fit != list.end(); ++fit)
		{
			if(fit->isNameKill())
				continue;

			Creature* tmp = fit->getKillerCreature();
			if(!(mdc->getName() == tmp->getName() && mdc->getMaster() == tmp->getMaster()) &&
				(!mdc->getMaster() || (mdc->getMaster() != tmp && mdc->getMaster() != tmp->getMaster()))
				&& (mdc->getSummonCount() <= 0 || tmp->getMaster() != mdc))
				continue;

			deny = true;
			break;
		}

		if(!deny)
			list.push_back(DeathEntry(mdc, cb.total));
	}

	if(list.size() > 1)
		std::sort(list.begin() + 1, list.end(), DeathLessThan());

	return list;
}

bool Creature::hasBeenAttacked(uint32_t attackerId) const
{
	CountMap::const_iterator it = damageMap.find(attackerId);
	if(it != damageMap.end())
		return (OTSYS_TIME() - it->second.ticks) <= g_config.getNumber(ConfigManager::PZ_LOCKED);

	return false;
}

Item* Creature::createCorpse(DeathList)
{
	return Item::CreateItem(getLookCorpse());
}

void Creature::changeHealth(int32_t healthChange)
{
	if(healthChange > 0)
		health += std::min(healthChange, getMaxHealth() - health);
	else
		health = std::max((int32_t)0, health + healthChange);

	g_game.addCreatureHealth(this);
}

void Creature::changeMana(int32_t manaChange)
{
	if(manaChange > 0)
		mana += std::min(manaChange, getMaxMana() - mana);
	else
		mana = std::max((int32_t)0, mana + manaChange);
}

bool Creature::getStorage(const std::string& key, std::string& value) const
{
	StorageMap::const_iterator it = storageMap.find(key);
	if(it != storageMap.end())
	{
		value = it->second;
		return true;
	}

	value = "-1";
	return false;
}

bool Creature::setStorage(const std::string& key, const std::string& value)
{
	storageMap[key] = value;
	return true;
}

void Creature::gainHealth(Creature* caster, int32_t healthGain)
{
	if(healthGain > 0)
	{
		int32_t prevHealth = getHealth();
		changeHealth(healthGain);

		int32_t effectiveGain = getHealth() - prevHealth;
		if(caster)
			caster->onTargetGainHealth(this, effectiveGain);
	}
	else
		changeHealth(healthGain);
}

void Creature::drainHealth(Creature* attacker, CombatType_t combatType, int32_t damage)
{
	lastDamageSource = combatType;
	onAttacked();

	changeHealth(-damage);
	if(attacker)
		attacker->onTargetDrainHealth(this, damage);
}

void Creature::drainMana(Creature* attacker, CombatType_t combatType, int32_t damage)
{
	lastDamageSource = combatType;
	onAttacked();

	changeMana(-damage);
	if(attacker)
		attacker->onTargetDrainMana(this, damage);
}

BlockType_t Creature::blockHit(Creature* attacker, CombatType_t combatType, int32_t& damage,
	bool checkDefense/* = false*/, bool checkArmor/* = false*/, bool/* reflect = true*/, bool/* field = false*/, bool/* element = false*/)
{
	BlockType_t blockType = BLOCK_NONE;
	if(isImmune(combatType))
	{
		damage = 0;
		blockType = BLOCK_IMMUNITY;
	}
	else if(checkDefense || checkArmor)
	{
		bool hasDefense = false;
		if(blockCount > 0)
		{
			--blockCount;
			hasDefense = true;
		}

		if(checkDefense && hasDefense)
		{
			int32_t maxDefense = getDefense(), minDefense = maxDefense / 2;
			damage -= random_range(minDefense, maxDefense);
			if(damage <= 0)
			{
				damage = 0;
				blockType = BLOCK_DEFENSE;
				checkArmor = false;
			}
		}

		if(checkArmor)
		{
			int32_t armorValue = getArmor(), minArmorReduction = 0,
				maxArmorReduction = 0;
			if(armorValue > 1)
			{
				minArmorReduction = (int32_t)std::ceil(armorValue * 0.475);
				maxArmorReduction = (int32_t)std::ceil(
					((armorValue * 0.475) - 1) + minArmorReduction);
			}
			else if(armorValue == 1)
			{
				minArmorReduction = 1;
				maxArmorReduction = 1;
			}

			damage -= random_range(minArmorReduction, maxArmorReduction);
			if(damage <= 0)
			{
				damage = 0;
				blockType = BLOCK_ARMOR;
			}
		}

		if(hasDefense && blockType != BLOCK_NONE)
			onBlockHit(blockType);
	}

	if(attacker)
	{
		attacker->onTarget(this);
		attacker->onTargetBlockHit(this, blockType);
	}

	onAttacked();
	return blockType;
}

bool Creature::setAttackedCreature(Creature* creature)
{
	if(creature)
	{
		const Position& creaturePos = creature->getPosition();
		if(creaturePos.z != getPosition().z || !canSee(creaturePos))
		{
			attackedCreature = NULL;
			return false;
		}
	}

	attackedCreature = creature;
	if(attackedCreature)
	{
		onTarget(attackedCreature);
		attackedCreature->onAttacked();
	}

	for(std::list<Creature*>::iterator cit = summons.begin(); cit != summons.end(); ++cit)
		(*cit)->setAttackedCreature(creature);

	Condition* condition = getCondition(CONDITION_LOGINPROTECTION, CONDITIONID_DEFAULT);
	if(condition)
		removeCondition(condition);

	return true;
}

void Creature::getPathSearchParams(const Creature*, FindPathParams& fpp) const
{
	fpp.fullPathSearch = !hasFollowPath;
	fpp.clearSight = true;
	fpp.maxSearchDist = 12;
	fpp.minTargetDist = fpp.maxTargetDist = 1;
}

void Creature::goToFollowCreature()
{
	if(getPlayer() && (OTSYS_TIME() - lastFailedFollow <= g_config.getNumber(ConfigManager::FOLLOW_EXHAUST)))
		return;

	if(followCreature)
	{
		FindPathParams fpp;
		getPathSearchParams(followCreature, fpp);
		if(g_game.getPathToEx(this, followCreature->getPosition(), listWalkDir, fpp))
		{
			hasFollowPath = true;
			startAutoWalk(listWalkDir);
		}
		else
		{
			hasFollowPath = false;
			lastFailedFollow = OTSYS_TIME();
		}
	}

	onFollowCreatureComplete(followCreature);
}

bool Creature::setFollowCreature(Creature* creature, bool /*fullPathSearch = false*/)
{
	if(creature)
	{
		if(followCreature == creature)
			return true;

		const Position& creaturePos = creature->getPosition();
		if(creaturePos.z != getPosition().z || !canSee(creaturePos))
		{
			followCreature = NULL;
			return false;
		}

		if(!listWalkDir.empty())
		{
			listWalkDir.clear();
			onWalkAborted();
		}

		hasFollowPath = forceUpdateFollowPath = false;
		followCreature = creature;
		isUpdatingPath = true;
	}
	else
	{
		isUpdatingPath = false;
		followCreature = NULL;
	}

	//g_game.updateCreatureWalk(id);
	onFollowCreature(creature);
	return true;
}

double Creature::getDamageRatio(Creature* attacker) const
{
	double totalDamage = 0, attackerDamage = 0;
	for(CountMap::const_iterator it = damageMap.begin(); it != damageMap.end(); ++it)
	{
		totalDamage += it->second.total;
		if(it->first == attacker->getID())
			attackerDamage += it->second.total;
	}

	return (totalDamage ? attackerDamage / totalDamage : 0);
}

double Creature::getGainedExperience(Creature* attacker) const
{
	return getDamageRatio(attacker) * (double)getLostExperience();
}

void Creature::addDamagePoints(Creature* attacker, int32_t damagePoints)
{
	if(damagePoints < 0)
		return;

	uint32_t attackerId = 0;
	if(attacker)
		attackerId = attacker->getID();

	CountMap::iterator it = damageMap.find(attackerId);
	if(it != damageMap.end())
	{
		it->second.ticks = OTSYS_TIME();
		if(damagePoints > 0)
			it->second.total += damagePoints;
	}
	else
		damageMap[attackerId] = CountBlock_t(damagePoints);

	if(damagePoints > 0)
		lastHitCreature = attackerId;
}

void Creature::addHealPoints(Creature* caster, int32_t healthPoints)
{
	if(healthPoints <= 0)
		return;

	uint32_t casterId = 0;
	if(caster)
		casterId = caster->getID();

	CountMap::iterator it = healMap.find(casterId);
	if(it != healMap.end())
	{
		it->second.ticks = OTSYS_TIME();
		it->second.total += healthPoints;
	}
	else
		healMap[casterId] = CountBlock_t(healthPoints);
}

void Creature::onAddCondition(ConditionType_t type, bool hadCondition)
{
	switch(type)
	{
		case CONDITION_INVISIBLE:
		{
			if(!hadCondition)
				g_game.internalCreatureChangeVisible(this, VISIBLE_DISAPPEAR);

			break;
		}

		case CONDITION_PARALYZE:
		{
			if(hasCondition(CONDITION_HASTE, -1, false))
				removeCondition(CONDITION_HASTE);

			break;
		}

		case CONDITION_HASTE:
		{
			if(hasCondition(CONDITION_PARALYZE, -1, false))
				removeCondition(CONDITION_PARALYZE);

			break;
		}

		default:
			break;
	}
}

void Creature::onEndCondition(ConditionType_t type, ConditionId_t)
{
	if(type == CONDITION_INVISIBLE && !hasCondition(CONDITION_INVISIBLE, -1, false))
		g_game.internalCreatureChangeVisible(this, VISIBLE_APPEAR);
}

void Creature::onTickCondition(ConditionType_t type, ConditionId_t, int32_t, bool& _remove)
{
	if(const MagicField* field = getTile()->getFieldItem())
	{
		switch(type)
		{
			case CONDITION_FIRE:
				_remove = field->getCombatType() != COMBAT_FIREDAMAGE;
				break;
			case CONDITION_ENERGY:
				_remove = field->getCombatType() != COMBAT_ENERGYDAMAGE;
				break;
			case CONDITION_POISON:
				_remove = field->getCombatType() != COMBAT_EARTHDAMAGE;
				break;
			case CONDITION_FREEZING:
				_remove = field->getCombatType() != COMBAT_ICEDAMAGE;
				break;
			case CONDITION_DAZZLED:
				_remove = field->getCombatType() != COMBAT_HOLYDAMAGE;
				break;
			case CONDITION_CURSED:
				_remove = field->getCombatType() != COMBAT_DEATHDAMAGE;
				break;
			case CONDITION_DROWN:
				_remove = field->getCombatType() != COMBAT_DROWNDAMAGE;
				break;
			case CONDITION_BLEEDING:
				_remove = field->getCombatType() != COMBAT_PHYSICALDAMAGE;
				break;
			default:
				break;
		}
	}
}

void Creature::onCombatRemoveCondition(const Creature*, Condition* condition)
{
	removeCondition(condition);
}

void Creature::onIdleStatus()
{
	if(getHealth() > 0)
	{
		healMap.clear();
		damageMap.clear();
	}
}

void Creature::onTargetDrainHealth(Creature* target, int32_t points)
{
	onTargetDrain(target, points);
}

void Creature::onTargetDrainMana(Creature* target, int32_t points)
{
	onTargetDrain(target, points);
}

void Creature::onTargetDrain(Creature* target, int32_t points)
{
	if(points >= 0)
		target->addDamagePoints(this, points);
}

void Creature::onTargetGainHealth(Creature* target, int32_t points)
{
	onTargetGain(target, points);
}

void Creature::onTargetGainMana(Creature* target, int32_t points)
{
	onTargetGain(target, points);
}

void Creature::onTargetGain(Creature* target, int32_t points)
{
	if(points > 0)
		target->addHealPoints(this, points);
}

void Creature::onTargetKilled(Creature* target)
{
	if(target == this)
		return;

	double exp = target->getGainedExperience(this);
	onGainExperience(exp, target, false);
}

bool Creature::onKilledCreature(Creature* target, DeathEntry& entry)
{
	bool ret = true;
	if(master)
		ret = master->onKilledCreature(target, entry);

	CreatureEventList killEvents = getCreatureEvents(CREATURE_EVENT_KILL);
	if(!entry.isLast())
	{
		for(CreatureEventList::iterator it = killEvents.begin(); it != killEvents.end(); ++it)
			(*it)->executeKill(this, target, entry);

		return true;
	}

	for(CreatureEventList::iterator it = killEvents.begin(); it != killEvents.end(); ++it)
	{
		if(!(*it)->executeKill(this, target, entry) && ret)
			ret = false;
	}

	return ret;
}

void Creature::onGainExperience(double& gainExp, Creature* target, bool multiplied)
{
	if(gainExp <= 0)
		return;

	if(master)
	{
		gainExp = gainExp / 2;
		master->onGainExperience(gainExp, target, multiplied);
	}
	else if(!multiplied)
		gainExp *= g_config.getDouble(ConfigManager::RATE_EXPERIENCE);

	int16_t color = g_config.getNumber(ConfigManager::EXPERIENCE_COLOR);
	if(color < 0)
		color = random_range(0, 255);

	const Position& targetPos = getPosition();

	SpectatorVec list;
	g_game.getSpectators(list, targetPos, false, true, Map::maxViewportX, Map::maxViewportX,
		Map::maxViewportY, Map::maxViewportY);

	std::ostringstream ss;
	ss << ucfirst(getNameDescription()) << " gained " << (uint64_t)gainExp << " experience points.";

	SpectatorVec textList;
	for (Creature* spectator : list) {
		if (spectator != this)
			textList.insert(spectator);
	}

	MessageDetails* details = new MessageDetails((int32_t)gainExp, (Color_t)color);
	g_game.addStatsMessage(textList, MSG_EXPERIENCE_OTHERS, ss.str(), targetPos, details);
	if(Player* player = getPlayer())
	{
		ss.str("");
		ss << "You gained " << (uint64_t)gainExp << " experience points.";
		player->sendStatsMessage(MSG_EXPERIENCE, ss.str(), targetPos, details);
	}

	delete details;
}

void Creature::onGainSharedExperience(double& gainExp, Creature* target, bool multiplied)
{
	if(gainExp <= 0)
		return;

	if(master)
	{
		gainExp = gainExp / 2;
		master->onGainSharedExperience(gainExp, target, multiplied);
	}
	else if(!multiplied)
		gainExp *= g_config.getDouble(ConfigManager::RATE_EXPERIENCE);

	int16_t color = g_config.getNumber(ConfigManager::EXPERIENCE_COLOR);
	if(color < 0)
		color = random_range(0, 255);

	const Position& targetPos = getPosition();

	SpectatorVec list;
	g_game.getSpectators(list, targetPos, false, true, Map::maxViewportX, Map::maxViewportX,
		Map::maxViewportY, Map::maxViewportY);

	std::ostringstream ss;
	ss << ucfirst(getNameDescription()) << " gained " << (uint64_t)gainExp << " experience points.";

	SpectatorVec textList;
	for (Creature* spectator : list) {
		if (spectator != this)
			textList.insert(spectator);
	}

	MessageDetails* details = new MessageDetails((int32_t)gainExp, (Color_t)color);
	g_game.addStatsMessage(textList, MSG_EXPERIENCE_OTHERS, ss.str(), targetPos, details);
	if(Player* player = getPlayer())
	{
		ss.str("");
		ss << "You gained " << (uint64_t)gainExp << " experience points.";
		player->sendStatsMessage(MSG_EXPERIENCE, ss.str(), targetPos, details);
	}

	delete details;
}

void Creature::addSummon(Creature* creature)
{
	creature->setDropLoot(LOOT_DROP_NONE);
	creature->setLossSkill(false);

	creature->setMaster(this);
	creature->addRef();
	summons.push_back(creature);
}

void Creature::removeSummon(const Creature* creature)
{
	std::list<Creature*>::iterator it = std::find(summons.begin(), summons.end(), creature);
	if(it == summons.end())
		return;

	(*it)->setMaster(NULL);
	(*it)->unRef();
	summons.erase(it);
}

void Creature::destroySummons()
{
	for(std::list<Creature*>::iterator it = summons.begin(); it != summons.end(); ++it)
	{
		(*it)->setAttackedCreature(NULL);
		(*it)->changeHealth(-(*it)->getHealth());

		(*it)->setMaster(NULL);
		(*it)->unRef();
	}

	summons.clear();
}

bool Creature::addCondition(Condition* condition)
{
	if(!condition)
		return false;

	bool hadCondition = hasCondition(condition->getType(), -1, false);
	if(Condition* previous = getCondition(condition->getType(), condition->getId(), condition->getSubId()))
	{
		previous->addCondition(this, condition);
		delete condition;
		return true;
	}

	if(condition->startCondition(this))
	{
		conditions.push_back(condition);
		onAddCondition(condition->getType(), hadCondition);
		return true;
	}

	delete condition;
	return false;
}

bool Creature::addCombatCondition(Condition* condition)
{
	bool hadCondition = hasCondition(condition->getType(), -1, false);
	//Caution: condition variable could be deleted after the call to addCondition
	ConditionType_t type = condition->getType();
	if(!addCondition(condition))
		return false;

	onAddCombatCondition(type, hadCondition);
	return true;
}

void Creature::removeCondition(ConditionType_t type)
{
	for(ConditionList::iterator it = conditions.begin(); it != conditions.end(); )
	{
		if((*it)->getType() != type)
		{
			++it;
			continue;
		}

		Condition* condition = *it;
		it = conditions.erase(it);

		condition->endCondition(this, CONDITIONEND_ABORT);
		onEndCondition(condition->getType(), condition->getId());
		delete condition;
	}
}

void Creature::removeCondition(ConditionType_t type, ConditionId_t conditionId)
{
	for(ConditionList::iterator it = conditions.begin(); it != conditions.end(); )
	{
		if((*it)->getType() != type || (*it)->getId() != conditionId)
		{
			++it;
			continue;
		}

		Condition* condition = *it;
		it = conditions.erase(it);

		condition->endCondition(this, CONDITIONEND_ABORT);
		onEndCondition(condition->getType(), condition->getId());
		delete condition;
	}
}

void Creature::removeCondition(Condition* condition)
{
	ConditionList::iterator it = std::find(conditions.begin(), conditions.end(), condition);
	if(it != conditions.end())
	{
		Condition* condition = *it;
		it = conditions.erase(it);

		condition->endCondition(this, CONDITIONEND_ABORT);
		onEndCondition(condition->getType(), condition->getId());
		delete condition;
	}
}

void Creature::removeCondition(const Creature* attacker, ConditionType_t type)
{
	ConditionList tmpList = conditions;
	for(ConditionList::iterator it = tmpList.begin(); it != tmpList.end(); ++it)
	{
		if((*it)->getType() == type)
			onCombatRemoveCondition(attacker, *it);
	}
}

void Creature::removeConditions(ConditionEnd_t reason, bool onlyPersistent/* = true*/)
{
	for(ConditionList::iterator it = conditions.begin(); it != conditions.end(); )
	{
		if(onlyPersistent && !(*it)->isPersistent())
		{
			++it;
			continue;
		}

		Condition* condition = *it;
		it = conditions.erase(it);

		condition->endCondition(this, reason);
		onEndCondition(condition->getType(), condition->getId());
		delete condition;
	}
}

Condition* Creature::getCondition(ConditionType_t type, ConditionId_t conditionId, uint32_t subId/* = 0*/) const
{
	for(ConditionList::const_iterator it = conditions.begin(); it != conditions.end(); ++it)
	{
		if((*it)->getType() == type && (*it)->getId() == conditionId && (*it)->getSubId() == subId)
			return *it;
	}

	return NULL;
}

void Creature::executeConditions(uint32_t interval)
{
	for(ConditionList::iterator it = conditions.begin(); it != conditions.end(); )
	{
		if((*it)->executeCondition(this, interval))
		{
			++it;
			continue;
		}

		Condition* condition = *it;
		it = conditions.erase(it);

		condition->endCondition(this, CONDITIONEND_TICKS);
		onEndCondition(condition->getType(), condition->getId());
		delete condition;
	}
}

bool Creature::hasCondition(ConditionType_t type, int32_t subId/* = 0*/, bool checkTime/* = true*/) const
{
	if(isSuppress(type))
		return false;

	Condition* condition = NULL;
	for(ConditionList::const_iterator it = conditions.begin(); it != conditions.end(); ++it)
	{
		if(!(condition = *it) || condition->getType() != type ||
			(subId != -1 && condition->getSubId() != (uint32_t)subId))
			continue;

		if(!checkTime || !condition->getEndTime() || condition->getEndTime() >= OTSYS_TIME())
			return true;
	}

	return false;
}

bool Creature::isImmune(CombatType_t type) const
{
	return ((getDamageImmunities() & (uint32_t)type) == (uint32_t)type);
}

bool Creature::isImmune(ConditionType_t type) const
{
	return ((getConditionImmunities() & (uint32_t)type) == (uint32_t)type);
}

bool Creature::isSuppress(ConditionType_t type) const
{
	return ((getConditionSuppressions() & (uint32_t)type) == (uint32_t)type);
}

std::string Creature::getDescription(int32_t) const
{
	return "a creature";
}

int64_t Creature::getStepDuration(Direction dir) const
{
	if(dir == NORTHWEST || dir == NORTHEAST || dir == SOUTHWEST || dir == SOUTHEAST)
		return getStepDuration() << 1;

	return getStepDuration();
}

int64_t Creature::getStepDuration() const
{
	if(removed)
		return 0;

	uint32_t stepSpeed = getStepSpeed();
	if(!stepSpeed)
		return 0;

	const Tile* tile = getTile();
	if(!tile || !tile->ground)
		return 0;

	return ((1000 * Item::items[tile->ground->getID()].speed) / stepSpeed) * lastStepCost;
}

int64_t Creature::getEventStepTicks(bool onlyDelay/* = false*/) const
{
	int64_t ret = getWalkDelay();
	if(ret > 0)
		return ret;

	if(!onlyDelay)
		return getStepDuration();

	return 1;
}

void Creature::getCreatureLight(LightInfo& light) const
{
	light = internalLight;
}

void Creature::resetLight()
{
	internalLight.level = internalLight.color = 0;
}

bool Creature::registerCreatureEvent(const std::string& name)
{
	CreatureEvent* event = g_creatureEvents->getEventByName(name);
	if(!event || !event->isLoaded()) //check for existance
		return false;

	for(CreatureEventList::iterator it = eventsList.begin(); it != eventsList.end(); ++it)
	{
		if((*it) == event) //do not allow registration of same event more than once
			return false;
	}

	eventsList.push_back(event);
	return true;
}

bool Creature::unregisterCreatureEvent(const std::string& name)
{
	CreatureEvent* event = g_creatureEvents->getEventByName(name);
	if(!event || !event->isLoaded()) //check for existance
		return false;

	for(CreatureEventList::iterator it = eventsList.begin(); it != eventsList.end(); ++it)
	{
		if((*it) != event)
			continue;

		eventsList.erase(it);
		return true; // we shouldn't have a duplicate
	}

	return false;
}

void Creature::unregisterCreatureEvent(CreatureEventType_t type)
{
	for(CreatureEventList::iterator it = eventsList.begin(); it != eventsList.end(); ++it)
	{
		if((*it)->getEventType() == type)
			it = eventsList.erase(it);
	}
}

CreatureEventList Creature::getCreatureEvents(CreatureEventType_t type)
{
	CreatureEventList list;
	for(CreatureEventList::iterator it = eventsList.begin(); it != eventsList.end(); ++it)
	{
		if((*it)->getEventType() == type && (*it)->isLoaded())
			list.push_back(*it);
	}

	return list;
}

FrozenPathingConditionCall::FrozenPathingConditionCall(const Position& _targetPos)
{
	targetPos = _targetPos;
}

bool FrozenPathingConditionCall::isInRange(const Position& startPos, const Position& testPos,
	const FindPathParams& fpp) const
{
	if (fpp.fullPathSearch) {
		if (testPos.x > targetPos.x + fpp.maxTargetDist) {
			return false;
		}

		if (testPos.x < targetPos.x - fpp.maxTargetDist) {
			return false;
		}

		if (testPos.y > targetPos.y + fpp.maxTargetDist) {
			return false;
		}

		if (testPos.y < targetPos.y - fpp.maxTargetDist) {
			return false;
		}
	}
	else {
		int_fast32_t dx = Position::getOffsetX(startPos, targetPos);

		int32_t dxMax = (dx >= 0 ? fpp.maxTargetDist : 0);
		if (testPos.x > targetPos.x + dxMax) {
			return false;
		}

		int32_t dxMin = (dx <= 0 ? fpp.maxTargetDist : 0);
		if (testPos.x < targetPos.x - dxMin) {
			return false;
		}

		int_fast32_t dy = Position::getOffsetY(startPos, targetPos);

		int32_t dyMax = (dy >= 0 ? fpp.maxTargetDist : 0);
		if (testPos.y > targetPos.y + dyMax) {
			return false;
		}

		int32_t dyMin = (dy <= 0 ? fpp.maxTargetDist : 0);
		if (testPos.y < targetPos.y - dyMin) {
			return false;
		}
	}
	return true;
}

bool FrozenPathingConditionCall::operator()(const Position& startPos, const Position& testPos,
	const FindPathParams& fpp, int32_t& bestMatchDist) const
{
	if (!isInRange(startPos, testPos, fpp)) {
		return false;
	}

	if (fpp.clearSight && !g_game.isSightClear(testPos, targetPos, true)) {
		return false;
	}

	int32_t testDist = std::max<int32_t>(Position::getDistanceX(targetPos, testPos), Position::getDistanceY(targetPos, testPos));
	if (fpp.maxTargetDist == 1) {
		if (testDist < fpp.minTargetDist || testDist > fpp.maxTargetDist) {
			return false;
		}

		return true;
	}
	else if (testDist <= fpp.maxTargetDist) {
		if (testDist < fpp.minTargetDist) {
			return false;
		}

		if (testDist == fpp.maxTargetDist) {
			bestMatchDist = 0;
			return true;
		}
		else if (testDist > bestMatchDist) {
			//not quite what we want, but the best so far
			bestMatchDist = testDist;
			return true;
		}
	}
	return false;
}
