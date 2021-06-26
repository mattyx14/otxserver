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

#ifndef __CREATURE__
#define __CREATURE__
#include "otsystem.h"

#include "templates.h"
#include <boost/any.hpp>

#include "const.h"
#include "enums.h"

#include "map.h"
#include "condition.h"
#include "creatureevent.h"

enum slots_t
{
	SLOT_PRE_FIRST = 0,
	SLOT_WHEREEVER = SLOT_PRE_FIRST,
	SLOT_FIRST = 1,
	SLOT_HEAD = SLOT_FIRST,
	SLOT_NECKLACE = 2,
	SLOT_BACKPACK = 3,
	SLOT_ARMOR = 4,
	SLOT_RIGHT = 5,
	SLOT_LEFT = 6,
	SLOT_LEGS = 7,
	SLOT_FEET = 8,
	SLOT_RING = 9,
	SLOT_AMMO = 10,
	SLOT_DEPOT = 11,
	SLOT_LAST = SLOT_DEPOT,
	SLOT_HAND = 12,
	SLOT_TWO_HAND = SLOT_HAND
};

enum lootDrop_t
{
	LOOT_DROP_FULL = 0,
	LOOT_DROP_PREVENT,
	LOOT_DROP_NONE
};

enum Visible_t
{
	VISIBLE_NONE = 0,
	VISIBLE_APPEAR = 1,
	VISIBLE_DISAPPEAR = 2,
	VISIBLE_GHOST_APPEAR = 3,
	VISIBLE_GHOST_DISAPPEAR = 4
};

struct FindPathParams
{
	bool fullPathSearch, clearSight, allowDiagonal, keepDistance;
	uint16_t maxClosedNodes;
	int32_t maxSearchDist, minTargetDist, maxTargetDist;
	FindPathParams()
	{
		fullPathSearch = clearSight = allowDiagonal = true;
		keepDistance = false;
		maxClosedNodes = 100;
		maxSearchDist = minTargetDist = maxTargetDist = -1;
	}
};

struct DeathLessThan;
struct DeathEntry
{
		DeathEntry(std::string name, int32_t dmg):
			data(name), damage(dmg), last(false), justify(false), unjustified(false) {}
		DeathEntry(Creature* killer, int32_t dmg):
			data(killer), damage(dmg), last(false), justify(false), unjustified(false) {}

		bool isCreatureKill() const {return data.type() == typeid(Creature*);}
		bool isNameKill() const {return !isCreatureKill();}

		void setWar(War_t v) {war = v;}
		War_t getWar() const {return war;}

		void setLast() {last = true;}
		bool isLast() const {return last;}

		void setJustify() {justify = true;}
		bool isJustify() const {return justify;}

		void setUnjustified() {unjustified = true;}
		bool isUnjustified() const {return unjustified;}

		const std::type_info& getKillerType() const {return data.type();}
		int32_t getDamage() const {return damage;}

		Creature* getKillerCreature() const {return boost::any_cast<Creature*>(data);}
		std::string getKillerName() const {return boost::any_cast<std::string>(data);}

	protected:
		friend struct DeathLessThan;

		boost::any data;
		int32_t damage;
		War_t war;

		bool last;
		bool justify;
		bool unjustified;
};

struct DeathLessThan
{
	bool operator()(const DeathEntry& d1, const DeathEntry& d2) {return d1.damage > d2.damage;}
};

typedef std::vector<DeathEntry> DeathList;
typedef std::list<CreatureEvent*> CreatureEventList;
typedef std::list<Condition*> ConditionList;
typedef std::map<std::string, std::string> StorageMap;

class Map;
class Tile;
class Thing;

class Player;
class Monster;
class Npc;

class Item;
class Container;

#define EVENT_CREATURECOUNT 10
#ifndef __GROUPED_ATTACKS__
#define EVENT_CREATURE_THINK_INTERVAL 1000
#else
#define EVENT_CREATURE_THINK_INTERVAL 500
#endif
#define EVENT_CHECK_CREATURE_INTERVAL (EVENT_CREATURE_THINK_INTERVAL / EVENT_CREATURECOUNT)

class FrozenPathingConditionCall
{
	public:
		FrozenPathingConditionCall(const Position& _targetPos);
		virtual ~FrozenPathingConditionCall() {}

		virtual bool operator()(const Position& startPos, const Position& testPos,
			const FindPathParams& fpp, int32_t& bestMatchDist) const;

		bool isInRange(const Position& startPos, const Position& testPos,
			const FindPathParams& fpp) const;

	protected:
		Position targetPos;
};

class Creature : public AutoId, virtual public Thing
{
	protected:
		Creature();

	public:
		virtual ~Creature();

		virtual Creature* getCreature() {return this;}
		virtual const Creature* getCreature() const {return this;}
		virtual Player* getPlayer() {return NULL;}
		virtual const Player* getPlayer() const {return NULL;}
		virtual Npc* getNpc() {return NULL;}
		virtual const Npc* getNpc() const {return NULL;}
		virtual Monster* getMonster() {return NULL;}
		virtual const Monster* getMonster() const {return NULL;}
		virtual CreatureType_t getType() const = 0;

		virtual const std::string& getName() const = 0;
		virtual const std::string& getNameDescription() const = 0;
		virtual std::string getDescription(int32_t lookDistance) const;

		uint32_t getID() const {return id;}
		virtual void setID() = 0;

		void setRemoved() {removed = true;}
		virtual bool isRemoved() const {return removed;}

		virtual uint32_t rangeId() = 0;
		virtual void removeList() = 0;
		virtual void addList() = 0;

		virtual bool canSee(const Position& pos) const;
		virtual bool canSeeCreature(const Creature* creature) const;
		virtual bool canWalkthrough(const Creature* creature) const;

		Direction getDirection() const {return direction;}
		void setDirection(Direction dir) {direction = dir;}

		bool getHideName() const {return hideName;}
		void setHideName(bool v) {hideName = v;}

		bool getHideHealth() const {return hideHealth;}
		void setHideHealth(bool v) {hideHealth = v;}

		MessageClasses getSpeakType() const {return speakType;}
		void setSpeakType(MessageClasses type) {speakType = type;}

		Position getMasterPosition() const {return masterPosition;}
		void setMasterPosition(const Position& pos, uint32_t radius = 1) {masterPosition = pos; masterRadius = radius;}

		virtual int32_t getThrowRange() const {return 1;}
		virtual RaceType_t getRace() const {return RACE_NONE;}

		virtual bool isPushable() const {return getWalkDelay() <= 0;}
		virtual bool canSeeInvisibility() const {return false;}

		int32_t getWalkDelay(Direction dir) const;
		int32_t getWalkDelay() const;
		int64_t getStepDuration(Direction dir) const;
		int64_t getStepDuration() const;

		int64_t getEventStepTicks(bool onlyDelay = false) const;
		int64_t getTimeSinceLastMove() const;
		virtual int32_t getStepSpeed() const {return getSpeed();}

		int32_t getSpeed() const {return baseSpeed + varSpeed;}
		void setSpeed(int32_t varSpeedDelta)
		{
			int32_t oldSpeed = getSpeed();
			varSpeed = varSpeedDelta;
			if(getSpeed() <= 0)
			{
				stopEventWalk();
				cancelNextWalk = true;
			}
			else if(oldSpeed <= 0 && !listWalkDir.empty())
				addEventWalk();
		}

		void setBaseSpeed(uint32_t newBaseSpeed) {baseSpeed = newBaseSpeed;}
		int32_t getBaseSpeed() {return baseSpeed;}

		virtual int32_t getHealth() const {return health;}
		virtual int32_t getMaxHealth() const {return healthMax;}
		virtual int32_t getMana() const {return mana;}
		virtual int32_t getMaxMana() const {return manaMax;}

		const Outfit_t getCurrentOutfit() const {return currentOutfit;}
		void setCurrentOutfit(Outfit_t outfit) {currentOutfit = outfit;}
		const Outfit_t getDefaultOutfit() const {return defaultOutfit;}

		bool isInvisible() const {return hasCondition(CONDITION_INVISIBLE, -1, false);}
		virtual bool isGhost() const {return false;}
		virtual bool isWalkable() const {return false;}

		ZoneType_t getZone() const {return getTile()->getZone();}

		//walk functions
		bool startAutoWalk(std::list<Direction>& listDir);
		void stopWalk() {cancelNextWalk = true;}
		void addEventWalk(bool firstStep = false);
		void stopEventWalk();
		virtual void goToFollowCreature();

		//walk events
		virtual void onWalk(Direction& dir);
		virtual void onWalkAborted() {}
		virtual void onWalkComplete() {}

		//follow functions
		virtual Creature* getFollowCreature() const {return followCreature;}
		virtual bool setFollowCreature(Creature* creature, bool fullPathSearch = false);

		//follow events
		virtual void onFollowCreature(const Creature*) {}
		virtual void onFollowCreatureComplete(const Creature*) {}

		//combat functions
		Creature* getAttackedCreature() {return attackedCreature;}
		virtual bool setAttackedCreature(Creature* creature);
		virtual BlockType_t blockHit(Creature* attacker, CombatType_t combatType, int32_t& damage,
			bool checkDefense = false, bool checkArmor = false, bool reflect = true, bool field = false, bool element = false);

		void setMaster(Creature* creature) {master = creature;}
		Creature* getMaster() {return master;}
		const Creature* getMaster() const {return master;}
		Player* getPlayerMaster() const {return isPlayerSummon() ? master->getPlayer() : NULL;}
		bool isSummon() const {return master != NULL;}
		bool isPlayerSummon() const {return master && master->getPlayer();}

		virtual void addSummon(Creature* creature);
		virtual void removeSummon(const Creature* creature);
		const std::list<Creature*>& getSummons() {return summons;}
		void destroySummons();
		uint32_t getSummonCount() const {return summons.size();}

		virtual int32_t getArmor() const {return 0;}
		virtual int32_t getDefense() const {return 0;}
		virtual float getAttackFactor() const {return 1.0f;}
		virtual float getDefenseFactor() const {return 1.0f;}

		bool addCondition(Condition* condition);
		bool addCombatCondition(Condition* condition);
		void removeCondition(ConditionType_t type);
		void removeCondition(ConditionType_t type, ConditionId_t conditionId);
		void removeCondition(Condition* condition);
		void removeCondition(const Creature* attacker, ConditionType_t type);
		void removeConditions(ConditionEnd_t reason, bool onlyPersistent = true);
		Condition* getCondition(ConditionType_t type, ConditionId_t conditionId, uint32_t subId = 0) const;
		void executeConditions(uint32_t interval);
		bool hasCondition(ConditionType_t type, int32_t subId = 0, bool checkTime = true) const;
		virtual bool isImmune(ConditionType_t type) const;
		virtual bool isImmune(CombatType_t type) const;
		virtual bool isSuppress(ConditionType_t type) const;
		virtual uint32_t getDamageImmunities() const {return 0;}
		virtual uint32_t getConditionImmunities() const {return 0;}
		virtual uint32_t getConditionSuppressions() const {return 0;}
		virtual bool isAttackable() const {return true;}
		virtual bool isAccountManager() const {return false;}

		virtual void changeHealth(int32_t healthChange);
		void changeMaxHealth(uint32_t healthChange) {healthMax = healthChange;}
		virtual void changeMana(int32_t manaChange);
		void changeMaxMana(uint32_t manaChange) {manaMax = manaChange;}

		virtual bool getStorage(const std::string& key, std::string& value) const;
		virtual bool setStorage(const std::string& key, const std::string& value);
		virtual void eraseStorage(const std::string& key) {storageMap.erase(key);}

		inline StorageMap::const_iterator getStorageBegin() const {return storageMap.begin();}
		inline StorageMap::const_iterator getStorageEnd() const {return storageMap.end();}

		virtual void gainHealth(Creature* caster, int32_t amount);
		virtual void drainHealth(Creature* attacker, CombatType_t combatType, int32_t damage);
		virtual void drainMana(Creature* attacker, CombatType_t combatType, int32_t damage);

		virtual bool challengeCreature(Creature*) {return false;}
		virtual bool convinceCreature(Creature*) {return false;}

		virtual bool onDeath();
		virtual double getGainedExperience(Creature* attacker) const;
		void addDamagePoints(Creature* attacker, int32_t damagePoints);
		void addHealPoints(Creature* caster, int32_t healthPoints);
		bool hasBeenAttacked(uint32_t attackerId) const;

		//combat event functions
		virtual void onAddCondition(ConditionType_t type, bool hadCondition);
		virtual void onAddCombatCondition(ConditionType_t, bool) {}
		virtual void onEndCondition(ConditionType_t type, ConditionId_t conditionId);
		virtual void onTickCondition(ConditionType_t type, ConditionId_t conditionId, int32_t interval, bool& _remove);
		virtual void onCombatRemoveCondition(const Creature* attacker, Condition* condition);
		virtual void onTarget(Creature*) {}
		virtual void onSummonTarget(Creature*, Creature*) {}
		virtual void onAttacked() {}
		virtual void onTargetDrainHealth(Creature* target, int32_t points);
		virtual void onSummonTargetDrainHealth(Creature*, Creature*, int32_t) {}
		virtual void onTargetDrainMana(Creature* target, int32_t points);
		virtual void onSummonTargetDrainMana(Creature*, Creature*, int32_t) {}
		virtual void onTargetDrain(Creature* target, int32_t points);
		virtual void onSummonTargetDrain(Creature*, Creature*, int32_t) {}
		virtual void onTargetGainHealth(Creature* target, int32_t points);
		virtual void onTargetGainMana(Creature* target, int32_t points);
		virtual void onTargetGain(Creature* target, int32_t points);
		virtual void onTargetKilled(Creature* target);
		virtual bool onKilledCreature(Creature* target, DeathEntry& entry);
		virtual void onGainExperience(double& gainExp, Creature* target, bool multiplied);
		virtual void onGainSharedExperience(double& gainExp, Creature* target, bool multiplied);
		virtual void onTargetBlockHit(Creature*, BlockType_t) {}
		virtual void onBlockHit(BlockType_t) {}
		virtual void onChangeZone(ZoneType_t zone);
		virtual void onTargetChangeZone(ZoneType_t zone);
		virtual void onIdleStatus();

		virtual void getCreatureLight(LightInfo& light) const;
		virtual void resetLight();
		void setCreatureLight(LightInfo& light) {internalLight = light;}

		virtual void onThink(uint32_t interval);
		virtual void onAttacking(uint32_t interval);
		virtual void onWalk();
		virtual bool getNextStep(Direction& dir, uint32_t& flags);

		virtual void onAddTileItem(const Tile* tile, const Position& pos, const Item* item);
		virtual void onUpdateTileItem(const Tile* tile, const Position& pos, const Item* oldItem,
			const ItemType& oldType, const Item* newItem, const ItemType& newType);
		virtual void onRemoveTileItem(const Tile* tile, const Position& pos, const ItemType& iType, const Item* item);
		virtual void onUpdateTile(const Tile*, const Position&) {}

		virtual void onCreatureAppear(const Creature* creature);
		virtual void onCreatureDisappear(const Creature* creature, bool) {internalCreatureDisappear(creature, true);}
		virtual void onCreatureMove(const Creature* creature, const Tile* newTile, const Position& newPos,
			const Tile* oldTile, const Position& oldPos, bool teleport);

		virtual void onTargetDisappear(bool) {}
		virtual void onFollowCreatureDisappear(bool) {}

		virtual void onCreatureTurn(const Creature*) {}
		virtual void onCreatureSay(const Creature*, MessageClasses, const std::string&,
			Position* = NULL) {}

		virtual void onCreatureChangeOutfit(const Creature*, const Outfit_t&) {}
		virtual void onCreatureConvinced(const Creature*, const Creature*) {}
		virtual void onCreatureChangeVisible(const Creature*, Visible_t) {}
		virtual void onPlacedCreature() {}
		virtual void onRemovedCreature();

		virtual WeaponType_t getWeaponType() {return WEAPON_NONE;}
		virtual bool getCombatValues(int32_t&, int32_t&) {return false;}

		virtual void setSkull(Skulls_t newSkull) {skull = newSkull;}
		virtual Skulls_t getSkull() const {return skull;}
		virtual Skulls_t getSkullType(const Creature* creature) const {return creature->getSkull();}

		virtual void setShield(PartyShields_t newPartyShield) {partyShield = newPartyShield;}
		virtual PartyShields_t getShield() const {return partyShield;}
		virtual PartyShields_t getPartyShield(const Creature* creature) const {return creature->getShield();}

		virtual void setEmblem(GuildEmblems_t newGuildEmblem) {guildEmblem = newGuildEmblem;}
		virtual GuildEmblems_t getEmblem() const {return guildEmblem;}
		virtual GuildEmblems_t getGuildEmblem(const Creature* creature) const {return creature->getEmblem();}

		virtual void setDropLoot(lootDrop_t _lootDrop) {lootDrop = _lootDrop;}
		virtual void setLossSkill(bool _skillLoss) {skillLoss = _skillLoss;}

		bool getLossSkill() const {return skillLoss;}
		void setNoMove(bool _cannotMove)
		{
			cannotMove = _cannotMove;
			cancelNextWalk = true;
		}
		bool getNoMove() const {return cannotMove;}

		//creature script events
		bool registerCreatureEvent(const std::string& name);
		bool unregisterCreatureEvent(const std::string& name);
		void unregisterCreatureEvent(CreatureEventType_t type);
		CreatureEventList getCreatureEvents(CreatureEventType_t type);

		virtual void setParent(Cylinder* cylinder)
		{
			_tile = dynamic_cast<Tile*>(cylinder);
			_position = _tile->getPosition();
			Thing::setParent(cylinder);
		}

		virtual Position getPosition() const {return _position;}
		virtual Tile* getTile() {return _tile;}
		virtual const Tile* getTile() const {return _tile;}
		int32_t getWalkCache(const Position& pos) const;

		const Position& getLastPosition() {return lastPosition;}
		void setLastPosition(Position newLastPos) {lastPosition = newLastPos;}
		static bool canSee(const Position& myPos, const Position& pos, uint32_t viewRangeX, uint32_t viewRangeY);

	protected:
		static const int32_t mapWalkWidth = Map::maxViewportX * 2 + 1;
		static const int32_t mapWalkHeight = Map::maxViewportY * 2 + 1;
		bool localMapCache[mapWalkHeight][mapWalkWidth];

		virtual bool useCacheMap() const {return false;}

		Position _position;
		Tile* _tile;
		uint32_t id;
		bool removed;
		bool isMapLoaded;
		bool isUpdatingPath;
		bool checked;
		StorageMap storageMap;

		int32_t checkVector;
		int32_t health, healthMax;
		int32_t mana, manaMax;
		int64_t lastFailedFollow;

		bool hideName, hideHealth, cannotMove;
		MessageClasses speakType;

		Outfit_t currentOutfit;
		Outfit_t defaultOutfit;

		Position masterPosition;
		Position lastPosition;
		int32_t masterRadius;
		uint64_t lastStep;
		uint32_t lastStepCost;
		uint32_t baseSpeed;
		int32_t varSpeed;
		bool skillLoss;
		lootDrop_t lootDrop;
		Skulls_t skull;
		PartyShields_t partyShield;
		GuildEmblems_t guildEmblem;
		Direction direction;
		ConditionList conditions;
		LightInfo internalLight;

		//summon variables
		Creature* master;
		std::list<Creature*> summons;

		//follow variables
		Creature* followCreature;
		uint32_t eventWalk;
		bool cancelNextWalk;
		std::list<Direction> listWalkDir;
		uint32_t walkUpdateTicks;
		bool hasFollowPath;
		bool forceUpdateFollowPath;

		//combat variables
		Creature* attackedCreature;
		struct CountBlock_t
		{
			uint32_t total;
			int64_t start, ticks;

			CountBlock_t(uint32_t points)
			{
				total = points;
				start = ticks = OTSYS_TIME();
			}

			CountBlock_t() {total = start = ticks = 0;}
		};

		typedef std::map<uint32_t, CountBlock_t> CountMap;
		CountMap damageMap;
		CountMap healMap;

		CreatureEventList eventsList;
		uint32_t blockCount, blockTicks, lastHitCreature;
		CombatType_t lastDamageSource;

		#ifdef __DEBUG__
		void validateMapCache();
		#endif
		void updateMapCache();

		void updateTileCache(const Tile* tile);
		void updateTileCache(const Tile* tile, int32_t dx, int32_t dy);
		void updateTileCache(const Tile* tile, const Position& pos);

		void internalCreatureDisappear(const Creature* creature, bool isLogout);

		virtual bool hasExtraSwing() {return false;}

		virtual uint16_t getLookCorpse() const {return 0;}
		virtual uint64_t getLostExperience() const {return 0;}

		virtual double getDamageRatio(Creature* attacker) const;
		virtual void getPathSearchParams(const Creature* creature, FindPathParams& fpp) const;
		DeathList getKillers();

		virtual Item* createCorpse(DeathList deathList);
		virtual void dropLoot(Container*) {}
		virtual void dropCorpse(DeathList deathList);

		virtual void doAttacking(uint32_t) {}

		friend class Game;
		friend class Map;
		friend class LuaInterface;
};
#endif
