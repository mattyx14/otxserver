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

#ifndef __PLAYER__
#define __PLAYER__
#include "otsystem.h"
#include "enums.h"

#include "creature.h"
#include "cylinder.h"

#include "container.h"
#include "depot.h"

#include "outfit.h"
#include "vocation.h"
#include "group.h"

#include "spectators.h"
#include "ioguild.h"
#include "party.h"
#include "npc.h"

class House;
class Weapon;
class Npc;
class Party;
class SchedulerTask;
class Quest;
class ProtocolGame;

enum skillsid_t
{
	SKILL_LEVEL = 0,
	SKILL_TRIES = 1,
	SKILL_PERCENT = 2
};

enum playerinfo_t
{
	PLAYERINFO_LEVEL,
	PLAYERINFO_LEVELPERCENT,
	PLAYERINFO_HEALTH,
	PLAYERINFO_MAXHEALTH,
	PLAYERINFO_MANA,
	PLAYERINFO_MAXMANA,
	PLAYERINFO_MAGICLEVEL,
	PLAYERINFO_MAGICLEVELPERCENT,
	PLAYERINFO_SOUL,
};

enum freeslot_t
{
	SLOT_TYPE_NONE,
	SLOT_TYPE_INVENTORY,
	SLOT_TYPE_CONTAINER
};

enum chaseMode_t
{
	CHASEMODE_STANDSTILL,
	CHASEMODE_FOLLOW,
};

enum fightMode_t
{
	FIGHTMODE_ATTACK,
	FIGHTMODE_BALANCED,
	FIGHTMODE_DEFENSE
};

enum secureMode_t
{
	SECUREMODE_ON,
	SECUREMODE_OFF
};

enum tradestate_t
{
	TRADE_NONE,
	TRADE_INITIATED,
	TRADE_ACCEPT,
	TRADE_ACKNOWLEDGE,
	TRADE_TRANSFER
};

enum AccountManager_t
{
	MANAGER_NONE,
	MANAGER_NEW,
	MANAGER_ACCOUNT,
	MANAGER_NAMELOCK
};

enum GamemasterCondition_t
{
	GAMEMASTER_INVISIBLE = 0,
	GAMEMASTER_IGNORE = 1,
	GAMEMASTER_TELEPORT = 2
};

typedef std::set<uint32_t> VIPSet;
typedef std::list<std::pair<uint16_t, std::string> > ChannelsList;
typedef std::vector<std::pair<uint32_t, Container*> > ContainerVector;
typedef std::map<uint32_t, std::pair<Depot*, bool> > DepotMap;
typedef std::map<uint32_t, uint32_t> MuteCountMap;
typedef std::list<std::string> LearnedInstantSpellList;
typedef std::list<uint32_t> InvitationsList;
typedef std::list<Party*> PartyList;
typedef std::map<uint32_t, War_t> WarMap;

#define SPEED_MAX 1500
#define SPEED_MIN 10
#define STAMINA_MAX (42 * 60 * 60 * 1000)
#define STAMINA_MULTIPLIER (60 * 1000)

class Player : public Creature, public Cylinder
{
	public:
#ifdef __ENABLE_SERVER_DIAGNOSTIC__
		static uint32_t playerCount;
#endif
		Player(const std::string& name, ProtocolGame_ptr p);
		virtual ~Player();

		virtual Player* getPlayer() {return this;}
		virtual const Player* getPlayer() const {return this;}
		virtual CreatureType_t getType() const {return CREATURETYPE_PLAYER;}

		void setID() override
		{
			if(id == 0) {
				id = playerAutoID++;
			}
		}

		static MuteCountMap muteCountMap;

		virtual const std::string& getName() const {return name;}
		virtual const std::string& getNameDescription() const {return nameDescription;}
		virtual std::string getDescription(int32_t lookDistance) const;

		const std::string& getSpecialDescription() const {return specialDescription;}
		void setSpecialDescription(const std::string& desc) {specialDescription = desc;}

		void manageAccount(const std::string& text);
		bool isAccountManager() const {return (accountManager != MANAGER_NONE);}
		void kick(bool displayEffect, bool forceLogout);

		void setGUID(uint32_t _guid) {guid = _guid;}
		uint32_t getGUID() const {return guid;}

		static AutoList<Player> autoList;
		virtual uint32_t rangeId() {return PLAYER_ID_RANGE;}
		static bool sort(Player* lhs, Player* rhs) {return lhs->getName() < rhs->getName();}

		void addList();
		void removeList();

		static uint64_t getExpForLevel(uint32_t lv)
		{
			static std::map<uint32_t, uint64_t> cache;
			lv--;

			std::map<uint32_t, uint64_t>::iterator it = cache.find(lv);
			if(it != cache.end())
				return it->second;

			uint64_t exp = ((50ULL * lv * lv * lv) - (150ULL * lv * lv) + (400ULL * lv)) / 3ULL;
			cache[lv] = exp;
			return exp;
		}

		bool addOfflineTrainingTries(skills_t skill, int32_t tries);

		void addOfflineTrainingTime(int32_t addTime) {offlineTrainingTime = std::min(12 * 3600 * 1000, offlineTrainingTime + addTime);}
		void removeOfflineTrainingTime(int32_t removeTime) {offlineTrainingTime = std::max(0, offlineTrainingTime - removeTime);}
		int32_t getOfflineTrainingTime() {return offlineTrainingTime;}

		int32_t getOfflineTrainingSkill() {return offlineTrainingSkill;}
		void setOfflineTrainingSkill(int32_t skill) {offlineTrainingSkill = skill;}

		uint32_t getPromotionLevel() const {return promotionLevel;}
		void setPromotionLevel(uint32_t pLevel);

		bool changeOutfit(Outfit_t outfit, bool checkList);
		void hasRequestedOutfit(bool v) {requestedOutfit = v;}

		Vocation* getVocation() const {return vocation;}
		int32_t getPlayerInfo(playerinfo_t playerinfo) const;

		void setParty(Party* _party) {party = _party;}
		Party* getParty() const {return party;}

		bool isInviting(const Player* player) const;
		bool isPartner(const Player* player) const;

		bool getHideHealth() const;

		bool addPartyInvitation(Party* party);
		bool removePartyInvitation(Party* party);
		void clearPartyInvitations();

		uint32_t getGuildId() const {return guildId;}
		void setGuildId(uint32_t newId) {guildId = newId;}
		uint32_t getRankId() const {return rankId;}
		void setRankId(uint32_t newId) {rankId = newId;}

		GuildLevel_t getGuildLevel() const {return guildLevel;}
		bool setGuildLevel(GuildLevel_t newLevel, uint32_t rank = 0);

		const std::string& getGuildName() const {return guildName;}
		void setGuildName(const std::string& newName) {guildName = newName;}
		const std::string& getRankName() const {return rankName;}
		void setRankName(const std::string& newName) {rankName = newName;}

		const std::string& getGuildNick() const {return guildNick;}
		void setGuildNick(const std::string& newNick) {guildNick = newNick;}

		bool isGuildInvited(uint32_t guildId) const;
		void leaveGuild();

		void setFlags(uint64_t flags) {if(group) group->setFlags(flags);}
		bool hasFlag(PlayerFlags value) const {return group != NULL && group->hasFlag(value);}
		void setCustomFlags(uint64_t flags) {if(group) group->setCustomFlags(flags);}
		bool hasCustomFlag(PlayerCustomFlags value) const {return group != NULL && group->hasCustomFlag(value);}

		void addBlessing(int16_t blessing) {blessings += blessing;}
		bool hasBlessing(int16_t blessing) const {return ((blessings & ((int16_t)1 << blessing)) != 0);}
		void setPVPBlessing(bool value) {pvpBlessing = value;}
		bool hasPVPBlessing() const {return pvpBlessing;}
		uint16_t getBlessings() const;

		bool isUsingOtclient() const { return operatingSystem >= CLIENTOS_OTCLIENT_LINUX; }
		OperatingSystem_t getOperatingSystem() const {return operatingSystem;}
		void setOperatingSystem(OperatingSystem_t os) {operatingSystem = os;}
		uint32_t getClientVersion() const {return clientVersion;}
		void setClientVersion(uint32_t version) {clientVersion = version;}

		bool hasClient() const {return (client->getOwner() != NULL);}
		bool isVirtual() const {return (getID() == 0);}
		uint32_t getIP() const;
		bool canOpenCorpse(uint32_t ownerId);

		Container* getContainer(uint32_t cid);
		int32_t getContainerID(const Container* container) const;

		void addContainer(uint32_t cid, Container* container);
		void closeContainer(uint32_t cid);

		virtual bool setStorage(const std::string& key, const std::string& value);
		virtual void eraseStorage(const std::string& key);

		void generateReservedStorage();
		bool transferMoneyTo(const std::string& name, uint64_t amount);
		void increaseCombatValues(int32_t& min, int32_t& max, bool useCharges, bool countWeapon);

		void setGroupId(int32_t newId);
		int32_t getGroupId() const {return groupId;}
		void setGroup(Group* newGroup);
		Group* getGroup() const {return group;}

		virtual bool isGhost() const {return hasCondition(CONDITION_GAMEMASTER, GAMEMASTER_INVISIBLE) || hasFlag(PlayerFlag_CannotBeSeen);}
		virtual bool isWalkable() const {return hasCustomFlag(PlayerCustomFlag_IsWalkable);}

		void switchSaving() {saving = !saving;}
		bool isSaving() const {return saving;}

		uint32_t getIdleTime() const {return idleTime;}
		void setIdleTime(uint32_t amount) {idleTime = amount;}

		bool checkLoginDelay() const;
		bool isTrading() const {return (tradePartner != NULL);}

		uint32_t getAccount() const {return accountId;}
		std::string getAccountName() const {return account;}
		uint16_t getAccess() const {return group ? group->getAccess() : 0;}
		uint16_t getGhostAccess() const {return group ? group->getGhostAccess() : 0;}

		uint32_t getLevel() const {return level;}
		uint64_t getExperience() const {return experience;}
		uint32_t getMagicLevel() const {return getPlayerInfo(PLAYERINFO_MAGICLEVEL);}
		uint32_t getBaseMagicLevel() const {return magLevel;}
		uint64_t getSpentMana() const {return manaSpent;}

		uint32_t getExtraAttackSpeed() const {return extraAttackSpeed;}
		void setPlayerExtraAttackSpeed(uint32_t speed);

		bool isPremium() const;
		int32_t getPremiumDays() const {return premiumDays;}
		void addPremiumDays(int32_t days);

		bool hasEnemy() const {return !warMap.empty();}
		bool getEnemy(const Player* player, War_t& data) const;

		bool isEnemy(const Player* player, bool allies) const;
		bool isAlly(const Player* player) const;

		void addEnemy(uint32_t guild, War_t war)
			{warMap[guild] = war;}
		void removeEnemy(uint32_t guild) {warMap.erase(guild);}

		uint32_t getVocationId() const {return vocationId;}
		void setVocation(uint32_t id);
		uint16_t getSex(bool full) const {return full ? sex : sex % 2;}
		void setSex(uint16_t);

		virtual void setDropLoot(lootDrop_t _lootDrop);
		virtual void setLossSkill(bool _skillLoss);

		uint64_t getStamina() const {return hasFlag(PlayerFlag_HasInfiniteStamina) ? STAMINA_MAX : stamina;}
		void setStamina(uint64_t value) {stamina = std::min((uint64_t)STAMINA_MAX, (uint64_t)std::max((uint64_t)0, value));}
		uint32_t getStaminaMinutes() const {return (uint32_t)(getStamina() / (uint64_t)STAMINA_MULTIPLIER);}
		void setStaminaMinutes(uint32_t value) {setStamina((uint64_t)(value * STAMINA_MULTIPLIER));}
		void useStamina(int64_t value) {stamina = std::min((int64_t)STAMINA_MAX, (int64_t)std::max((int64_t)0, ((int64_t)stamina + value)));}
		uint64_t getSpentStamina() {return (uint64_t)STAMINA_MAX - stamina;}

		int64_t getLastLoad() const {return lastLoad;}
		time_t getLastLogin() const {return lastLogin;}
		time_t getLastLogout() const {return lastLogout;}

		Position getLoginPosition() const {return loginPosition;}

		uint32_t getTown() const {return town;}
		void setTown(uint32_t _town) {town = _town;}

		virtual bool isPushable() const;
		virtual int32_t getThrowRange() const {return 1;}
		virtual double getGainedExperience(Creature* attacker) const;

		bool isMuted(uint16_t channelId, MessageClasses type, int32_t& time);
		void addMessageBuffer();
		void removeMessageBuffer();

		double getCapacity() const {return capacity;}
		void setCapacity(double newCapacity) {capacity = newCapacity;}
		double getFreeCapacity() const;

		virtual int32_t getSoul() const {return getPlayerInfo(PLAYERINFO_SOUL);}
		virtual int32_t getMaxHealth() const {return getPlayerInfo(PLAYERINFO_MAXHEALTH);}
		virtual int32_t getMaxMana() const {return getPlayerInfo(PLAYERINFO_MAXMANA);}
		int32_t getSoulMax() const {return soulMax;}

		Item* getInventoryItem(slots_t slot) const;
		Item* getEquippedItem(slots_t slot) const;

		bool isItemAbilityEnabled(slots_t slot) const {return inventoryAbilities[slot];}
		void setItemAbility(slots_t slot, bool enabled) {inventoryAbilities[slot] = enabled;}

		int32_t getBaseSkill(skills_t skill) const {return skills[skill][SKILL_LEVEL];}
		int32_t getVarSkill(skills_t skill) const {return varSkills[skill];}
		void setVarSkill(skills_t skill, int32_t modifier) {varSkills[skill] += modifier;}

		int32_t getVarStats(stats_t stat) const {return varStats[stat];}
		void setVarStats(stats_t stat, int32_t modifier);
		int32_t getDefaultStats(stats_t stat);

		void setConditionSuppressions(uint32_t conditions, bool remove);

		uint32_t getLossPercent(lossTypes_t lossType) const {return lossPercent[lossType];}
		void setLossPercent(lossTypes_t lossType, uint32_t newPercent) {lossPercent[lossType] = newPercent;}

		Depot* getDepot(uint32_t depotId, bool autoCreateDepot);
		bool addDepot(Depot* depot, uint32_t depotId);
		void useDepot(uint32_t depotId, bool value);

		virtual bool canSee(const Position& pos) const;
		virtual bool canSeeCreature(const Creature* creature) const;
		virtual bool canWalkthrough(const Creature* creature) const;
		void setWalkthrough(const Creature* creature, bool walkthrough);

		virtual bool canSeeInvisibility() const {return hasFlag(PlayerFlag_CanSenseInvisibility);}

		bool hasSentChat() const {return sentChat;}
		void setSentChat(bool sending) {sentChat = sending;}

		virtual RaceType_t getRace() const {return RACE_BLOOD;}

		//safe-trade functions
		void setTradeState(tradestate_t state) {tradeState = state;}
		tradestate_t getTradeState() {return tradeState;}
		Item* getTradeItem() {return tradeItem;}

		//shop functions
		void setShopOwner(Npc* owner, int32_t onBuy, int32_t onSell, ShopInfoList offer)
		{
			shopOwner = owner;
			purchaseCallback = onBuy;
			saleCallback = onSell;
			shopOffer = offer;
		}

		Npc* getShopOwner() const {return shopOwner;}
		Npc* getShopOwner(int32_t& onBuy, int32_t& onSell)
		{
			onBuy = purchaseCallback;
			onSell = saleCallback;
			return shopOwner;
		}

		const Npc* getShopOwner(int32_t& onBuy, int32_t& onSell) const
		{
			onBuy = purchaseCallback;
			onSell = saleCallback;
			return shopOwner;
		}

		//Quest functions
		void onUpdateQuest();

		//V.I.P. functions
		void notifyLogIn(Player* loginPlayer);
		void notifyLogOut(Player* logoutPlayer);
		bool removeVIP(uint32_t guid);
		bool addVIP(uint32_t guid, const std::string& name, bool online, bool loading = false);

		//follow functions
		virtual bool setFollowCreature(Creature* creature, bool fullPathSearch = false);
		virtual void goToFollowCreature();

		//follow events
		virtual void onFollowCreature(const Creature* creature);

		//walk events
		virtual void onWalk(Direction& dir);
		virtual void onWalkAborted();
		virtual void onWalkComplete();

		void openShopWindow(Npc* npc);
		void closeShopWindow(bool send = true);
		bool canShopItem(uint16_t itemId, uint8_t subType, ShopEvent_t event);

		chaseMode_t getChaseMode() const {return chaseMode;}
		void setChaseMode(chaseMode_t mode);
		fightMode_t getFightMode() const {return fightMode;}
		void setFightMode(fightMode_t mode) {fightMode = mode;}
		secureMode_t getSecureMode() const {return secureMode;}
		void setSecureMode(secureMode_t mode) {secureMode = mode;}

		//combat functions
		virtual bool setAttackedCreature(Creature* creature);
		bool hasShield() const;

		bool isImmune(CombatType_t type) const;
		bool isImmune(ConditionType_t type) const;
		bool isProtected() const;
		virtual bool isAttackable() const;

		virtual void changeHealth(int32_t healthChange);
		virtual void changeMana(int32_t manaChange);
		void changeSoul(int32_t soulChange);

		bool isPzLocked() const {return pzLocked;}
		void setPzLocked(bool v) {pzLocked = v;}

		virtual BlockType_t blockHit(Creature* attacker, CombatType_t combatType, int32_t& damage,
			bool checkDefense = false, bool checkArmor = false, bool reflect = true, bool field = false, bool element = false);

		virtual void doAttacking(uint32_t interval);
		virtual bool hasExtraSwing() {return lastAttack > 0 && ((OTSYS_TIME() - lastAttack) >= getAttackSpeed());}
		void setLastAttack(uint64_t time) {lastAttack = time;}

		int32_t getSkill(skills_t skilltype, skillsid_t skillinfo) const;
		bool getAddAttackSkill() const {return addAttackSkillPoint;}
		BlockType_t getLastAttackBlockType() const {return lastAttackBlockType;}

		Item* getWeapon(bool ignoreAmmo);
		ItemVector getWeapons() const;

		virtual WeaponType_t getWeaponType();
		int32_t getWeaponSkill(const Item* item) const;
		void getShieldAndWeapon(const Item* &_shield, const Item* &_weapon) const;

		virtual void drainHealth(Creature* attacker, CombatType_t combatType, int32_t damage);
		virtual void drainMana(Creature* attacker, CombatType_t combatType, int32_t damage);

		void addExperience(uint64_t exp);
		void removeExperience(uint64_t exp, bool updateStats = true);
		void addManaSpent(uint64_t amount, bool useMultiplier = true);
		void addSkillAdvance(skills_t skill, uint64_t count, bool useMultiplier = true);
		bool addUnjustifiedKill(const Player* attacked, bool countNow);

		virtual int32_t getArmor() const;
		virtual int32_t getCriticalHitChance() const;
		virtual int32_t getDefense() const;
		virtual float getAttackFactor() const;
		virtual float getDefenseFactor() const;

		void addCooldown(uint32_t ticks, uint16_t spellId);
		void addExhaust(uint32_t ticks, Exhaust_t exhaust);
		void addInFightTicks(bool pzLock, int32_t ticks = 0);
		void addDefaultRegeneration(uint32_t addTicks);

		//combat event functions
		virtual void onAddCondition(ConditionType_t type, bool hadCondition);
		virtual void onAddCombatCondition(ConditionType_t type, bool hadCondition);
		virtual void onEndCondition(ConditionType_t type, ConditionId_t id);
		virtual void onCombatRemoveCondition(const Creature* attacker, Condition* condition);
		virtual void onTickCondition(ConditionType_t type, ConditionId_t id, int32_t interval, bool& _remove);
		virtual void onTarget(Creature* target);
		virtual void onSummonTarget(Creature* summon, Creature* target);
		virtual void onAttacked();
		virtual void onTargetDrain(Creature* target, int32_t points);
		virtual void onSummonTargetDrain(Creature* summon, Creature* target, int32_t points);
		virtual void onTargetGain(Creature* target, int32_t points);
		virtual bool onKilledCreature(Creature* target, DeathEntry& entry);
		virtual void onGainExperience(double& gainExp, Creature* target, bool multiplied);
		virtual void onGainSharedExperience(double& gainExp, Creature* taraget, bool multiplied);
		virtual void onTargetBlockHit(Creature* target, BlockType_t blockType);
		virtual void onBlockHit(BlockType_t blockType);
		virtual void onChangeZone(ZoneType_t zone);
		virtual void onTargetChangeZone(ZoneType_t zone);
		virtual void onIdleStatus();
		virtual void onPlacedCreature();

		virtual void getCreatureLight(LightInfo& light) const;
		Skulls_t getSkull() const;

		Skulls_t getSkullType(const Creature* creature) const;
		PartyShields_t getPartyShield(const Creature* creature) const;
		GuildEmblems_t getGuildEmblem(const Creature* creature) const;

		bool hasAttacked(const Player* attacked) const;
		void addAttacked(const Player* attacked);
		void clearAttacked() {attackedSet.clear();}

		time_t getSkullEnd() const {return skullEnd;}
		void setSkullEnd(time_t _time, bool login, Skulls_t _skull);

		bool addOutfit(uint32_t outfitId, uint32_t addons);
		bool removeOutfit(uint32_t outfitId, uint32_t addons);

		bool canWearOutfit(uint32_t outfitId, uint32_t addons);

		//tile
		//send methods
		void sendAddTileItem(const Tile* tile, const Position& pos, const Item* item)
			{if(client) client->sendAddTileItem(tile, pos, tile->getClientIndexOfThing(this, item), item);}
		void sendUpdateTileItem(const Tile* tile, const Position& pos, const Item* oldItem, const Item* newItem)
			{if(client) client->sendUpdateTileItem(tile, pos, tile->getClientIndexOfThing(this, oldItem), newItem);}
		void sendRemoveTileItem(const Tile* tile, const Position& pos, uint32_t stackpos, const Item*)
			{if(client) client->sendRemoveTileItem(tile, pos, stackpos);}
		void sendUpdateTile(const Tile* tile, const Position& pos)
			{if(client) client->sendUpdateTile(tile, pos);}

		void sendChannelMessage(std::string author, std::string text, MessageClasses type, uint16_t channel)
			{if(client) client->sendChannelMessage(author, text, type, channel);}
		void sendCreatureAppear(const Creature* creature)
			{if(client) client->sendAddCreature(creature, creature->getPosition(), creature->getTile()->getClientIndexOfThing(this, creature));}
		void sendCreatureAppear(const Creature* creature, ProtocolGame* target)
			{if(target) target->sendAddCreature(creature, creature->getPosition(), creature->getTile()->getClientIndexOfThing(this, creature));}
		void sendCreatureDisappear(const Creature* creature, uint32_t stackpos)
			{if(client) client->sendRemoveCreature(creature, creature->getPosition(), stackpos);}
		void sendCreatureMove(const Creature* creature, const Tile* newTile, const Position& newPos,
			const Tile* oldTile, const Position& oldPos, uint32_t oldStackpos, bool teleport)
			{if(client) client->sendMoveCreature(creature, newTile, newPos, newTile->getClientIndexOfThing(this, creature), oldTile, oldPos, oldStackpos, teleport);}

		void sendCreatureTurn(const Creature* creature)
			{if(client) client->sendCreatureTurn(creature, creature->getTile()->getClientIndexOfThing(this, creature));}
		void sendCreatureSay(const Creature* creature, MessageClasses type, const std::string& text, Position* pos = NULL, uint32_t statementId = 0)
			{if(client) client->sendCreatureSay(creature, type, text, pos, statementId);}
		void sendCreatureChannelSay(Creature* creature, MessageClasses type, const std::string& text, uint16_t channelId, uint32_t statementId = 0) const
			{if(client) client->sendCreatureChannelSay(creature, type, text, channelId, statementId);}
		void sendCreatureSquare(const Creature* creature, uint8_t color)
			{if(client) client->sendCreatureSquare(creature, color);}
		void sendCreatureChangeOutfit(const Creature* creature, const Outfit_t& outfit)
			{if(client) client->sendCreatureOutfit(creature, outfit);}
		void sendCreatureChangeVisible(const Creature* creature, Visible_t visible);
		void sendCreatureLight(const Creature* creature)
			{if(client) client->sendCreatureLight(creature);}
		void sendCreatureShield(const Creature* creature)
			{if(client) client->sendCreatureShield(creature);}
		void sendCreatureEmblem(const Creature* creature)
			{if(client) client->sendCreatureEmblem(creature);}
		void sendCreatureWalkthrough(const Creature* creature, bool walkthrough)
			{if(client) client->sendCreatureWalkthrough(creature, walkthrough);}

		void sendExtendedOpcode(uint8_t opcode, const std::string& buffer)
			{if(client) client->sendExtendedOpcode(opcode, buffer);}

		//container
		void sendAddContainerItem(const Container* container, const Item* item);
		void sendUpdateContainerItem(const Container* container, uint8_t slot, const Item* oldItem, const Item* newItem);
		void sendRemoveContainerItem(const Container* container, uint8_t slot, const Item* item);
		void sendContainer(uint32_t cid, const Container* container, bool hasParent)
			{if(client) client->sendContainer(cid, container, hasParent);}
		void sendContainers(ProtocolGame* target);

		//inventory
		void sendAddInventoryItem(slots_t slot, const Item* item)
			{if(client) client->sendAddInventoryItem(slot, item);}
		void sendUpdateInventoryItem(slots_t slot, const Item*, const Item* newItem)
			{if(client) client->sendUpdateInventoryItem(slot, newItem);}
		void sendRemoveInventoryItem(slots_t slot, const Item*)
			{if(client) client->sendRemoveInventoryItem(slot);}

		//event methods
		virtual void onUpdateTileItem(const Tile* tile, const Position& pos, const Item* oldItem,
			const ItemType& oldType, const Item* newItem, const ItemType& newType);
		virtual void onRemoveTileItem(const Tile* tile, const Position& pos,
			const ItemType& iType, const Item* item);

		virtual void onCreatureAppear(const Creature* creature);
		virtual void onCreatureDisappear(const Creature* creature, bool isLogout);
		virtual void onCreatureMove(const Creature* creature, const Tile* newTile, const Position& newPos,
			const Tile* oldTile, const Position& oldPos, bool teleport);

		virtual void onTargetDisappear(bool isLogout);
		virtual void onFollowCreatureDisappear(bool isLogout);

		//cylinder implementations
		virtual Cylinder* getParent() {return Creature::getParent();}
		virtual const Cylinder* getParent() const {return Creature::getParent();}
		virtual bool isRemoved() const {return Creature::isRemoved();}
		virtual Position getPosition() const {return Creature::getPosition();}
		virtual Tile* getTile() {return Creature::getTile();}
		virtual const Tile* getTile() const {return Creature::getTile();}
		virtual Item* getItem() {return NULL;}
		virtual const Item* getItem() const {return NULL;}
		virtual Creature* getCreature() {return this;}
		virtual const Creature* getCreature() const {return this;}

		//container
		void onAddContainerItem(const Container* container, const Item* item);
		void onUpdateContainerItem(const Container* container, uint8_t slot,
			const Item* oldItem, const ItemType& oldType, const Item* newItem, const ItemType& newType);
		void onRemoveContainerItem(const Container* container, uint8_t slot, const Item* item);

		void onCloseContainer(const Container* container);
		void onSendContainer(const Container* container);
		void autoCloseContainers(const Container* container);

		//inventory
		void onAddInventoryItem(slots_t, Item*) {}
		void onUpdateInventoryItem(slots_t slot, Item* oldItem, const ItemType& oldType,
			Item* newItem, const ItemType& newType);
		void onRemoveInventoryItem(slots_t slot, Item* item);

		void sendCancel(const std::string& msg) const
			{if(client) client->sendCancel(msg);}
		void sendCancelMessage(ReturnValue message) const;
		void sendCancelTarget() const
			{if(client) client->sendCancelTarget();}
		void sendCancelWalk() const
			{if(client) client->sendCancelWalk();}
		void sendChangeSpeed(const Creature* creature, uint32_t newSpeed) const
			{if(client) client->sendChangeSpeed(creature, newSpeed);}
		void sendCreatureHealth(const Creature* creature) const
			{if(client) client->sendCreatureHealth(creature);}

#ifdef __EXTENDED_DISTANCE_SHOOT__
		void sendDistanceShoot(const Position& from, const Position& to, uint16_t type) const
			{if(client) client->sendDistanceShoot(from, to, type);}
#else
		void sendDistanceShoot(const Position& from, const Position& to, uint8_t type) const
		{if(client) client->sendDistanceShoot(from, to, type);}
#endif

		void sendHouseWindow(House* house, uint32_t listId) const;
		void sendOutfitWindow() const {if(client) client->sendOutfitWindow();}
		void sendQuests() const {if(client) client->sendQuests();}
		void sendQuestInfo(Quest* quest) const {if(client) client->sendQuestInfo(quest);}
		void sendCreatureSkull(const Creature* creature) const
			{if(client) client->sendCreatureSkull(creature);}
		void sendFYIBox(std::string message)
			{if(client) client->sendFYIBox(message);}
		void sendCreatePrivateChannel(uint16_t channelId, const std::string& channelName)
			{if(client) client->sendCreatePrivateChannel(channelId, channelName);}
		void sendClosePrivate(uint16_t channelId) const
			{if(client) client->sendClosePrivate(channelId);}
		void sendIcons() const;

#ifdef __EXTENDED_MAGIC_EFFECTS__
		void sendMagicEffect(const Position& pos, uint16_t type) const
			{if(client) client->sendMagicEffect(pos, type);}
#else
		void sendMagicEffect(const Position& pos, uint8_t type) const
			{if(client) client->sendMagicEffect(pos, type);}
#endif

		void sendAnimatedText(const Position& pos, uint8_t color, const std::string& text) const
			{if(client) client->sendAnimatedText(pos, color, text);}
		void sendSkills() const
			{if(client) client->sendSkills();}
		void sendTextMessage(MessageClasses type, const std::string& message) const
			{if(client) client->sendTextMessage(type, message);}
		void sendStatsMessage(MessageClasses type, const std::string& message, Position pos, MessageDetails* details = NULL) const
			{if(client) client->sendStatsMessage(type, message, pos, details);}
		void sendReLoginWindow() const
			{if(client) client->sendReLoginWindow();}
		void sendTextWindow(Item* item, uint16_t maxLen, bool canWrite) const
			{if(client) client->sendTextWindow(windowTextId, item, maxLen, canWrite);}
		void sendShop(Npc* npc) const
			{if(client) client->sendShop(npc, shopOffer);}
		void sendGoods() const
			{if(client) client->sendGoods(shopOffer);}
		void sendCloseShop() const
			{if(client) client->sendCloseShop();}
		void sendTradeItemRequest(const Player* player, const Item* item, bool ack) const
			{if(client) client->sendTradeItemRequest(player, item, ack);}
		void sendTradeClose() const
			{if(client) client->sendCloseTrade();}
		void sendWorldLight(LightInfo& lightInfo)
			{if(client) client->sendWorldLight(lightInfo);}
		void sendChannelsDialog(const ChannelsList& channels)
			{if(client) client->sendChannelsDialog(channels);}
		void sendOpenPrivateChannel(const std::string& receiver)
			{if(client) client->sendOpenPrivateChannel(receiver);}
		void sendOutfitWindow()
			{if(client) client->sendOutfitWindow();}
		void sendCloseContainer(uint32_t cid)
			{if(client) client->sendCloseContainer(cid);}
		void sendChannel(uint16_t channelId, const std::string& channelName)
			{if(client) client->sendChannel(channelId, channelName);}
		void sendTutorial(uint8_t tutorialId)
			{if(client) client->sendTutorial(tutorialId);}
		void sendAddMarker(const Position& pos, MapMarks_t markType, const std::string& desc)
			{if(client) client->sendAddMarker(pos, markType, desc);}
		void sendRuleViolationsChannel(uint16_t channelId)
			{if(client) client->sendRuleViolationsChannel(channelId);}
		void sendRemoveReport(const std::string& name)
			{if(client) client->sendRemoveReport(name);}
		void sendLockRuleViolation()
			{if(client) client->sendLockRuleViolation();}
		void sendRuleViolationCancel(const std::string& name)
			{if(client) client->sendRuleViolationCancel(name);}

		void sendCritical() const;
		void sendPlayerIcons(Player* player);
		void sendStats();

		void receivePing() {lastPong = OTSYS_TIME();}
		virtual void onThink(uint32_t interval);
		uint32_t getAttackSpeed() const;

		void setLastMail(uint64_t v) {lastMail = v;}
		uint16_t getMailAttempts() const {return mailAttempts;}
		void addMailAttempt() {++mailAttempts;}

		virtual void postAddNotification(Creature* actor, Thing* thing, const Cylinder* oldParent,
			int32_t index, CylinderLink_t link = LINK_OWNER);
		virtual void postRemoveNotification(Creature* actor, Thing* thing, const Cylinder* newParent,
			int32_t index, bool isCompleteRemoval, CylinderLink_t link = LINK_OWNER);

		void setNextAction(int64_t time) {if(time > nextAction) nextAction = time;}
		bool canDoAction() const {return nextAction <= OTSYS_TIME();}
		uint32_t getNextActionTime(bool scheduler = true) const;

		void setNextExAction(int64_t time) {if(time > nextExAction) nextExAction = time;}
		bool canDoExAction() const {return nextExAction <= OTSYS_TIME();}

		Item* getWriteItem(uint32_t& _windowTextId, uint16_t& _maxWriteLen);
		void setWriteItem(Item* item, uint16_t _maxLen = 0);

		House* getEditHouse(uint32_t& _windowTextId, uint32_t& _listId);
		void setEditHouse(House* house, uint32_t listId = 0);

		void learnInstantSpell(const std::string& name);
		void unlearnInstantSpell(const std::string& name);
		bool hasLearnedInstantSpell(const std::string& name) const;

		VIPSet VIPList;
		ContainerVector containerVec;
		InvitationsList invitationsList;
		ConditionList storedConditionList;
		DepotMap depots;
		Container transferContainer;

		// TODO: make it private?
		uint32_t marriage;
		uint64_t balance;
		double rates[SKILL__LAST + 1];

	protected:
		void checkTradeState(const Item* item);
		void internalAddDepot(Depot* depot, uint32_t depotId);

		bool gainExperience(double& gainExp, Creature* target);
		bool rateExperience(double& gainExp, Creature* target);

		void updateInventoryWeight();
		void updateInventoryGoods(uint32_t itemId);
		void updateItemsLight(bool internal = false);
		void updateWeapon();
		void updateBaseSpeed()
		{
			if(!hasFlag(PlayerFlag_SetMaxSpeed))
				baseSpeed = vocation->getBaseSpeed() + (2 * (level - 1));
			else
				baseSpeed = SPEED_MAX;
		}

		void setNextWalkActionTask(SchedulerTask* task);
		void setNextWalkTask(SchedulerTask* task);
		void setNextActionTask(SchedulerTask* task);

		virtual bool onDeath();
		virtual Item* createCorpse(DeathList deathList);

		virtual void dropCorpse(DeathList deathList);
		virtual void dropLoot(Container* corpse);

		//cylinder implementations
		virtual ReturnValue __queryAdd(int32_t index, const Thing* thing, uint32_t count,
			uint32_t flags, Creature* actor = NULL) const;
		virtual ReturnValue __queryMaxCount(int32_t index, const Thing* thing, uint32_t count, uint32_t& maxQueryCount,
			uint32_t flags) const;
		virtual ReturnValue __queryRemove(const Thing* thing, uint32_t count, uint32_t flags, Creature* actor = NULL) const;
		virtual Cylinder* __queryDestination(int32_t& index, const Thing* thing, Item** destItem,
			uint32_t& flags);

		virtual void __addThing(Creature* actor, Thing* thing);
		virtual void __addThing(Creature* actor, int32_t index, Thing* thing);

		virtual void __updateThing(Thing* thing, uint16_t itemId, uint32_t count);
		virtual void __replaceThing(uint32_t index, Thing* thing);

		virtual void __removeThing(Thing* thing, uint32_t count);

		virtual Thing* __getThing(uint32_t index) const;
		virtual int32_t __getIndexOfThing(const Thing* thing) const;
		virtual int32_t __getFirstIndex() const;
		virtual int32_t __getLastIndex() const;
		virtual uint32_t __getItemTypeCount(uint16_t itemId, int32_t subType = -1) const;
		virtual std::map<uint32_t, uint32_t>& __getAllItemTypeCount(std::map<uint32_t, uint32_t>& countMap) const;

		virtual void __internalAddThing(Thing* thing);
		virtual void __internalAddThing(uint32_t index, Thing* thing);

		uint32_t getVocAttackSpeed() const {return vocation->getAttackSpeed() - getPlayer()->getExtraAttackSpeed();}
		int32_t getStepSpeed() const override
		{
			return std::max<int32_t>(SPEED_MIN, std::min<int32_t>(SPEED_MAX, getSpeed()));
		}

		virtual uint32_t getDamageImmunities() const {return damageImmunities;}
		virtual uint32_t getConditionImmunities() const {return conditionImmunities;}
		virtual uint32_t getConditionSuppressions() const {return conditionSuppressions;}

		virtual uint16_t getLookCorpse() const;
		virtual uint64_t getLostExperience() const;

		virtual void getPathSearchParams(const Creature* creature, FindPathParams& fpp) const;
		static uint16_t getPercentLevel(uint64_t count, uint64_t nextLevelCount);

		bool isPromoted(uint32_t pLevel = 1) const {return promotionLevel >= pLevel;}
		bool hasCapacity(const Item* item, uint32_t count) const;

	private:
		bool talkState[13];
		bool inventoryAbilities[SLOT_LAST];
		bool pzLocked;
		bool saving;
		bool isConnecting;
		bool requestedOutfit;
		bool outfitAttributes;
		bool addAttackSkillPoint;
		bool pvpBlessing;
		bool sentChat;
		static uint32_t playerAutoID;

		OperatingSystem_t operatingSystem;
		AccountManager_t accountManager;
		PlayerSex_t managerSex;
		BlockType_t lastAttackBlockType;
		chaseMode_t chaseMode;
		fightMode_t fightMode;
		secureMode_t secureMode;
		tradestate_t tradeState;
		GuildLevel_t guildLevel;

		int16_t blessings;
		uint16_t maxWriteLen;
		uint16_t sex;
		uint16_t mailAttempts;
		uint16_t lastStatsTrainingTime;

		int32_t premiumDays;
		int32_t soul;
		int32_t soulMax;
		int32_t vocationId;
		int32_t groupId;
		int32_t managerNumber, managerNumber2;
		int32_t purchaseCallback;
		int32_t saleCallback;
		int32_t varSkills[SKILL_LAST + 1];
		int32_t varStats[STAT_LAST + 1];
		int32_t messageBuffer;
		int32_t bloodHitCount;
		int32_t shieldBlockCount;
		int32_t offlineTrainingSkill;
		int32_t offlineTrainingTime;

		uint32_t clientVersion;
		uint32_t messageTicks;
		uint32_t idleTime;
		uint32_t extraAttackSpeed;
		uint32_t accountId;
		uint32_t lastIP;
		uint32_t level;
		uint32_t levelPercent;
		uint32_t magLevel;
		uint32_t magLevelPercent;
		uint32_t damageImmunities;
		uint32_t conditionImmunities;
		uint32_t conditionSuppressions;
		uint32_t nextStepEvent;
		uint32_t actionTaskEvent;
		uint32_t walkTaskEvent;
		uint32_t lossPercent[LOSS_LAST + 1];
		uint32_t guid;
		uint32_t editListId;
		uint32_t windowTextId;
		uint32_t guildId;
		uint32_t rankId;
		uint32_t promotionLevel;
		uint32_t town;

		time_t skullEnd;
		time_t lastLogin;
		time_t lastLogout;

		int64_t lastLoad;
		int64_t lastPong;
		int64_t lastPing;
		int64_t nextAction;
		int64_t nextExAction;
		uint64_t stamina;
		uint64_t experience;
		uint64_t manaSpent;
		uint64_t lastAttack;
		uint64_t lastMail;
		uint64_t skills[SKILL_LAST + 1][3];

		double inventoryWeight;
		double capacity;
		char managerChar[100];

		std::string managerString, managerString2;
		std::string account, password;
		std::string name, nameDescription, specialDescription;
		std::string guildName, rankName, guildNick;

		Position loginPosition;
		LightInfo itemsLight;
		std::pair<Container*, int32_t> backpack;

		Vocation* vocation;
		Spectators* client;
		SchedulerTask* walkTask;
		Party* party;
		Group* group;
		Item* inventory[SLOT_LAST];
		Player* tradePartner;
		Item* tradeItem;
		Item* writeItem;
		House* editHouse;
		Npc* shopOwner;
		Item* weapon;

		std::vector<uint32_t> forceWalkthrough;

		typedef std::set<uint32_t> AttackedSet;
		AttackedSet attackedSet;
		ShopInfoList shopOffer;
		PartyList invitePartyList;
		OutfitMap outfits;
		LearnedInstantSpellList learnedInstantSpellList;
		WarMap warMap;

		friend class Game;
		friend class LuaInterface;
		friend class Npc;
		friend class Actions;
		friend class IOLoginData;
		friend class ProtocolGame;
		friend class ProtocolLogin;
		friend class Spectators;
};
#endif
