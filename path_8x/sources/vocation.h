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

#ifndef __VOCATION__
#define __VOCATION__

#include "otsystem.h"
#include "enums.h"
#include "const.h"

class Vocation
{
	public:
		virtual ~Vocation();

		Vocation() {reset();}
		Vocation(uint32_t _id): id(_id) {reset();}

		void reset();

		uint32_t getId() const {return id;}
		void setId(int32_t v) {id = v;}

		uint16_t getClientId() const {return clientId;}
		void setClientId(uint16_t v) {clientId = v;}

		uint32_t getFromVocation() const {return fromVocation;}
		void setFromVocation(int32_t v) {fromVocation = v;}

		std::string getName() const {return name;}
		void setName(const std::string& v) {name = v;}

		std::string getDescription() const {return description;}
		void setDescription(const std::string& v) {description = v;}

		bool isAttackable() const {return attackable;}
		void setAttackable(bool v) {attackable = v;}

		bool isPremiumNeeded() const {return needPremium;}
		void setNeedPremium(bool v) {needPremium = v;}

		bool getDropLoot() const {return dropLoot;}
		void setDropLoot(bool v) {dropLoot = v;}

		bool getLossSkill() const {return skillLoss;}
		void setLossSkill(bool v) {skillLoss = v;}

		uint32_t getAttackSpeed() const {return attackSpeed;}
		void setAttackSpeed(uint32_t v) {attackSpeed = v;}

		uint32_t getBaseSpeed() const {return baseSpeed;}
		void setBaseSpeed(uint32_t v) {baseSpeed = v;}

		int32_t getLessLoss() const {return lessLoss;}
		void setLessLoss(int32_t v) {lessLoss = v;}

		int32_t getGainCap() const {return capGain;}
		void setGainCap(int32_t v) {capGain = v;}

		uint32_t getGain(gain_t type) const {return gain[type];}
		void setGain(gain_t type, uint32_t v) {gain[type] = v;}

		uint32_t getGainTicks(gain_t type) const {return gainTicks[type];}
		void setGainTicks(gain_t type, uint32_t v) {gainTicks[type] = v;}

		uint32_t getGainAmount(gain_t type) const {return gainAmount[type];}
		void setGainAmount(gain_t type, uint32_t v) {gainAmount[type] = v;}

		float getMultiplier(multiplier_t type) const {return formulaMultipliers[type];}
		void setMultiplier(multiplier_t type, float v) {formulaMultipliers[type] = v;}

		int16_t getAbsorb(CombatType_t combat) const {return absorb[combat];}
		void increaseAbsorb(CombatType_t combat, int16_t v) {absorb[combat] += v;}

		int16_t getReflect(CombatType_t combat) const;
		void increaseReflect(Reflect_t type, CombatType_t combat, int16_t v) {reflect[type][combat] += v;}

		double getExperienceMultiplier() const {return skillMultipliers[SKILL__LEVEL];}
		void setSkillMultiplier(skills_t s, float v) {skillMultipliers[s] = v;}
		void setSkillBase(skills_t s, uint32_t v) {skillBase[s] = v;}

		uint64_t getReqSkillTries(int32_t skill, int32_t level);
		uint64_t getReqMana(uint32_t magLevel);

	private:
		typedef std::map<uint32_t, uint64_t> cacheMap;
		cacheMap cacheSkill[SKILL_LAST + 1];
		cacheMap cacheMana;

		bool attackable, needPremium, dropLoot, skillLoss;
		uint16_t clientId;
		int32_t lessLoss, capGain;
		uint32_t id, fromVocation, baseSpeed, attackSpeed;
		std::string name, description;

		int16_t absorb[COMBAT_LAST + 1], reflect[REFLECT_LAST + 1][COMBAT_LAST + 1];
		uint32_t gain[GAIN_LAST + 1], gainTicks[GAIN_LAST + 1], gainAmount[GAIN_LAST + 1], skillBase[SKILL_LAST + 1];
		float skillMultipliers[SKILL__LAST + 1], formulaMultipliers[MULTIPLIER_LAST + 1];
};

typedef std::map<uint32_t, Vocation*> VocationsMap;
class Vocations
{
	public:
		virtual ~Vocations() {clear();}
		static Vocations* getInstance()
		{
			static Vocations instance;
			return &instance;
		}

		bool reload();
		bool loadFromXml();
		bool parseVocationNode(xmlNodePtr p);

		Vocation* getVocation(uint32_t vocId);
		int32_t getVocationId(const std::string& name);
		int32_t getPromotedVocation(uint32_t vocationId);

		VocationsMap::iterator getFirstVocation() {return vocationsMap.begin();}
		VocationsMap::iterator getLastVocation() {return vocationsMap.end();}

	private:
		static Vocation defVoc;
		VocationsMap vocationsMap;

		Vocations() {}
		void clear();
};
#endif
