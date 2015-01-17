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
#include <iostream>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "vocation.h"
#include "tools.h"

Vocation Vocations::defVoc = Vocation();

void Vocations::clear()
{
	for(VocationsMap::iterator it = vocationsMap.begin(); it != vocationsMap.end(); ++it)
		delete it->second;

	vocationsMap.clear();
}

bool Vocations::reload()
{
	clear();
	return loadFromXml();
}

bool Vocations::parseVocationNode(xmlNodePtr p)
{
	std::string strValue;
	int32_t intValue;
	float floatValue;
	if(xmlStrcmp(p->name, (const xmlChar*)"vocation"))
		return false;

	if(!readXMLInteger(p, "id", intValue))
	{
		std::clog << "[Error - Vocations::parseVocationNode] Missing vocation id." << std::endl;
		return false;
	}

	Vocation* voc = new Vocation(intValue);
	if(readXMLString(p, "name", strValue))
		voc->setName(strValue);

	if(readXMLInteger(p, "clientId", intValue))
		voc->setClientId(intValue);

	if(readXMLString(p, "description", strValue))
		voc->setDescription(strValue);

	if(readXMLString(p, "needpremium", strValue))
		voc->setNeedPremium(booleanString(strValue));

	if(readXMLInteger(p, "gaincap", intValue) || readXMLInteger(p, "gaincapacity", intValue))
		voc->setGainCap(intValue);

	if(readXMLInteger(p, "gainhp", intValue) || readXMLInteger(p, "gainhealth", intValue))
		voc->setGain(GAIN_HEALTH, intValue);

	if(readXMLInteger(p, "gainmana", intValue))
		voc->setGain(GAIN_MANA, intValue);

	if(readXMLInteger(p, "gainhpticks", intValue) || readXMLInteger(p, "gainhealthticks", intValue))
		voc->setGainTicks(GAIN_HEALTH, intValue);

	if(readXMLInteger(p, "gainhpamount", intValue) || readXMLInteger(p, "gainhealthamount", intValue))
		voc->setGainAmount(GAIN_HEALTH, intValue);

	if(readXMLInteger(p, "gainmanaticks", intValue))
		voc->setGainTicks(GAIN_MANA, intValue);

	if(readXMLInteger(p, "gainmanaamount", intValue))
		voc->setGainAmount(GAIN_MANA, intValue);

	if(readXMLFloat(p, "manamultiplier", floatValue))
	{
		if(floatValue >= 1.0f)
			voc->setMultiplier(MULTIPLIER_MANA, floatValue);
		else
			std::clog << "[Error - Vocations::parseVocationNode] Mana multiplier must be equal or greater than 1." << std::endl;
	}

	if(readXMLInteger(p, "attackspeed", intValue))
		voc->setAttackSpeed(intValue);

	if(readXMLInteger(p, "basespeed", intValue))
		voc->setBaseSpeed(intValue);

	if(readXMLInteger(p, "soulmax", intValue))
		voc->setGain(GAIN_SOUL, intValue);

	if(readXMLInteger(p, "gainsoulamount", intValue))
		voc->setGainAmount(GAIN_SOUL, intValue);

	if(readXMLInteger(p, "gainsoulticks", intValue))
		voc->setGainTicks(GAIN_SOUL, intValue);

	if(readXMLString(p, "attackable", strValue))
		voc->setAttackable(booleanString(strValue));

	if(readXMLInteger(p, "fromvoc", intValue) || readXMLInteger(p, "fromvocation", intValue))
		voc->setFromVocation(intValue);

	if(readXMLInteger(p, "lessloss", intValue))
		voc->setLessLoss(intValue);

	if(readXMLString(p, "droploot", strValue) || readXMLString(p, "lootdrop", strValue))
		voc->setDropLoot(booleanString(strValue));

	if(readXMLString(p, "skillloss", strValue) || readXMLString(p, "lossskill", strValue))
		voc->setLossSkill(booleanString(strValue));

	for(xmlNodePtr configNode = p->children; configNode; configNode = configNode->next)
	{
		if(!xmlStrcmp(configNode->name, (const xmlChar*)"skill"))
		{
			if(readXMLFloat(configNode, "fist", floatValue))
				voc->setSkillMultiplier(SKILL_FIST, floatValue);

			if(readXMLInteger(configNode, "fistBase", intValue))
				voc->setSkillBase(SKILL_FIST, intValue);

			if(readXMLFloat(configNode, "club", floatValue))
				voc->setSkillMultiplier(SKILL_CLUB, floatValue);

			if(readXMLInteger(configNode, "clubBase", intValue))
				voc->setSkillBase(SKILL_CLUB, intValue);

			if(readXMLFloat(configNode, "axe", floatValue))
				voc->setSkillMultiplier(SKILL_AXE, floatValue);

			if(readXMLInteger(configNode, "axeBase", intValue))
				voc->setSkillBase(SKILL_AXE, intValue);

			if(readXMLFloat(configNode, "sword", floatValue))
				voc->setSkillMultiplier(SKILL_SWORD, floatValue);

			if(readXMLInteger(configNode, "swordBase", intValue))
				voc->setSkillBase(SKILL_SWORD, intValue);

			if(readXMLFloat(configNode, "distance", floatValue) || readXMLFloat(configNode, "dist", floatValue))
				voc->setSkillMultiplier(SKILL_DIST, floatValue);

			if(readXMLInteger(configNode, "distanceBase", intValue) || readXMLInteger(configNode, "distBase", intValue))
				voc->setSkillBase(SKILL_DIST, intValue);

			if(readXMLFloat(configNode, "shielding", floatValue) || readXMLFloat(configNode, "shield", floatValue))
				voc->setSkillMultiplier(SKILL_SHIELD, floatValue);

			if(readXMLInteger(configNode, "shieldingBase", intValue) || readXMLInteger(configNode, "shieldBase", intValue))
				voc->setSkillBase(SKILL_SHIELD, intValue);

			if(readXMLFloat(configNode, "fishing", floatValue) || readXMLFloat(configNode, "fish", floatValue))
				voc->setSkillMultiplier(SKILL_FISH, floatValue);

			if(readXMLInteger(configNode, "fishingBase", intValue) || readXMLInteger(configNode, "fishBase", intValue))
				voc->setSkillBase(SKILL_FISH, intValue);

			if(readXMLFloat(configNode, "experience", floatValue) || readXMLFloat(configNode, "exp", floatValue))
				voc->setSkillMultiplier(SKILL__LEVEL, floatValue);

			if(readXMLInteger(configNode, "id", intValue))
			{
				skills_t skill = (skills_t)intValue;
				if(skill < SKILL_FIRST || skill >= SKILL__LAST)
				{
					std::clog << "[Error - Vocations::parseVocationNode] No valid skill id (" << intValue << ")." << std::endl;
					continue;
				}

				if(readXMLInteger(configNode, "base", intValue))
					voc->setSkillBase(skill, intValue);

				if(readXMLFloat(configNode, "multiplier", floatValue))
					voc->setSkillMultiplier(skill, floatValue);
			}
		}
		else if(!xmlStrcmp(configNode->name, (const xmlChar*)"formula"))
		{
			if(readXMLFloat(configNode, "meleeDamage", floatValue))
				voc->setMultiplier(MULTIPLIER_MELEE, floatValue);

			if(readXMLFloat(configNode, "distDamage", floatValue) || readXMLFloat(configNode, "distanceDamage", floatValue))
				voc->setMultiplier(MULTIPLIER_DISTANCE, floatValue);

			if(readXMLFloat(configNode, "wandDamage", floatValue) || readXMLFloat(configNode, "rodDamage", floatValue))
				voc->setMultiplier(MULTIPLIER_WAND, floatValue);

			if(readXMLFloat(configNode, "magDamage", floatValue) || readXMLFloat(configNode, "magicDamage", floatValue))
				voc->setMultiplier(MULTIPLIER_MAGIC, floatValue);

			if(readXMLFloat(configNode, "magHealingDamage", floatValue) || readXMLFloat(configNode, "magicHealingDamage", floatValue))
				voc->setMultiplier(MULTIPLIER_HEALING, floatValue);

			if(readXMLFloat(configNode, "defense", floatValue))
				voc->setMultiplier(MULTIPLIER_DEFENSE, floatValue);

			if(readXMLFloat(configNode, "magDefense", floatValue) || readXMLFloat(configNode, "magicDefense", floatValue))
				voc->setMultiplier(MULTIPLIER_MAGICDEFENSE, floatValue);

			if(readXMLFloat(configNode, "armor", floatValue))
				voc->setMultiplier(MULTIPLIER_ARMOR, floatValue);
		}
		else if(!xmlStrcmp(configNode->name, (const xmlChar*)"absorb"))
		{
			if(readXMLInteger(configNode, "percentAll", intValue))
			{
				for(uint32_t i = (COMBAT_FIRST + 1); i <= COMBAT_LAST; i <<= 1)
					voc->increaseAbsorb((CombatType_t)i, intValue);
			}

			if(readXMLInteger(configNode, "percentElements", intValue))
			{
				voc->increaseAbsorb(COMBAT_ENERGYDAMAGE, intValue);
				voc->increaseAbsorb(COMBAT_FIREDAMAGE, intValue);
				voc->increaseAbsorb(COMBAT_EARTHDAMAGE, intValue);
				voc->increaseAbsorb(COMBAT_ICEDAMAGE, intValue);
			}

			if(readXMLInteger(configNode, "percentMagic", intValue))
			{
				voc->increaseAbsorb(COMBAT_ENERGYDAMAGE, intValue);
				voc->increaseAbsorb(COMBAT_FIREDAMAGE, intValue);
				voc->increaseAbsorb(COMBAT_EARTHDAMAGE, intValue);
				voc->increaseAbsorb(COMBAT_ICEDAMAGE, intValue);
				voc->increaseAbsorb(COMBAT_HOLYDAMAGE, intValue);
				voc->increaseAbsorb(COMBAT_DEATHDAMAGE, intValue);
			}

			if(readXMLInteger(configNode, "percentEnergy", intValue))
				voc->increaseAbsorb(COMBAT_ENERGYDAMAGE, intValue);

			if(readXMLInteger(configNode, "percentFire", intValue))
				voc->increaseAbsorb(COMBAT_FIREDAMAGE, intValue);

			if(readXMLInteger(configNode, "percentPoison", intValue) || readXMLInteger(configNode, "percentEarth", intValue))
				voc->increaseAbsorb(COMBAT_EARTHDAMAGE, intValue);

			if(readXMLInteger(configNode, "percentIce", intValue))
				voc->increaseAbsorb(COMBAT_ICEDAMAGE, intValue);

			if(readXMLInteger(configNode, "percentHoly", intValue))
				voc->increaseAbsorb(COMBAT_HOLYDAMAGE, intValue);

			if(readXMLInteger(configNode, "percentDeath", intValue))
				voc->increaseAbsorb(COMBAT_DEATHDAMAGE, intValue);

			if(readXMLInteger(configNode, "percentLifeDrain", intValue))
				voc->increaseAbsorb(COMBAT_LIFEDRAIN, intValue);

			if(readXMLInteger(configNode, "percentManaDrain", intValue))
				voc->increaseAbsorb(COMBAT_MANADRAIN, intValue);

			if(readXMLInteger(configNode, "percentDrown", intValue))
				voc->increaseAbsorb(COMBAT_DROWNDAMAGE, intValue);

			if(readXMLInteger(configNode, "percentPhysical", intValue))
				voc->increaseAbsorb(COMBAT_PHYSICALDAMAGE, intValue);

			if(readXMLInteger(configNode, "percentHealing", intValue))
				voc->increaseAbsorb(COMBAT_HEALING, intValue);

			if(readXMLInteger(configNode, "percentUndefined", intValue))
				voc->increaseAbsorb(COMBAT_UNDEFINEDDAMAGE, intValue);
		}
		else if(!xmlStrcmp(configNode->name, (const xmlChar*)"reflect"))
		{
			if(readXMLInteger(configNode, "percentAll", intValue))
			{
				for(uint32_t i = (COMBAT_FIRST + 1); i <= COMBAT_LAST; i <<= 1)
					voc->increaseReflect(REFLECT_PERCENT, (CombatType_t)i, intValue);
			}

			if(readXMLInteger(configNode, "percentElements", intValue))
			{
				voc->increaseReflect(REFLECT_PERCENT, COMBAT_ENERGYDAMAGE, intValue);
				voc->increaseReflect(REFLECT_PERCENT, COMBAT_FIREDAMAGE, intValue);
				voc->increaseReflect(REFLECT_PERCENT, COMBAT_EARTHDAMAGE, intValue);
				voc->increaseReflect(REFLECT_PERCENT, COMBAT_ICEDAMAGE, intValue);
			}

			if(readXMLInteger(configNode, "percentMagic", intValue))
			{
				voc->increaseReflect(REFLECT_PERCENT, COMBAT_ENERGYDAMAGE, intValue);
				voc->increaseReflect(REFLECT_PERCENT, COMBAT_FIREDAMAGE, intValue);
				voc->increaseReflect(REFLECT_PERCENT, COMBAT_EARTHDAMAGE, intValue);
				voc->increaseReflect(REFLECT_PERCENT, COMBAT_ICEDAMAGE, intValue);
				voc->increaseReflect(REFLECT_PERCENT, COMBAT_HOLYDAMAGE, intValue);
				voc->increaseReflect(REFLECT_PERCENT, COMBAT_DEATHDAMAGE, intValue);
			}

			if(readXMLInteger(configNode, "percentEnergy", intValue))
				voc->increaseReflect(REFLECT_PERCENT, COMBAT_ENERGYDAMAGE, intValue);

			if(readXMLInteger(configNode, "percentFire", intValue))
				voc->increaseReflect(REFLECT_PERCENT, COMBAT_FIREDAMAGE, intValue);

			if(readXMLInteger(configNode, "percentPoison", intValue) || readXMLInteger(configNode, "percentEarth", intValue))
				voc->increaseReflect(REFLECT_PERCENT, COMBAT_EARTHDAMAGE, intValue);

			if(readXMLInteger(configNode, "percentIce", intValue))
				voc->increaseReflect(REFLECT_PERCENT, COMBAT_ICEDAMAGE, intValue);

			if(readXMLInteger(configNode, "percentHoly", intValue))
				voc->increaseReflect(REFLECT_PERCENT, COMBAT_HOLYDAMAGE, intValue);

			if(readXMLInteger(configNode, "percentDeath", intValue))
				voc->increaseReflect(REFLECT_PERCENT, COMBAT_DEATHDAMAGE, intValue);

			if(readXMLInteger(configNode, "percentLifeDrain", intValue))
				voc->increaseReflect(REFLECT_PERCENT, COMBAT_LIFEDRAIN, intValue);

			if(readXMLInteger(configNode, "percentManaDrain", intValue))
				voc->increaseReflect(REFLECT_PERCENT, COMBAT_MANADRAIN, intValue);

			if(readXMLInteger(configNode, "percentDrown", intValue))
				voc->increaseReflect(REFLECT_PERCENT, COMBAT_DROWNDAMAGE, intValue);

			if(readXMLInteger(configNode, "percentPhysical", intValue))
				voc->increaseReflect(REFLECT_PERCENT, COMBAT_PHYSICALDAMAGE, intValue);

			if(readXMLInteger(configNode, "percentHealing", intValue))
				voc->increaseReflect(REFLECT_PERCENT, COMBAT_HEALING, intValue);

			if(readXMLInteger(configNode, "percentUndefined", intValue))
				voc->increaseReflect(REFLECT_PERCENT, COMBAT_UNDEFINEDDAMAGE, intValue);

			if(readXMLInteger(configNode, "chanceAll", intValue))
			{
				for(uint32_t i = (COMBAT_FIRST + 1); i <= COMBAT_LAST; i <<= 1)
					voc->increaseReflect(REFLECT_CHANCE, (CombatType_t)i, intValue);
			}

			if(readXMLInteger(configNode, "chanceElements", intValue))
			{
				voc->increaseReflect(REFLECT_CHANCE, COMBAT_ENERGYDAMAGE, intValue);
				voc->increaseReflect(REFLECT_CHANCE, COMBAT_FIREDAMAGE, intValue);
				voc->increaseReflect(REFLECT_CHANCE, COMBAT_EARTHDAMAGE, intValue);
				voc->increaseReflect(REFLECT_CHANCE, COMBAT_ICEDAMAGE, intValue);
			}

			if(readXMLInteger(configNode, "chanceMagic", intValue))
			{
				voc->increaseReflect(REFLECT_CHANCE, COMBAT_ENERGYDAMAGE, intValue);
				voc->increaseReflect(REFLECT_CHANCE, COMBAT_FIREDAMAGE, intValue);
				voc->increaseReflect(REFLECT_CHANCE, COMBAT_EARTHDAMAGE, intValue);
				voc->increaseReflect(REFLECT_CHANCE, COMBAT_ICEDAMAGE, intValue);
				voc->increaseReflect(REFLECT_CHANCE, COMBAT_HOLYDAMAGE, intValue);
				voc->increaseReflect(REFLECT_CHANCE, COMBAT_DEATHDAMAGE, intValue);
			}

			if(readXMLInteger(configNode, "chanceEnergy", intValue))
				voc->increaseReflect(REFLECT_CHANCE, COMBAT_ENERGYDAMAGE, intValue);

			if(readXMLInteger(configNode, "chanceFire", intValue))
				voc->increaseReflect(REFLECT_CHANCE, COMBAT_FIREDAMAGE, intValue);

			if(readXMLInteger(configNode, "chancePoison", intValue) || readXMLInteger(configNode, "chanceEarth", intValue))
				voc->increaseReflect(REFLECT_CHANCE, COMBAT_EARTHDAMAGE, intValue);

			if(readXMLInteger(configNode, "chanceIce", intValue))
				voc->increaseReflect(REFLECT_CHANCE, COMBAT_ICEDAMAGE, intValue);

			if(readXMLInteger(configNode, "chanceHoly", intValue))
				voc->increaseReflect(REFLECT_CHANCE, COMBAT_HOLYDAMAGE, intValue);

			if(readXMLInteger(configNode, "chanceDeath", intValue))
				voc->increaseReflect(REFLECT_CHANCE, COMBAT_DEATHDAMAGE, intValue);

			if(readXMLInteger(configNode, "chanceLifeDrain", intValue))
				voc->increaseReflect(REFLECT_CHANCE, COMBAT_LIFEDRAIN, intValue);

			if(readXMLInteger(configNode, "chanceManaDrain", intValue))
				voc->increaseReflect(REFLECT_CHANCE, COMBAT_MANADRAIN, intValue);

			if(readXMLInteger(configNode, "chanceDrown", intValue))
				voc->increaseReflect(REFLECT_CHANCE, COMBAT_DROWNDAMAGE, intValue);

			if(readXMLInteger(configNode, "chancePhysical", intValue))
				voc->increaseReflect(REFLECT_CHANCE, COMBAT_PHYSICALDAMAGE, intValue);

			if(readXMLInteger(configNode, "chanceHealing", intValue))
				voc->increaseReflect(REFLECT_CHANCE, COMBAT_HEALING, intValue);

			if(readXMLInteger(configNode, "chanceUndefined", intValue))
				voc->increaseReflect(REFLECT_CHANCE, COMBAT_UNDEFINEDDAMAGE, intValue);
		}
	}

	vocationsMap[voc->getId()] = voc;
	return true;
}

bool Vocations::loadFromXml()
{
	xmlDocPtr doc = xmlParseFile(getFilePath(FILE_TYPE_XML,"vocations.xml").c_str());
	if(!doc)
	{
		std::clog << "[Warning - Vocations::loadFromXml] Cannot load vocations file." << std::endl;
		std::clog << getLastXMLError() << std::endl;
		return false;
	}

	xmlNodePtr root = xmlDocGetRootElement(doc);
	if(xmlStrcmp(root->name,(const xmlChar*)"vocations"))
	{
		std::clog << "[Error - Vocations::loadFromXml] Malformed vocations file." << std::endl;
		xmlFreeDoc(doc);
		return false;
	}

	for(xmlNodePtr p = root->children; p; p = p->next)
		parseVocationNode(p);

	xmlFreeDoc(doc);
	return true;
}

Vocation* Vocations::getVocation(uint32_t vocId)
{
	VocationsMap::iterator it = vocationsMap.find(vocId);
	if(it != vocationsMap.end())
		return it->second;

	std::clog << "[Warning - Vocations::getVocation] Vocation " << vocId << " not found." << std::endl;
	return &Vocations::defVoc;
}

int32_t Vocations::getVocationId(const std::string& name)
{
	for(VocationsMap::iterator it = vocationsMap.begin(); it != vocationsMap.end(); ++it)
	{
		if(boost::algorithm::iequals(it->second->getName(), name))
			return it->first;
	}

	return -1;
}

int32_t Vocations::getPromotedVocation(uint32_t vocationId)
{
	for(VocationsMap::iterator it = vocationsMap.begin(); it != vocationsMap.end(); ++it)
	{
		if(it->second->getFromVocation() == vocationId && it->first != vocationId)
			return it->first;
	}

	return -1;
}

Vocation::~Vocation()
{
	cacheMana.clear();
	for(int32_t i = SKILL_FIRST; i < SKILL_LAST; ++i)
		cacheSkill[i].clear();
}

void Vocation::reset()
{
	memset(absorb, 0, sizeof(absorb));
	memset(reflect[REFLECT_PERCENT], 0, sizeof(reflect[REFLECT_PERCENT]));
	memset(reflect[REFLECT_CHANCE], 0, sizeof(reflect[REFLECT_CHANCE]));

	needPremium = false;
	attackable = dropLoot = skillLoss = true;
	lessLoss = fromVocation = 0;
	clientId = 0;
	gain[GAIN_SOUL] = 100;
	gainTicks[GAIN_SOUL] = 120;
	baseSpeed = 220;
	attackSpeed = 1500;
	name = description = "";

	gainAmount[GAIN_HEALTH] = gainAmount[GAIN_MANA] = gainAmount[GAIN_SOUL] = 1;
	gain[GAIN_HEALTH] = gain[GAIN_MANA] = capGain = 5;
	gainTicks[GAIN_HEALTH] = gainTicks[GAIN_MANA] = 6;

	skillBase[SKILL_SHIELD] = 100;
	skillBase[SKILL_DIST] = 30;
	skillBase[SKILL_FISH] = 20;
	for(int32_t i = SKILL_FIST; i < SKILL_DIST; ++i)
		skillBase[i] = 50;

	skillMultipliers[SKILL_FIST] = 1.5f;
	skillMultipliers[SKILL_FISH] = 1.1f;
	skillMultipliers[SKILL__LEVEL] = 1.0f;
	for(int32_t i = SKILL_CLUB; i < SKILL_FISH; ++i)
		skillMultipliers[i] = 2.0f;

	formulaMultipliers[MULTIPLIER_MANA] = 4.0f;
	for(int32_t i = MULTIPLIER_FIRST; i < MULTIPLIER_LAST; ++i)
		formulaMultipliers[i] = 1.0f;
}

int16_t Vocation::getReflect(CombatType_t combat) const
{
	if(reflect[REFLECT_CHANCE][combat] >= random_range(1, 100))
		return reflect[REFLECT_PERCENT][combat];

	return 0;
}

uint64_t Vocation::getReqSkillTries(int32_t skill, int32_t level)
{
	if(skill < SKILL_FIRST || skill > SKILL_LAST)
		return 0;

	cacheMap& skillMap = cacheSkill[skill];
	cacheMap::iterator it = skillMap.find(level);
	if(it != skillMap.end())
		return it->second;

	skillMap[level] = (uint64_t)(skillBase[skill] * std::pow(skillMultipliers[skill], (level - 11)));
	return skillMap[level];
}

uint64_t Vocation::getReqMana(uint32_t magLevel)
{
	if(!magLevel)
		return 0;

	cacheMap::iterator it = cacheMana.find(magLevel);
	if(it != cacheMana.end())
		return it->second;

	cacheMana[magLevel] = (uint64_t)(1600 * std::pow(formulaMultipliers[MULTIPLIER_MANA], (float)(magLevel - 1)));
	return cacheMana[magLevel];
}
