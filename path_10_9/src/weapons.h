/**
 * The Forgotten Server - a free and open-source MMORPG server emulator
 * Copyright (C) 2016  Mark Samman <mark.samman@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef FS_WEAPONS_H_69D1993478AA42948E24C0B90B8F5BF5
#define FS_WEAPONS_H_69D1993478AA42948E24C0B90B8F5BF5

#include "luascript.h"
#include "player.h"
#include "baseevents.h"
#include "combat.h"
#include "const.h"
#include "vocation.h"

extern Vocations g_vocations;

class Weapon;
class WeaponMelee;
class WeaponDistance;
class WeaponWand;

typedef std::map<uint16_t, bool> VocWeaponMap;

class Weapons final : public BaseEvents
{
	public:
		Weapons();
		~Weapons();

		// non-copyable
		Weapons(const Weapons&) = delete;
		Weapons& operator=(const Weapons&) = delete;

		void loadDefaults();
		const Weapon* getWeapon(const Item* item) const;

		static int32_t getMaxMeleeDamage(int32_t attackSkill, int32_t attackValue);
		static int32_t getMaxWeaponDamage(uint32_t level, int32_t attackSkill, int32_t attackValue, float attackFactor);

		bool registerLuaEvent(Event* event);

	protected:
		void clear() final;
		LuaScriptInterface& getScriptInterface() final;
		std::string getScriptBaseName() const final;
		Event* getEvent(const std::string& nodeName) final;
		bool registerEvent(Event* event, const pugi::xml_node& node) final;

		std::map<uint32_t, Weapon*> weapons;

		LuaScriptInterface scriptInterface;
};

class Weapon : public Event
{
	public:
		explicit Weapon(LuaScriptInterface* interface);

		bool configureEvent(const pugi::xml_node& node) override;
		bool loadFunction(const pugi::xml_attribute&, bool) final {
			return true;
		}
		virtual void configureWeapon(const ItemType& it);
		virtual bool interruptSwing() const {
			return false;
		}

		int32_t playerWeaponCheck(Player* player, Creature* target, uint8_t shootRange) const;
		static bool useFist(Player* player, Creature* target);
		virtual bool useWeapon(Player* player, Item* item, Creature* target) const;

		virtual int32_t getWeaponDamage(const Player* player, const Creature* target, const Item* item, bool maxDamage = false) const = 0;
		virtual int32_t getElementDamage(const Player* player, const Creature* target, const Item* item) const = 0;
		virtual CombatType_t getElementType() const = 0;

		uint16_t getID() const {
			return id;
		}
		void setID(uint16_t newId) {
			id = newId;
		}

		uint32_t getReqLevel() const {
			return level;
		}
		void setRequiredLevel(uint32_t reqlvl) {
			level = reqlvl;
		}

		uint32_t getReqMagLv() const {
			return magLevel;
		}
		void setRequiredMagLevel(uint32_t reqlvl) {
			magLevel = reqlvl;
		}

		bool isPremium() const {
			return premium;
		}
		void setNeedPremium(bool prem) {
			premium = prem;
		}

		bool isWieldedUnproperly() const {
			return wieldUnproperly;
		}
		void setWieldUnproperly(bool unproperly) {
			wieldUnproperly = unproperly;
		}

		uint32_t getMana() const {
			return mana;
		}
		void setMana(uint32_t m) {
			mana = m;
		}

		uint32_t getManaPercent() const {
			return manaPercent;
		}
		void setManaPercent(uint32_t m) {
			manaPercent = m;
		}

		int32_t getHealth() const {
			return health;
		}
		void setHealth(int32_t h) {
			health = h;
		}

		uint32_t getHealthPercent() const {
			return healthPercent;
		}
		void setHealthPercent(uint32_t m) {
			healthPercent = m;
		}

		uint32_t getSoul() const {
			return soul;
		}
		void setSoul(uint32_t s) {
			soul = s;
		}

		uint8_t getBreakChance() const {
			return breakChance;
		}
		void setBreakChance(uint8_t b) {
			breakChance = b;
		}

		bool isEnabled() const {
			return enabled;
		}
		void setIsEnabled(bool e) {
			enabled = e;
		}

		uint32_t getWieldInfo() const {
			return wieldInfo;
		}
		void setWieldInfo(uint32_t info) {
			wieldInfo |= info;
		}

		const VocWeaponMap& getVocWeaponMap() const {
			return vocWeaponMap;
		}
		void addVocWeaponMap(std::string vocName) {
			int32_t vocationId = g_vocations.getVocationId(vocName);
			if (vocationId != -1) {
				vocWeaponMap[vocationId] = true;
			}
		}

		const std::string& getVocationString() const {
			return vocationString;
		}
		void setVocationString(const std::string& str) {
			vocationString = str;
		}

		WeaponAction_t action;
		CombatParams params;

	protected:
		std::string getScriptEventName() const final;

		bool executeUseWeapon(Player* player, const LuaVariant& var) const;
		void internalUseWeapon(Player* player, Item* item, Creature* target, int32_t damageModifier) const;
		void internalUseWeapon(Player* player, Item* item, Tile* tile) const;

		void onUsedWeapon(Player* player, Item* item, Tile* destTile) const;
		virtual bool getSkillType(const Player*, const Item*, skills_t&, uint32_t&) const {
			return false;
		}

		uint32_t getManaCost(const Player* player) const;
		int32_t getHealthCost(const Player* player) const;

		uint32_t level;
		uint32_t magLevel;
		uint32_t mana;
		uint32_t manaPercent;
		int32_t health;
		int32_t healthPercent;
		uint32_t soul;
		uint32_t wieldInfo;
		uint16_t id;
		uint8_t breakChance;
		bool enabled;
		bool premium;
		bool wieldUnproperly;
		std::string vocationString;

	private:
		static void decrementItemCount(Item* item);

		VocWeaponMap vocWeaponMap;
		friend class Combat;
};

class WeaponMelee final : public Weapon
{
	public:
		explicit WeaponMelee(LuaScriptInterface* interface);

		void configureWeapon(const ItemType& it) final;

		bool useWeapon(Player* player, Item* item, Creature* target) const final;

		int32_t getWeaponDamage(const Player* player, const Creature* target, const Item* item, bool maxDamage = false) const final;
		int32_t getElementDamage(const Player* player, const Creature* target, const Item* item) const final;
		CombatType_t getElementType() const final { return elementType; }

	protected:
		bool getSkillType(const Player* player, const Item* item, skills_t& skill, uint32_t& skillpoint) const final;

		CombatType_t elementType;
		uint16_t elementDamage;
};

class WeaponDistance final : public Weapon
{
	public:
		explicit WeaponDistance(LuaScriptInterface* interface);

		void configureWeapon(const ItemType& it) final;
		bool interruptSwing() const final {
			return true;
		}

		bool useWeapon(Player* player, Item* item, Creature* target) const final;

		int32_t getWeaponDamage(const Player* player, const Creature* target, const Item* item, bool maxDamage = false) const final;
		int32_t getElementDamage(const Player* player, const Creature* target, const Item* item) const final;
		CombatType_t getElementType() const final { return elementType; }

	protected:
		bool getSkillType(const Player* player, const Item* item, skills_t& skill, uint32_t& skillpoint) const final;

		CombatType_t elementType;
		uint16_t elementDamage;
};

class WeaponWand final : public Weapon
{
	public:
		explicit WeaponWand(LuaScriptInterface* interface);

		bool configureEvent(const pugi::xml_node& node) final;
		void configureWeapon(const ItemType& it) final;

		int32_t getWeaponDamage(const Player* player, const Creature* target, const Item* item, bool maxDamage = false) const final;
		int32_t getElementDamage(const Player*, const Creature*, const Item*) const final { return 0; }
		CombatType_t getElementType() const final { return COMBAT_NONE; }

		void setMinChange(int32_t change) {
			minChange = change;
		}

		void setMaxChange(int32_t change) {
			maxChange = change;
		}

	protected:
		bool getSkillType(const Player*, const Item*, skills_t&, uint32_t&) const final {
			return false;
		}

		int32_t minChange;
		int32_t maxChange;
};

#endif
