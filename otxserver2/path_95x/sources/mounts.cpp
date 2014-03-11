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
#include "mounts.h"

bool Mount::isTamed(Player* player) const
{
	if(!player)
		return false;

	if(player->hasCustomFlag(PlayerCustomFlag_CanUseAllMounts))
		return true;

	if(premium && !player->isPremium())
		return false;

	uint8_t tmpId = id - 1;
	std::string value;

	int32_t key = PSTRG_MOUNTS_RANGE_START + (tmpId / 31);
	if(player->getStorage(asString(key), value))
	{
		int32_t tmp = (int32_t)std::pow(2., tmpId % 31);
		return (tmp & atoi(value.c_str())) == tmp;
	}

	if(storageId.empty())
		return false;

	player->getStorage(storageId, value);
	if(value == storageValue)
		return true;

	int32_t intValue = atoi(value.c_str());
	if(!intValue && value != "0")
		return false;

	int32_t tmp = atoi(storageValue.c_str());
	if(!tmp && storageValue != "0")
		return false;

	return intValue >= tmp;
}

void Mount::addAttributes(Player* player)
{
	if(invisible)
	{
		Condition* condition = Condition::createCondition(CONDITIONID_MOUNT, CONDITION_INVISIBLE, -1, 0);
		player->addCondition(condition);
	}

	if(manaShield)
	{
		Condition* condition = Condition::createCondition(CONDITIONID_MOUNT, CONDITION_MANASHIELD, -1, 0);
		player->addCondition(condition);
	}

	if(conditionSuppressions)
	{
		player->setConditionSuppressions(conditionSuppressions, false);
		player->sendIcons();
	}

	if(regeneration)
	{
		Condition* condition = Condition::createCondition(CONDITIONID_MOUNT, CONDITION_REGENERATION, -1, 0);
		if(healthGain)
			condition->setParam(CONDITIONPARAM_HEALTHGAIN, healthGain);

		if(healthTicks)
			condition->setParam(CONDITIONPARAM_HEALTHTICKS, healthTicks);

		if(manaGain)
			condition->setParam(CONDITIONPARAM_MANAGAIN, manaGain);

		if(manaTicks)
			condition->setParam(CONDITIONPARAM_MANATICKS, manaTicks);

		player->addCondition(condition);
	}

	bool needUpdateSkills = false;
	for(uint32_t i = SKILL_FIRST; i <= SKILL_LAST; ++i)
	{
		if(skills[i])
		{
			needUpdateSkills = true;
			player->setVarSkill((skills_t)i, skills[i]);
		}

		if(skillsPercent[i])
		{
			needUpdateSkills = true;
			player->setVarSkill((skills_t)i, (int32_t)(player->getSkill((skills_t)i, SKILL_LEVEL) * ((skillsPercent[i] - 100) / 100.f)));
		}
	}

	if(needUpdateSkills)
		player->sendSkills();

	bool needUpdateStats = false;
	for(uint32_t s = STAT_FIRST; s <= STAT_LAST; ++s)
	{
		if(stats[s])
		{
			needUpdateStats = true;
			player->setVarStats((stats_t)s, stats[s]);
		}

		if(statsPercent[s])
		{
			needUpdateStats = true;
			player->setVarStats((stats_t)s, (int32_t)(player->getDefaultStats((stats_t)s) * ((statsPercent[s] - 100) / 100.f)));
		}
	}

	if(needUpdateStats)
		player->sendStats();
}

void Mount::removeAttributes(Player* player)
{
	if(invisible)
		player->removeCondition(CONDITION_INVISIBLE, CONDITIONID_MOUNT);

	if(manaShield)
		player->removeCondition(CONDITION_MANASHIELD, CONDITIONID_MOUNT);

	if(conditionSuppressions)
	{
		player->setConditionSuppressions(conditionSuppressions, true);
		player->sendIcons();
	}

	if(regeneration)
		player->removeCondition(CONDITION_REGENERATION, CONDITIONID_MOUNT);

	bool needUpdateSkills = false;
	for(uint32_t i = SKILL_FIRST; i <= SKILL_LAST; ++i)
	{
		if(skills[i])
		{
			needUpdateSkills = true;
			player->setVarSkill((skills_t)i, -skills[i]);
		}

		if(skillsPercent[i])
		{
			needUpdateSkills = true;
			player->setVarSkill((skills_t)i, -(int32_t)(player->getSkill((skills_t)i, SKILL_LEVEL) * ((skillsPercent[i] - 100) / 100.f)));
		}
	}

	if(needUpdateSkills)
		player->sendSkills();

	bool needUpdateStats = false;
	for(uint32_t s = STAT_FIRST; s <= STAT_LAST; ++s)
	{
		if(stats[s])
		{
			needUpdateStats = true;
			player->setVarStats((stats_t)s, -stats[s]);
		}

		if(statsPercent[s])
		{
			needUpdateStats = true;
			player->setVarStats((stats_t)s, -(int32_t)(player->getDefaultStats((stats_t)s) * ((statsPercent[s] - 100) / 100.f)));
		}
	}

	if(needUpdateStats)
		player->sendStats();
}

void Mounts::clear()
{
	for(MountList::iterator it = mounts.begin(); it != mounts.end(); ++it)
		delete *it;

	mounts.clear();
}

bool Mounts::reload()
{
	clear();
	return loadFromXml();
}

bool Mounts::loadFromXml()
{
	xmlDocPtr doc = xmlParseFile(getFilePath(FILE_TYPE_XML, "mounts.xml").c_str());
	if(!doc)
	{
		std::clog << "[Warning - Mounts::loadFromXml] Cannot load mounts file." << std::endl;
		std::clog << getLastXMLError() << std::endl;
		return false;
	}

	xmlNodePtr root = xmlDocGetRootElement(doc);
	if(xmlStrcmp(root->name, (const xmlChar*)"mounts"))
	{
		std::clog << "[Error - Mounts::loadFromXml] Malformed mounts file." << std::endl;
		xmlFreeDoc(doc);
		return false;
	}

	for(xmlNodePtr p = root->children; p; p = p->next)
		parseMountNode(p);

	xmlFreeDoc(doc);
	return true;
}

bool Mounts::parseMountNode(xmlNodePtr p)
{
	if(xmlStrcmp(p->name, (const xmlChar*)"mount"))
		return false;

	int32_t intValue;
	std::string strValue;

	uint8_t mountId = 0;
	if(readXMLInteger(p, "id", intValue))
		mountId = intValue;

	std::string name;
	if(readXMLString(p, "name", strValue))
		name = strValue;

	uint16_t clientId = 0;
	if(readXMLInteger(p, "clientid", intValue) || readXMLInteger(p, "clientId", intValue) || readXMLInteger(p, "cid", intValue))
		clientId = intValue;

	int32_t speed = 0;
	if(readXMLInteger(p, "speed", intValue))
		speed = intValue;

	int32_t attackSpeed = 0;
	if(readXMLInteger(p, "attackspeed", intValue) || readXMLInteger(p, "attackSpeed", intValue))
		attackSpeed = intValue;

	bool premium = true;
	if(readXMLString(p, "premium", strValue))
		premium = booleanString(strValue);

	std::string storageId, storageValue;
	if(readXMLString(p, "quest", strValue))
	{
		storageId = strValue;
		storageValue = "1";
	}
	else
	{
		if(readXMLString(p, "storageId", strValue))
			storageId = strValue;

		if(readXMLString(p, "storageValue", strValue))
			storageValue = strValue;
	}

	if(!mountId)
	{
		std::clog << "[Error - Mounts::parseMountNode] Entry without mountId" << std::endl;
		return false;
	}

	if(!clientId)
	{
		std::clog << "[Error - Mounts::parseMountNode] Entry without clientId" << std::endl;
		return false;
	}

	Mount* mount = new Mount(name, mountId, clientId,
		speed, attackSpeed, premium, storageId, storageValue);
	if(!mount)
		return false;

	if(readXMLString(p, "manaShield", strValue))
		mount->manaShield = booleanString(strValue);

	if(readXMLString(p, "invisible", strValue))
		mount->invisible = booleanString(strValue);

	if(readXMLInteger(p, "healthGain", intValue))
	{
		mount->healthGain = intValue;
		mount->regeneration = true;
	}

	if(readXMLInteger(p, "healthTicks", intValue))
	{
		mount->healthTicks = intValue;
		mount->regeneration = true;
	}

	if(readXMLInteger(p, "manaGain", intValue))
	{
		mount->manaGain = intValue;
		mount->regeneration = true;
	}

	if(readXMLInteger(p, "manaTicks", intValue))
	{
		mount->manaTicks = intValue;
		mount->regeneration = true;
	}

	for(xmlNodePtr configNode = p->children; configNode != NULL; configNode = configNode->next)
	{
		if(!xmlStrcmp(configNode->name, (const xmlChar*)"reflect"))
		{
			if(readXMLInteger(configNode, "percentAll", intValue))
			{
				for(uint32_t i = (COMBAT_FIRST + 1); i <= COMBAT_LAST; i <<= 1)
					mount->reflect[REFLECT_PERCENT][i] += intValue;
			}

			if(readXMLInteger(configNode, "percentElements", intValue))
			{
				mount->reflect[REFLECT_PERCENT][COMBAT_ENERGYDAMAGE] += intValue;
				mount->reflect[REFLECT_PERCENT][COMBAT_FIREDAMAGE] += intValue;
				mount->reflect[REFLECT_PERCENT][COMBAT_EARTHDAMAGE] += intValue;
				mount->reflect[REFLECT_PERCENT][COMBAT_ICEDAMAGE] += intValue;
			}

			if(readXMLInteger(configNode, "percentMagic", intValue))
			{
				mount->reflect[REFLECT_PERCENT][COMBAT_ENERGYDAMAGE] += intValue;
				mount->reflect[REFLECT_PERCENT][COMBAT_FIREDAMAGE] += intValue;
				mount->reflect[REFLECT_PERCENT][COMBAT_EARTHDAMAGE] += intValue;
				mount->reflect[REFLECT_PERCENT][COMBAT_ICEDAMAGE] += intValue;
				mount->reflect[REFLECT_PERCENT][COMBAT_HOLYDAMAGE] += intValue;
				mount->reflect[REFLECT_PERCENT][COMBAT_DEATHDAMAGE] += intValue;
			}

			if(readXMLInteger(configNode, "percentEnergy", intValue))
				mount->reflect[REFLECT_PERCENT][COMBAT_ENERGYDAMAGE] += intValue;

			if(readXMLInteger(configNode, "percentFire", intValue))
				mount->reflect[REFLECT_PERCENT][COMBAT_FIREDAMAGE] += intValue;

			if(readXMLInteger(configNode, "percentPoison", intValue) || readXMLInteger(configNode, "percentEarth", intValue))
				mount->reflect[REFLECT_PERCENT][COMBAT_EARTHDAMAGE] += intValue;

			if(readXMLInteger(configNode, "percentIce", intValue))
				mount->reflect[REFLECT_PERCENT][COMBAT_ICEDAMAGE] += intValue;

			if(readXMLInteger(configNode, "percentHoly", intValue))
				mount->reflect[REFLECT_PERCENT][COMBAT_HOLYDAMAGE] += intValue;

			if(readXMLInteger(configNode, "percentDeath", intValue))
				mount->reflect[REFLECT_PERCENT][COMBAT_DEATHDAMAGE] += intValue;

			if(readXMLInteger(configNode, "percentLifeDrain", intValue))
				mount->reflect[REFLECT_PERCENT][COMBAT_LIFEDRAIN] += intValue;

			if(readXMLInteger(configNode, "percentManaDrain", intValue))
				mount->reflect[REFLECT_PERCENT][COMBAT_MANADRAIN] += intValue;

			if(readXMLInteger(configNode, "percentDrown", intValue))
				mount->reflect[REFLECT_PERCENT][COMBAT_DROWNDAMAGE] += intValue;

			if(readXMLInteger(configNode, "percentPhysical", intValue))
				mount->reflect[REFLECT_PERCENT][COMBAT_PHYSICALDAMAGE] += intValue;

			if(readXMLInteger(configNode, "percentHealing", intValue))
				mount->reflect[REFLECT_PERCENT][COMBAT_HEALING] += intValue;

			if(readXMLInteger(configNode, "percentUndefined", intValue))
				mount->reflect[REFLECT_PERCENT][COMBAT_UNDEFINEDDAMAGE] += intValue;

			if(readXMLInteger(configNode, "chanceAll", intValue))
			{
				for(uint32_t i = (COMBAT_FIRST + 1); i <= COMBAT_LAST; i <<= 1)
					mount->reflect[REFLECT_CHANCE][i] += intValue;
			}

			if(readXMLInteger(configNode, "chanceElements", intValue))
			{
				mount->reflect[REFLECT_CHANCE][COMBAT_ENERGYDAMAGE] += intValue;
				mount->reflect[REFLECT_CHANCE][COMBAT_FIREDAMAGE] += intValue;
				mount->reflect[REFLECT_CHANCE][COMBAT_EARTHDAMAGE] += intValue;
				mount->reflect[REFLECT_CHANCE][COMBAT_ICEDAMAGE] += intValue;
			}

			if(readXMLInteger(configNode, "chanceMagic", intValue))
			{
				mount->reflect[REFLECT_CHANCE][COMBAT_ENERGYDAMAGE] += intValue;
				mount->reflect[REFLECT_CHANCE][COMBAT_FIREDAMAGE] += intValue;
				mount->reflect[REFLECT_CHANCE][COMBAT_EARTHDAMAGE] += intValue;
				mount->reflect[REFLECT_CHANCE][COMBAT_ICEDAMAGE] += intValue;
				mount->reflect[REFLECT_CHANCE][COMBAT_HOLYDAMAGE] += intValue;
				mount->reflect[REFLECT_CHANCE][COMBAT_DEATHDAMAGE] += intValue;
			}

			if(readXMLInteger(configNode, "chanceEnergy", intValue))
				mount->reflect[REFLECT_CHANCE][COMBAT_ENERGYDAMAGE] += intValue;

			if(readXMLInteger(configNode, "chanceFire", intValue))
				mount->reflect[REFLECT_CHANCE][COMBAT_FIREDAMAGE] += intValue;

			if(readXMLInteger(configNode, "chancePoison", intValue) || readXMLInteger(configNode, "chanceEarth", intValue))
				mount->reflect[REFLECT_CHANCE][COMBAT_EARTHDAMAGE] += intValue;

			if(readXMLInteger(configNode, "chanceIce", intValue))
				mount->reflect[REFLECT_CHANCE][COMBAT_ICEDAMAGE] += intValue;

			if(readXMLInteger(configNode, "chanceHoly", intValue))
				mount->reflect[REFLECT_CHANCE][COMBAT_HOLYDAMAGE] += intValue;

			if(readXMLInteger(configNode, "chanceDeath", intValue))
				mount->reflect[REFLECT_CHANCE][COMBAT_DEATHDAMAGE] += intValue;

			if(readXMLInteger(configNode, "chanceLifeDrain", intValue))
				mount->reflect[REFLECT_CHANCE][COMBAT_LIFEDRAIN] += intValue;

			if(readXMLInteger(configNode, "chanceManaDrain", intValue))
				mount->reflect[REFLECT_CHANCE][COMBAT_MANADRAIN] += intValue;

			if(readXMLInteger(configNode, "chanceDrown", intValue))
				mount->reflect[REFLECT_CHANCE][COMBAT_DROWNDAMAGE] += intValue;

			if(readXMLInteger(configNode, "chancePhysical", intValue))
				mount->reflect[REFLECT_CHANCE][COMBAT_PHYSICALDAMAGE] += intValue;

			if(readXMLInteger(configNode, "chanceHealing", intValue))
				mount->reflect[REFLECT_CHANCE][COMBAT_HEALING] += intValue;

			if(readXMLInteger(configNode, "chanceUndefined", intValue))
				mount->reflect[REFLECT_CHANCE][COMBAT_UNDEFINEDDAMAGE] += intValue;
		}
		else if(!xmlStrcmp(configNode->name, (const xmlChar*)"absorb"))
		{
			if(readXMLInteger(configNode, "percentAll", intValue))
			{
				for(uint32_t i = (COMBAT_FIRST + 1); i <= COMBAT_LAST; i <<= 1)
					mount->absorb[i] += intValue;
			}

			if(readXMLInteger(configNode, "percentElements", intValue))
			{
				mount->absorb[COMBAT_ENERGYDAMAGE] += intValue;
				mount->absorb[COMBAT_FIREDAMAGE] += intValue;
				mount->absorb[COMBAT_EARTHDAMAGE] += intValue;
				mount->absorb[COMBAT_ICEDAMAGE] += intValue;
			}

			if(readXMLInteger(configNode, "percentMagic", intValue))
			{
				mount->absorb[COMBAT_ENERGYDAMAGE] += intValue;
				mount->absorb[COMBAT_FIREDAMAGE] += intValue;
				mount->absorb[COMBAT_EARTHDAMAGE] += intValue;
				mount->absorb[COMBAT_ICEDAMAGE] += intValue;
				mount->absorb[COMBAT_HOLYDAMAGE] += intValue;
				mount->absorb[COMBAT_DEATHDAMAGE] += intValue;
			}

			if(readXMLInteger(configNode, "percentEnergy", intValue))
				mount->absorb[COMBAT_ENERGYDAMAGE] += intValue;

			if(readXMLInteger(configNode, "percentFire", intValue))
				mount->absorb[COMBAT_FIREDAMAGE] += intValue;

			if(readXMLInteger(configNode, "percentPoison", intValue) || readXMLInteger(configNode, "percentEarth", intValue))
				mount->absorb[COMBAT_EARTHDAMAGE] += intValue;

			if(readXMLInteger(configNode, "percentIce", intValue))
				mount->absorb[COMBAT_ICEDAMAGE] += intValue;

			if(readXMLInteger(configNode, "percentHoly", intValue))
				mount->absorb[COMBAT_HOLYDAMAGE] += intValue;

			if(readXMLInteger(configNode, "percentDeath", intValue))
				mount->absorb[COMBAT_DEATHDAMAGE] += intValue;

			if(readXMLInteger(configNode, "percentLifeDrain", intValue))
				mount->absorb[COMBAT_LIFEDRAIN] += intValue;

			if(readXMLInteger(configNode, "percentManaDrain", intValue))
				mount->absorb[COMBAT_MANADRAIN] += intValue;

			if(readXMLInteger(configNode, "percentDrown", intValue))
				mount->absorb[COMBAT_DROWNDAMAGE] += intValue;

			if(readXMLInteger(configNode, "percentPhysical", intValue))
				mount->absorb[COMBAT_PHYSICALDAMAGE] += intValue;

			if(readXMLInteger(configNode, "percentHealing", intValue))
				mount->absorb[COMBAT_HEALING] += intValue;

			if(readXMLInteger(configNode, "percentUndefined", intValue))
				mount->absorb[COMBAT_UNDEFINEDDAMAGE] += intValue;
		}
		else if(!xmlStrcmp(configNode->name, (const xmlChar*)"skills"))
		{
			if(readXMLInteger(configNode, "fist", intValue))
				mount->skills[SKILL_FIST] += intValue;

			if(readXMLInteger(configNode, "club", intValue))
				mount->skills[SKILL_CLUB] += intValue;

			if(readXMLInteger(configNode, "axe", intValue))
				mount->skills[SKILL_AXE] += intValue;

			if(readXMLInteger(configNode, "sword", intValue))
				mount->skills[SKILL_SWORD] += intValue;

			if(readXMLInteger(configNode, "distance", intValue) || readXMLInteger(configNode, "dist", intValue))
				mount->skills[SKILL_DIST] += intValue;

			if(readXMLInteger(configNode, "shielding", intValue) || readXMLInteger(configNode, "shield", intValue))
				mount->skills[SKILL_SHIELD] = intValue;

			if(readXMLInteger(configNode, "fishing", intValue) || readXMLInteger(configNode, "fish", intValue))
				mount->skills[SKILL_FISH] = intValue;

			if(readXMLInteger(configNode, "melee", intValue))
			{
				mount->skills[SKILL_FIST] += intValue;
				mount->skills[SKILL_CLUB] += intValue;
				mount->skills[SKILL_SWORD] += intValue;
				mount->skills[SKILL_AXE] += intValue;
			}

			if(readXMLInteger(configNode, "weapon", intValue) || readXMLInteger(configNode, "weapons", intValue))
			{
				mount->skills[SKILL_CLUB] += intValue;
				mount->skills[SKILL_SWORD] += intValue;
				mount->skills[SKILL_AXE] += intValue;
				mount->skills[SKILL_DIST] += intValue;
			}

			if(readXMLInteger(configNode, "fistPercent", intValue))
				mount->skillsPercent[SKILL_FIST] += intValue;

			if(readXMLInteger(configNode, "clubPercent", intValue))
				mount->skillsPercent[SKILL_CLUB] += intValue;

			if(readXMLInteger(configNode, "swordPercent", intValue))
				mount->skillsPercent[SKILL_SWORD] += intValue;

			if(readXMLInteger(configNode, "axePercent", intValue))
				mount->skillsPercent[SKILL_AXE] += intValue;

			if(readXMLInteger(configNode, "distancePercent", intValue) || readXMLInteger(configNode, "distPercent", intValue))
				mount->skillsPercent[SKILL_DIST] += intValue;

			if(readXMLInteger(configNode, "shieldingPercent", intValue) || readXMLInteger(configNode, "shieldPercent", intValue))
				mount->skillsPercent[SKILL_SHIELD] = intValue;

			if(readXMLInteger(configNode, "fishingPercent", intValue) || readXMLInteger(configNode, "fishPercent", intValue))
				mount->skillsPercent[SKILL_FISH] = intValue;

			if(readXMLInteger(configNode, "meleePercent", intValue))
			{
				mount->skillsPercent[SKILL_FIST] += intValue;
				mount->skillsPercent[SKILL_CLUB] += intValue;
				mount->skillsPercent[SKILL_SWORD] += intValue;
				mount->skillsPercent[SKILL_AXE] += intValue;
			}

			if(readXMLInteger(configNode, "weaponPercent", intValue) || readXMLInteger(configNode, "weaponsPercent", intValue))
			{
				mount->skillsPercent[SKILL_CLUB] += intValue;
				mount->skillsPercent[SKILL_SWORD] += intValue;
				mount->skillsPercent[SKILL_AXE] += intValue;
				mount->skillsPercent[SKILL_DIST] += intValue;
			}
		}
		else if(!xmlStrcmp(configNode->name, (const xmlChar*)"stats"))
		{
			if(readXMLInteger(configNode, "maxHealth", intValue))
				mount->stats[STAT_MAXHEALTH] = intValue;

			if(readXMLInteger(configNode, "maxMana", intValue))
				mount->stats[STAT_MAXMANA] = intValue;

			if(readXMLInteger(configNode, "soul", intValue))
				mount->stats[STAT_SOUL] = intValue;

			if(readXMLInteger(configNode, "level", intValue))
				mount->stats[STAT_LEVEL] = intValue;

			if(readXMLInteger(configNode, "magLevel", intValue) ||
				readXMLInteger(configNode, "magicLevel", intValue))
				mount->stats[STAT_MAGICLEVEL] = intValue;

			if(readXMLInteger(configNode, "maxHealthPercent", intValue))
				mount->statsPercent[STAT_MAXHEALTH] = intValue;

			if(readXMLInteger(configNode, "maxManaPercent", intValue))
				mount->statsPercent[STAT_MAXMANA] = intValue;

			if(readXMLInteger(configNode, "soulPercent", intValue))
				mount->statsPercent[STAT_SOUL] = intValue;

			if(readXMLInteger(configNode, "levelPercent", intValue))
				mount->statsPercent[STAT_LEVEL] = intValue;

			if(readXMLInteger(configNode, "magLevelPercent", intValue) ||
				readXMLInteger(configNode, "magicLevelPercent", intValue))
				mount->statsPercent[STAT_MAGICLEVEL] = intValue;
		}
		else if(!xmlStrcmp(configNode->name, (const xmlChar*)"suppress"))
		{
			if(readXMLString(configNode, "poison", strValue) && booleanString(strValue))
				mount->conditionSuppressions |= CONDITION_POISON;

			if(readXMLString(configNode, "fire", strValue) && booleanString(strValue))
				mount->conditionSuppressions |= CONDITION_FIRE;

			if(readXMLString(configNode, "energy", strValue) && booleanString(strValue))
				mount->conditionSuppressions |= CONDITION_ENERGY;

			if(readXMLString(configNode, "physical", strValue) && booleanString(strValue))
				mount->conditionSuppressions |= CONDITION_BLEEDING;

			if(readXMLString(configNode, "haste", strValue) && booleanString(strValue))
				mount->conditionSuppressions |= CONDITION_HASTE;

			if(readXMLString(configNode, "paralyze", strValue) && booleanString(strValue))
				mount->conditionSuppressions |= CONDITION_PARALYZE;

			if(readXMLString(configNode, "outfit", strValue) && booleanString(strValue))
				mount->conditionSuppressions |= CONDITION_OUTFIT;

			if(readXMLString(configNode, "invisible", strValue) && booleanString(strValue))
				mount->conditionSuppressions |= CONDITION_INVISIBLE;

			if(readXMLString(configNode, "light", strValue) && booleanString(strValue))
				mount->conditionSuppressions |= CONDITION_LIGHT;

			if(readXMLString(configNode, "manaShield", strValue) && booleanString(strValue))
				mount->conditionSuppressions |= CONDITION_MANASHIELD;

			if(readXMLString(configNode, "infight", strValue) && booleanString(strValue))
				mount->conditionSuppressions |= CONDITION_INFIGHT;

			if(readXMLString(configNode, "drunk", strValue) && booleanString(strValue))
				mount->conditionSuppressions |= CONDITION_DRUNK;

			if(readXMLString(configNode, "exhaust", strValue) && booleanString(strValue))
				mount->conditionSuppressions |= CONDITION_EXHAUST;

			if(readXMLString(configNode, "regeneration", strValue) && booleanString(strValue))
				mount->conditionSuppressions |= CONDITION_REGENERATION;

			if(readXMLString(configNode, "soul", strValue) && booleanString(strValue))
				mount->conditionSuppressions |= CONDITION_SOUL;

			if(readXMLString(configNode, "drown", strValue) && booleanString(strValue))
				mount->conditionSuppressions |= CONDITION_DROWN;

			if(readXMLString(configNode, "muted", strValue) && booleanString(strValue))
				mount->conditionSuppressions |= CONDITION_MUTED;

			if(readXMLString(configNode, "attributes", strValue) && booleanString(strValue))
				mount->conditionSuppressions |= CONDITION_ATTRIBUTES;

			if(readXMLString(configNode, "freezing", strValue) && booleanString(strValue))
				mount->conditionSuppressions |= CONDITION_FREEZING;

			if(readXMLString(configNode, "dazzled", strValue) && booleanString(strValue))
				mount->conditionSuppressions |= CONDITION_DAZZLED;

			if(readXMLString(configNode, "cursed", strValue) && booleanString(strValue))
				mount->conditionSuppressions |= CONDITION_CURSED;

			if(readXMLString(configNode, "pacified", strValue) && booleanString(strValue))
				mount->conditionSuppressions |= CONDITION_PACIFIED;

			if(readXMLString(configNode, "gamemaster", strValue) && booleanString(strValue))
				mount->conditionSuppressions |= CONDITION_GAMEMASTER;
		}
	}

	mounts.push_back(mount);
	return true;
}

Mount* Mounts::getMountById(uint16_t id) const
{
	if(!id)
		return NULL;

	for(MountList::const_iterator it = mounts.begin(); it != mounts.end(); ++it)
	{
		if((*it)->getId() == id)
			return (*it);
	}

	return NULL;
}

Mount* Mounts::getMountByCid(uint16_t id) const
{
	if(!id)
		return NULL;

	for(MountList::const_iterator it = mounts.begin(); it != mounts.end(); ++it)
	{
		if((*it)->getClientId() == id)
			return (*it);
	}

	return NULL;
}

bool Mounts::isPremium() const
{
	for(MountList::const_iterator it = mounts.begin(); it != mounts.end(); ++it)
	{
		if(!(*it)->isPremium())
			return false;
	}

	return true;
}
