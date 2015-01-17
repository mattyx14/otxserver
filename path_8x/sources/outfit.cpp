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
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "outfit.h"
#include "tools.h"

#include "player.h"
#include "condition.h"

#include "game.h"
extern Game g_game;

bool Outfits::parseOutfitNode(xmlNodePtr p)
{
	if(xmlStrcmp(p->name, (const xmlChar*)"outfit"))
		return false;

	int32_t intValue;
	if(!readXMLInteger(p, "id", intValue))
	{
		std::clog << "[Error - Outfits::parseOutfitNode] Missing outfit id, skipping" << std::endl;
		return false;
	}

	Outfit newOutfit;
	newOutfit.outfitId = intValue;

	std::string strValue;
	if(readXMLString(p, "default", strValue))
		newOutfit.isDefault = booleanString(strValue);

	if(!readXMLString(p, "name", strValue))
	{
		std::stringstream ss;
		ss << "Outfit #" << newOutfit.outfitId;
		ss >> newOutfit.name;
	}
	else
		newOutfit.name = strValue;

	bool override = false;
	if(readXMLString(p, "override", strValue) && booleanString(strValue))
		override = true;

	if(readXMLInteger(p, "access", intValue))
		newOutfit.accessLevel = intValue;

	if(readXMLString(p, "group", strValue) || readXMLString(p, "groups", strValue))
	{
		newOutfit.groups.clear();
		if(!parseIntegerVec(strValue, newOutfit.groups))
			std::clog << "[Warning - Outfits::parseOutfitNode] Invalid group(s) for an outfit with id " << newOutfit.outfitId << std::endl;
	}

	if(readXMLString(p, "quest", strValue))
	{
		newOutfit.storageId = strValue;
		newOutfit.storageValue = "1";
	}
	else
	{
		if(readXMLString(p, "storageId", strValue))
			newOutfit.storageId = strValue;

		if(readXMLString(p, "storageValue", strValue))
			newOutfit.storageValue = strValue;
	}

	if(readXMLString(p, "premium", strValue))
		newOutfit.isPremium = booleanString(strValue);

	for(xmlNodePtr listNode = p->children; listNode != NULL; listNode = listNode->next)
	{
		if(xmlStrcmp(listNode->name, (const xmlChar*)"list"))
			continue;

		Outfit outfit = newOutfit;
		if(!readXMLInteger(listNode, "looktype", intValue) && !readXMLInteger(listNode, "lookType", intValue))
		{
			std::clog << "[Error - Outfits::parseOutfitNode] Missing looktype for an outfit with id " << outfit.outfitId << std::endl;
			continue;
		}

		outfit.lookType = intValue;
		if(!readXMLString(listNode, "gender", strValue) && !readXMLString(listNode, "type", strValue) && !readXMLString(listNode, "sex", strValue))
		{
			std::clog << "[Error - Outfits::parseOutfitNode] Missing gender(s) for an outfit with id " << outfit.outfitId
				<< " and looktype " << outfit.lookType << std::endl;
			continue;
		}

		IntegerVec intVector;
		if(!parseIntegerVec(strValue, intVector))
		{
			std::clog << "[Error - Outfits::parseOutfitNode] Invalid gender(s) for an outfit with id " << outfit.outfitId
				<< " and looktype " << outfit.lookType << std::endl;
			continue;
		}

		if(readXMLInteger(listNode, "addons", intValue))
			outfit.addons = intValue;

		if(readXMLString(listNode, "name", strValue))
			outfit.name = strValue;

		if(readXMLString(listNode, "premium", strValue))
			outfit.isPremium = booleanString(strValue);

		if(readXMLString(listNode, "requirement", strValue))
		{
			std::string tmpStrValue = asLowerCaseString(strValue);
			if(tmpStrValue == "none")
				outfit.requirement = REQUIREMENT_NONE;
			else if(tmpStrValue == "first")
				outfit.requirement = REQUIREMENT_FIRST;
			else if(tmpStrValue == "second")
				outfit.requirement = REQUIREMENT_SECOND;
			else if(tmpStrValue == "any")
				outfit.requirement = REQUIREMENT_ANY;
			else if(tmpStrValue != "both")
				std::clog << "[Warning - Outfits::loadFromXml] Unknown requirement tag value, using default (both)" << std::endl;
		}

		if(readXMLString(listNode, "manaShield", strValue))
			outfit.manaShield = booleanString(strValue);

		if(readXMLString(listNode, "invisible", strValue))
			outfit.invisible = booleanString(strValue);

		if(readXMLInteger(listNode, "healthGain", intValue))
		{
			outfit.healthGain = intValue;
			outfit.regeneration = true;
		}

		if(readXMLInteger(listNode, "healthTicks", intValue))
		{
			outfit.healthTicks = intValue;
			outfit.regeneration = true;
		}

		if(readXMLInteger(listNode, "manaGain", intValue))
		{
			outfit.manaGain = intValue;
			outfit.regeneration = true;
		}

		if(readXMLInteger(listNode, "manaTicks", intValue))
		{
			outfit.manaTicks = intValue;
			outfit.regeneration = true;
		}

		if(readXMLInteger(listNode, "speed", intValue))
			outfit.speed = intValue;

		if(readXMLInteger(listNode, "attackspeed", intValue) || readXMLInteger(listNode, "attackSpeed", intValue))
			outfit.attackSpeed = intValue;

		for(xmlNodePtr configNode = listNode->children; configNode != NULL; configNode = configNode->next)
		{
			if(!xmlStrcmp(configNode->name, (const xmlChar*)"reflect"))
			{
				if(readXMLInteger(configNode, "percentAll", intValue))
				{
					for(uint32_t i = (COMBAT_FIRST + 1); i <= COMBAT_LAST; i <<= 1)
						outfit.reflect[REFLECT_PERCENT][i] += intValue;
				}

				if(readXMLInteger(configNode, "percentElements", intValue))
				{
					outfit.reflect[REFLECT_PERCENT][COMBAT_ENERGYDAMAGE] += intValue;
					outfit.reflect[REFLECT_PERCENT][COMBAT_FIREDAMAGE] += intValue;
					outfit.reflect[REFLECT_PERCENT][COMBAT_EARTHDAMAGE] += intValue;
					outfit.reflect[REFLECT_PERCENT][COMBAT_ICEDAMAGE] += intValue;
				}

				if(readXMLInteger(configNode, "percentMagic", intValue))
				{
					outfit.reflect[REFLECT_PERCENT][COMBAT_ENERGYDAMAGE] += intValue;
					outfit.reflect[REFLECT_PERCENT][COMBAT_FIREDAMAGE] += intValue;
					outfit.reflect[REFLECT_PERCENT][COMBAT_EARTHDAMAGE] += intValue;
					outfit.reflect[REFLECT_PERCENT][COMBAT_ICEDAMAGE] += intValue;
					outfit.reflect[REFLECT_PERCENT][COMBAT_HOLYDAMAGE] += intValue;
					outfit.reflect[REFLECT_PERCENT][COMBAT_DEATHDAMAGE] += intValue;
				}

				if(readXMLInteger(configNode, "percentEnergy", intValue))
					outfit.reflect[REFLECT_PERCENT][COMBAT_ENERGYDAMAGE] += intValue;

				if(readXMLInteger(configNode, "percentFire", intValue))
					outfit.reflect[REFLECT_PERCENT][COMBAT_FIREDAMAGE] += intValue;

				if(readXMLInteger(configNode, "percentPoison", intValue) || readXMLInteger(configNode, "percentEarth", intValue))
					outfit.reflect[REFLECT_PERCENT][COMBAT_EARTHDAMAGE] += intValue;

				if(readXMLInteger(configNode, "percentIce", intValue))
					outfit.reflect[REFLECT_PERCENT][COMBAT_ICEDAMAGE] += intValue;

				if(readXMLInteger(configNode, "percentHoly", intValue))
					outfit.reflect[REFLECT_PERCENT][COMBAT_HOLYDAMAGE] += intValue;

				if(readXMLInteger(configNode, "percentDeath", intValue))
					outfit.reflect[REFLECT_PERCENT][COMBAT_DEATHDAMAGE] += intValue;

				if(readXMLInteger(configNode, "percentLifeDrain", intValue))
					outfit.reflect[REFLECT_PERCENT][COMBAT_LIFEDRAIN] += intValue;

				if(readXMLInteger(configNode, "percentManaDrain", intValue))
					outfit.reflect[REFLECT_PERCENT][COMBAT_MANADRAIN] += intValue;

				if(readXMLInteger(configNode, "percentDrown", intValue))
					outfit.reflect[REFLECT_PERCENT][COMBAT_DROWNDAMAGE] += intValue;

				if(readXMLInteger(configNode, "percentPhysical", intValue))
					outfit.reflect[REFLECT_PERCENT][COMBAT_PHYSICALDAMAGE] += intValue;

				if(readXMLInteger(configNode, "percentHealing", intValue))
					outfit.reflect[REFLECT_PERCENT][COMBAT_HEALING] += intValue;

				if(readXMLInteger(configNode, "percentUndefined", intValue))
					outfit.reflect[REFLECT_PERCENT][COMBAT_UNDEFINEDDAMAGE] += intValue;

				if(readXMLInteger(configNode, "chanceAll", intValue))
				{
					for(uint32_t i = (COMBAT_FIRST + 1); i <= COMBAT_LAST; i <<= 1)
						outfit.reflect[REFLECT_CHANCE][i] += intValue;
				}

				if(readXMLInteger(configNode, "chanceElements", intValue))
				{
					outfit.reflect[REFLECT_CHANCE][COMBAT_ENERGYDAMAGE] += intValue;
					outfit.reflect[REFLECT_CHANCE][COMBAT_FIREDAMAGE] += intValue;
					outfit.reflect[REFLECT_CHANCE][COMBAT_EARTHDAMAGE] += intValue;
					outfit.reflect[REFLECT_CHANCE][COMBAT_ICEDAMAGE] += intValue;
				}

				if(readXMLInteger(configNode, "chanceMagic", intValue))
				{
					outfit.reflect[REFLECT_CHANCE][COMBAT_ENERGYDAMAGE] += intValue;
					outfit.reflect[REFLECT_CHANCE][COMBAT_FIREDAMAGE] += intValue;
					outfit.reflect[REFLECT_CHANCE][COMBAT_EARTHDAMAGE] += intValue;
					outfit.reflect[REFLECT_CHANCE][COMBAT_ICEDAMAGE] += intValue;
					outfit.reflect[REFLECT_CHANCE][COMBAT_HOLYDAMAGE] += intValue;
					outfit.reflect[REFLECT_CHANCE][COMBAT_DEATHDAMAGE] += intValue;
				}

				if(readXMLInteger(configNode, "chanceEnergy", intValue))
					outfit.reflect[REFLECT_CHANCE][COMBAT_ENERGYDAMAGE] += intValue;

				if(readXMLInteger(configNode, "chanceFire", intValue))
					outfit.reflect[REFLECT_CHANCE][COMBAT_FIREDAMAGE] += intValue;

				if(readXMLInteger(configNode, "chancePoison", intValue) || readXMLInteger(configNode, "chanceEarth", intValue))
					outfit.reflect[REFLECT_CHANCE][COMBAT_EARTHDAMAGE] += intValue;

				if(readXMLInteger(configNode, "chanceIce", intValue))
					outfit.reflect[REFLECT_CHANCE][COMBAT_ICEDAMAGE] += intValue;

				if(readXMLInteger(configNode, "chanceHoly", intValue))
					outfit.reflect[REFLECT_CHANCE][COMBAT_HOLYDAMAGE] += intValue;

				if(readXMLInteger(configNode, "chanceDeath", intValue))
					outfit.reflect[REFLECT_CHANCE][COMBAT_DEATHDAMAGE] += intValue;

				if(readXMLInteger(configNode, "chanceLifeDrain", intValue))
					outfit.reflect[REFLECT_CHANCE][COMBAT_LIFEDRAIN] += intValue;

				if(readXMLInteger(configNode, "chanceManaDrain", intValue))
					outfit.reflect[REFLECT_CHANCE][COMBAT_MANADRAIN] += intValue;

				if(readXMLInteger(configNode, "chanceDrown", intValue))
					outfit.reflect[REFLECT_CHANCE][COMBAT_DROWNDAMAGE] += intValue;

				if(readXMLInteger(configNode, "chancePhysical", intValue))
					outfit.reflect[REFLECT_CHANCE][COMBAT_PHYSICALDAMAGE] += intValue;

				if(readXMLInteger(configNode, "chanceHealing", intValue))
					outfit.reflect[REFLECT_CHANCE][COMBAT_HEALING] += intValue;

				if(readXMLInteger(configNode, "chanceUndefined", intValue))
					outfit.reflect[REFLECT_CHANCE][COMBAT_UNDEFINEDDAMAGE] += intValue;
			}
			else if(!xmlStrcmp(configNode->name, (const xmlChar*)"absorb"))
			{
				if(readXMLInteger(configNode, "percentAll", intValue))
				{
					for(uint32_t i = (COMBAT_FIRST + 1); i <= COMBAT_LAST; i <<= 1)
						outfit.absorb[i] += intValue;
				}

				if(readXMLInteger(configNode, "percentElements", intValue))
				{
					outfit.absorb[COMBAT_ENERGYDAMAGE] += intValue;
					outfit.absorb[COMBAT_FIREDAMAGE] += intValue;
					outfit.absorb[COMBAT_EARTHDAMAGE] += intValue;
					outfit.absorb[COMBAT_ICEDAMAGE] += intValue;
				}

				if(readXMLInteger(configNode, "percentMagic", intValue))
				{
					outfit.absorb[COMBAT_ENERGYDAMAGE] += intValue;
					outfit.absorb[COMBAT_FIREDAMAGE] += intValue;
					outfit.absorb[COMBAT_EARTHDAMAGE] += intValue;
					outfit.absorb[COMBAT_ICEDAMAGE] += intValue;
					outfit.absorb[COMBAT_HOLYDAMAGE] += intValue;
					outfit.absorb[COMBAT_DEATHDAMAGE] += intValue;
				}

				if(readXMLInteger(configNode, "percentEnergy", intValue))
					outfit.absorb[COMBAT_ENERGYDAMAGE] += intValue;

				if(readXMLInteger(configNode, "percentFire", intValue))
					outfit.absorb[COMBAT_FIREDAMAGE] += intValue;

				if(readXMLInteger(configNode, "percentPoison", intValue) || readXMLInteger(configNode, "percentEarth", intValue))
					outfit.absorb[COMBAT_EARTHDAMAGE] += intValue;

				if(readXMLInteger(configNode, "percentIce", intValue))
					outfit.absorb[COMBAT_ICEDAMAGE] += intValue;

				if(readXMLInteger(configNode, "percentHoly", intValue))
					outfit.absorb[COMBAT_HOLYDAMAGE] += intValue;

				if(readXMLInteger(configNode, "percentDeath", intValue))
					outfit.absorb[COMBAT_DEATHDAMAGE] += intValue;

				if(readXMLInteger(configNode, "percentLifeDrain", intValue))
					outfit.absorb[COMBAT_LIFEDRAIN] += intValue;

				if(readXMLInteger(configNode, "percentManaDrain", intValue))
					outfit.absorb[COMBAT_MANADRAIN] += intValue;

				if(readXMLInteger(configNode, "percentDrown", intValue))
					outfit.absorb[COMBAT_DROWNDAMAGE] += intValue;

				if(readXMLInteger(configNode, "percentPhysical", intValue))
					outfit.absorb[COMBAT_PHYSICALDAMAGE] += intValue;

				if(readXMLInteger(configNode, "percentHealing", intValue))
					outfit.absorb[COMBAT_HEALING] += intValue;

				if(readXMLInteger(configNode, "percentUndefined", intValue))
					outfit.absorb[COMBAT_UNDEFINEDDAMAGE] += intValue;
			}
			else if(!xmlStrcmp(configNode->name, (const xmlChar*)"skills"))
			{
				if(readXMLInteger(configNode, "fist", intValue))
					outfit.skills[SKILL_FIST] += intValue;

				if(readXMLInteger(configNode, "club", intValue))
					outfit.skills[SKILL_CLUB] += intValue;

				if(readXMLInteger(configNode, "axe", intValue))
					outfit.skills[SKILL_AXE] += intValue;

				if(readXMLInteger(configNode, "sword", intValue))
					outfit.skills[SKILL_SWORD] += intValue;

				if(readXMLInteger(configNode, "distance", intValue) || readXMLInteger(configNode, "dist", intValue))
					outfit.skills[SKILL_DIST] += intValue;

				if(readXMLInteger(configNode, "shielding", intValue) || readXMLInteger(configNode, "shield", intValue))
					outfit.skills[SKILL_SHIELD] = intValue;

				if(readXMLInteger(configNode, "fishing", intValue) || readXMLInteger(configNode, "fish", intValue))
					outfit.skills[SKILL_FISH] = intValue;

				if(readXMLInteger(configNode, "melee", intValue))
				{
					outfit.skills[SKILL_FIST] += intValue;
					outfit.skills[SKILL_CLUB] += intValue;
					outfit.skills[SKILL_SWORD] += intValue;
					outfit.skills[SKILL_AXE] += intValue;
				}

				if(readXMLInteger(configNode, "weapon", intValue) || readXMLInteger(configNode, "weapons", intValue))
				{
					outfit.skills[SKILL_CLUB] += intValue;
					outfit.skills[SKILL_SWORD] += intValue;
					outfit.skills[SKILL_AXE] += intValue;
					outfit.skills[SKILL_DIST] += intValue;
				}

				if(readXMLInteger(configNode, "fistPercent", intValue))
					outfit.skillsPercent[SKILL_FIST] += intValue;

				if(readXMLInteger(configNode, "clubPercent", intValue))
					outfit.skillsPercent[SKILL_CLUB] += intValue;

				if(readXMLInteger(configNode, "swordPercent", intValue))
					outfit.skillsPercent[SKILL_SWORD] += intValue;

				if(readXMLInteger(configNode, "axePercent", intValue))
					outfit.skillsPercent[SKILL_AXE] += intValue;

				if(readXMLInteger(configNode, "distancePercent", intValue) || readXMLInteger(configNode, "distPercent", intValue))
					outfit.skillsPercent[SKILL_DIST] += intValue;

				if(readXMLInteger(configNode, "shieldingPercent", intValue) || readXMLInteger(configNode, "shieldPercent", intValue))
					outfit.skillsPercent[SKILL_SHIELD] = intValue;

				if(readXMLInteger(configNode, "fishingPercent", intValue) || readXMLInteger(configNode, "fishPercent", intValue))
					outfit.skillsPercent[SKILL_FISH] = intValue;

				if(readXMLInteger(configNode, "meleePercent", intValue))
				{
					outfit.skillsPercent[SKILL_FIST] += intValue;
					outfit.skillsPercent[SKILL_CLUB] += intValue;
					outfit.skillsPercent[SKILL_SWORD] += intValue;
					outfit.skillsPercent[SKILL_AXE] += intValue;
				}

				if(readXMLInteger(configNode, "weaponPercent", intValue) || readXMLInteger(configNode, "weaponsPercent", intValue))
				{
					outfit.skillsPercent[SKILL_CLUB] += intValue;
					outfit.skillsPercent[SKILL_SWORD] += intValue;
					outfit.skillsPercent[SKILL_AXE] += intValue;
					outfit.skillsPercent[SKILL_DIST] += intValue;
				}
			}
			else if(!xmlStrcmp(configNode->name, (const xmlChar*)"stats"))
			{
				if(readXMLInteger(configNode, "maxHealth", intValue))
					outfit.stats[STAT_MAXHEALTH] = intValue;

				if(readXMLInteger(configNode, "maxMana", intValue))
					outfit.stats[STAT_MAXMANA] = intValue;

				if(readXMLInteger(configNode, "soul", intValue))
					outfit.stats[STAT_SOUL] = intValue;

				if(readXMLInteger(configNode, "level", intValue))
					outfit.stats[STAT_LEVEL] = intValue;

				if(readXMLInteger(configNode, "magLevel", intValue) ||
					readXMLInteger(configNode, "magicLevel", intValue))
					outfit.stats[STAT_MAGICLEVEL] = intValue;

				if(readXMLInteger(configNode, "maxHealthPercent", intValue))
					outfit.statsPercent[STAT_MAXHEALTH] = intValue;

				if(readXMLInteger(configNode, "maxManaPercent", intValue))
					outfit.statsPercent[STAT_MAXMANA] = intValue;

				if(readXMLInteger(configNode, "soulPercent", intValue))
					outfit.statsPercent[STAT_SOUL] = intValue;

				if(readXMLInteger(configNode, "levelPercent", intValue))
					outfit.statsPercent[STAT_LEVEL] = intValue;

				if(readXMLInteger(configNode, "magLevelPercent", intValue) ||
					readXMLInteger(configNode, "magicLevelPercent", intValue))
					outfit.statsPercent[STAT_MAGICLEVEL] = intValue;
			}
			else if(!xmlStrcmp(configNode->name, (const xmlChar*)"suppress"))
			{
				if(readXMLString(configNode, "poison", strValue) && booleanString(strValue))
					outfit.conditionSuppressions |= CONDITION_POISON;

				if(readXMLString(configNode, "fire", strValue) && booleanString(strValue))
					outfit.conditionSuppressions |= CONDITION_FIRE;

				if(readXMLString(configNode, "energy", strValue) && booleanString(strValue))
					outfit.conditionSuppressions |= CONDITION_ENERGY;

				if(readXMLString(configNode, "physical", strValue) && booleanString(strValue))
					outfit.conditionSuppressions |= CONDITION_BLEEDING;

				if(readXMLString(configNode, "haste", strValue) && booleanString(strValue))
					outfit.conditionSuppressions |= CONDITION_HASTE;

				if(readXMLString(configNode, "paralyze", strValue) && booleanString(strValue))
					outfit.conditionSuppressions |= CONDITION_PARALYZE;

				if(readXMLString(configNode, "outfit", strValue) && booleanString(strValue))
					outfit.conditionSuppressions |= CONDITION_OUTFIT;

				if(readXMLString(configNode, "invisible", strValue) && booleanString(strValue))
					outfit.conditionSuppressions |= CONDITION_INVISIBLE;

				if(readXMLString(configNode, "light", strValue) && booleanString(strValue))
					outfit.conditionSuppressions |= CONDITION_LIGHT;

				if(readXMLString(configNode, "manaShield", strValue) && booleanString(strValue))
					outfit.conditionSuppressions |= CONDITION_MANASHIELD;

				if(readXMLString(configNode, "infight", strValue) && booleanString(strValue))
					outfit.conditionSuppressions |= CONDITION_INFIGHT;

				if(readXMLString(configNode, "drunk", strValue) && booleanString(strValue))
					outfit.conditionSuppressions |= CONDITION_DRUNK;

				if(readXMLString(configNode, "exhaust", strValue) && booleanString(strValue))
					outfit.conditionSuppressions |= CONDITION_EXHAUST;

				if(readXMLString(configNode, "regeneration", strValue) && booleanString(strValue))
					outfit.conditionSuppressions |= CONDITION_REGENERATION;

				if(readXMLString(configNode, "soul", strValue) && booleanString(strValue))
					outfit.conditionSuppressions |= CONDITION_SOUL;

				if(readXMLString(configNode, "drown", strValue) && booleanString(strValue))
					outfit.conditionSuppressions |= CONDITION_DROWN;

				if(readXMLString(configNode, "muted", strValue) && booleanString(strValue))
					outfit.conditionSuppressions |= CONDITION_MUTED;

				if(readXMLString(configNode, "attributes", strValue) && booleanString(strValue))
					outfit.conditionSuppressions |= CONDITION_ATTRIBUTES;

				if(readXMLString(configNode, "freezing", strValue) && booleanString(strValue))
					outfit.conditionSuppressions |= CONDITION_FREEZING;

				if(readXMLString(configNode, "dazzled", strValue) && booleanString(strValue))
					outfit.conditionSuppressions |= CONDITION_DAZZLED;

				if(readXMLString(configNode, "cursed", strValue) && booleanString(strValue))
					outfit.conditionSuppressions |= CONDITION_CURSED;

				if(readXMLString(configNode, "pacified", strValue) && booleanString(strValue))
					outfit.conditionSuppressions |= CONDITION_PACIFIED;

				if(readXMLString(configNode, "gamemaster", strValue) && booleanString(strValue))
					outfit.conditionSuppressions |= CONDITION_GAMEMASTER;
			}
		}

		bool add = false;
		OutfitMap::iterator fit;
		for(IntegerVec::iterator it = intVector.begin(); it != intVector.end(); ++it)
		{
			fit = outfitsMap[(*it)].find(outfit.outfitId);
			if(fit != outfitsMap[(*it)].end())
			{
				if(override)
				{
					fit->second = outfit;
					if(!add)
						add = true;
				}
				else
					std::clog << "[Warning - Outfits::parseOutfitNode] Duplicated outfit for gender " << (*it) << " with lookType " << outfit.outfitId << std::endl;
			}
			else
			{
				outfitsMap[(*it)][outfit.outfitId] = outfit;
				if(!add)
					add = true;
			}
		}

		if(add)
			allOutfits.push_back(outfit);
	}

	return true;
}

bool Outfits::loadFromXml()
{
	xmlDocPtr doc = xmlParseFile(getFilePath(FILE_TYPE_XML, "outfits.xml").c_str());
	if(!doc)
	{
		std::clog << "[Warning - Outfits::loadFromXml] Cannot load outfits file, using defaults." << std::endl;
		std::clog << getLastXMLError() << std::endl;
		return false;
	}

	xmlNodePtr root = xmlDocGetRootElement(doc);
	if(xmlStrcmp(root->name,(const xmlChar*)"outfits"))
	{
		std::clog << "[Error - Outfits::loadFromXml] Malformed outfits file." << std::endl;
		xmlFreeDoc(doc);
		return false;
	}

	for(xmlNodePtr p = root->children; p; p = p->next)
		parseOutfitNode(p);

	xmlFreeDoc(doc);
	return true;
}

uint32_t Outfits::getOutfitId(uint32_t lookType)
{
	for(OutfitList::iterator it = allOutfits.begin(); it != allOutfits.end(); ++it)
	{
		if(it->lookType == lookType)
			return it->outfitId;
	}

	return 0;
}

bool Outfits::getOutfit(uint32_t lookType, Outfit& outfit)
{
	for(OutfitList::iterator it = allOutfits.begin(); it != allOutfits.end(); ++it)
	{
		if(it->lookType != lookType)
			continue;

		outfit = *it;
		return true;
	}

	return false;
}

bool Outfits::getOutfit(uint32_t outfitId, uint16_t sex, Outfit& outfit)
{
	OutfitMap map = outfitsMap[sex];
	OutfitMap::iterator it = map.find(outfitId);
	if(it == map.end())
		return false;

	outfit = it->second;
	return true;
}

bool Outfits::addAttributes(uint32_t playerId, uint32_t outfitId, uint16_t sex, uint16_t addons)
{
	Player* player = g_game.getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	OutfitMap map = outfitsMap[sex];
	OutfitMap::iterator it = map.find(outfitId);
	if(it == map.end())
		return false;

	Outfit outfit = it->second;
	if(outfit.requirement != (AddonRequirement_t)addons && (outfit.requirement != REQUIREMENT_ANY || !addons))
		return false;

	if(outfit.invisible)
	{
		Condition* condition = Condition::createCondition(CONDITIONID_OUTFIT, CONDITION_INVISIBLE, -1, 0);
		player->addCondition(condition);
	}

	if(outfit.manaShield)
	{
		Condition* condition = Condition::createCondition(CONDITIONID_OUTFIT, CONDITION_MANASHIELD, -1, 0);
		player->addCondition(condition);
	}

	if(outfit.speed)
		g_game.changeSpeed(player, outfit.speed);

	if(outfit.conditionSuppressions)
	{
		player->setConditionSuppressions(outfit.conditionSuppressions, false);
		player->sendIcons();
	}

	if(outfit.regeneration)
	{
		Condition* condition = Condition::createCondition(CONDITIONID_OUTFIT, CONDITION_REGENERATION, -1, 0);
		if(outfit.healthGain)
			condition->setParam(CONDITIONPARAM_HEALTHGAIN, outfit.healthGain);

		if(outfit.healthTicks)
			condition->setParam(CONDITIONPARAM_HEALTHTICKS, outfit.healthTicks);

		if(outfit.manaGain)
			condition->setParam(CONDITIONPARAM_MANAGAIN, outfit.manaGain);

		if(outfit.manaTicks)
			condition->setParam(CONDITIONPARAM_MANATICKS, outfit.manaTicks);

		player->addCondition(condition);
	}

	bool needUpdateSkills = false;
	for(uint32_t i = SKILL_FIRST; i <= SKILL_LAST; ++i)
	{
		if(outfit.skills[i])
		{
			needUpdateSkills = true;
			player->setVarSkill((skills_t)i, outfit.skills[i]);
		}

		if(outfit.skillsPercent[i])
		{
			needUpdateSkills = true;
			player->setVarSkill((skills_t)i, (int32_t)(player->getSkill((skills_t)i, SKILL_LEVEL) * ((outfit.skillsPercent[i] - 100) / 100.f)));
		}
	}

	if(needUpdateSkills)
		player->sendSkills();

	bool needUpdateStats = false;
	for(uint32_t s = STAT_FIRST; s <= STAT_LAST; ++s)
	{
		if(outfit.stats[s])
		{
			needUpdateStats = true;
			player->setVarStats((stats_t)s, outfit.stats[s]);
		}

		if(outfit.statsPercent[s])
		{
			needUpdateStats = true;
			player->setVarStats((stats_t)s, (int32_t)(player->getDefaultStats((stats_t)s) * ((outfit.statsPercent[s] - 100) / 100.f)));
		}
	}

	if(needUpdateStats)
		player->sendStats();

	return true;
}

bool Outfits::removeAttributes(uint32_t playerId, uint32_t outfitId, uint16_t sex)
{
	Player* player = g_game.getPlayerByID(playerId);
	if(!player || player->isRemoved())
		return false;

	OutfitMap map = outfitsMap[sex];
	OutfitMap::iterator it = map.find(outfitId);
	if(it == map.end())
		return false;

	Outfit outfit = it->second;
	if(outfit.invisible)
		player->removeCondition(CONDITION_INVISIBLE, CONDITIONID_OUTFIT);

	if(outfit.manaShield)
		player->removeCondition(CONDITION_MANASHIELD, CONDITIONID_OUTFIT);

	if(outfit.speed)
		g_game.changeSpeed(player, -outfit.speed);

	if(outfit.conditionSuppressions)
	{
		player->setConditionSuppressions(outfit.conditionSuppressions, true);
		player->sendIcons();
	}

	if(outfit.regeneration)
		player->removeCondition(CONDITION_REGENERATION, CONDITIONID_OUTFIT);

	bool needUpdateSkills = false;
	for(uint32_t i = SKILL_FIRST; i <= SKILL_LAST; ++i)
	{
		if(outfit.skills[i])
		{
			needUpdateSkills = true;
			player->setVarSkill((skills_t)i, -outfit.skills[i]);
		}

		if(outfit.skillsPercent[i])
		{
			needUpdateSkills = true;
			player->setVarSkill((skills_t)i, -(int32_t)(player->getSkill((skills_t)i, SKILL_LEVEL) * ((outfit.skillsPercent[i] - 100) / 100.f)));
		}
	}

	if(needUpdateSkills)
		player->sendSkills();

	bool needUpdateStats = false;
	for(uint32_t s = STAT_FIRST; s <= STAT_LAST; ++s)
	{
		if(outfit.stats[s])
		{
			needUpdateStats = true;
			player->setVarStats((stats_t)s, -outfit.stats[s]);
		}

		if(outfit.statsPercent[s])
		{
			needUpdateStats = true;
			player->setVarStats((stats_t)s, -(int32_t)(player->getDefaultStats((stats_t)s) * ((outfit.statsPercent[s] - 100) / 100.f)));
		}
	}

	if(needUpdateStats)
		player->sendStats();

	return true;
}

int16_t Outfits::getOutfitAbsorb(uint32_t lookType, uint16_t sex, CombatType_t combat)
{
	OutfitMap map = outfitsMap[sex];
	if(!map.size())
		return 0;

	for(OutfitMap::iterator it = map.begin(); it != map.end(); ++it)
	{
		if(it->second.lookType == lookType)
			return it->second.absorb[combat];
	}

	return 0;
}

int16_t Outfits::getOutfitReflect(uint32_t lookType, uint16_t sex, CombatType_t combat)
{
	OutfitMap map = outfitsMap[sex];
	if(!map.size())
		return 0;

	for(OutfitMap::iterator it = map.begin(); it != map.end(); ++it)
	{
		if(it->second.lookType != lookType)
			continue;

		if(it->second.reflect[REFLECT_PERCENT][combat] && it->second.reflect[REFLECT_CHANCE][combat] >= random_range(1, 100))
			return it->second.reflect[REFLECT_PERCENT][combat];
	}

	return 0;
}
