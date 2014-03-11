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

#include "condition.h"
#include "tools.h"

#include "game.h"
#include "creature.h"
#include "combat.h"

extern Game g_game;

Condition::Condition(ConditionId_t _id, ConditionType_t _type, int32_t _ticks, bool _buff, uint32_t _subId):
id(_id), subId(_subId), ticks(_ticks), endTime(0), conditionType(_type), buff(_buff)
{
	//
}

bool Condition::setParam(ConditionParam_t param, int32_t value)
{
	switch(param)
	{
		case CONDITIONPARAM_TICKS:
			ticks = value;
			return true;

		case CONDITIONPARAM_BUFF:
			buff = (value != 0);
			return true;

		case CONDITIONPARAM_SUBID:
			subId = value;
			return true;

		default:
			break;
	}

	return false;
}

bool Condition::unserialize(PropStream& propStream)
{
	uint8_t attrType;
	while(propStream.getByte(attrType) && attrType != CONDITIONATTR_END)
	{
		if(!unserializeProp((ConditionAttr_t)attrType, propStream))
			return false;
	}

	return true;
}

bool Condition::unserializeProp(ConditionAttr_t attr, PropStream& propStream)
{
	switch(attr)
	{
		case CONDITIONATTR_TYPE:
		{
			int32_t value = 0;
			if(!propStream.getType(value))
				return false;

			conditionType = (ConditionType_t)value;
			return true;
		}

		case CONDITIONATTR_ID:
		{
			int32_t value = 0;
			if(!propStream.getType(value))
				return false;

			id = (ConditionId_t)value;
			return true;
		}

		case CONDITIONATTR_TICKS:
		{
			int32_t value = 0;
			if(!propStream.getType(value))
				return false;

			ticks = value;
			return true;
		}

		case CONDITIONATTR_BUFF:
		{
			int32_t value = 0;
			if(!propStream.getType(value))
				return false;

			buff = (value != 0);
			return true;
		}

		case CONDITIONATTR_SUBID:
		{
			int32_t value = 0;
			if(!propStream.getType(value))
				return false;

			subId = value;
			return true;
		}

		case CONDITIONATTR_END:
			return true;

		default:
			break;
	}

	return false;
}

bool Condition::serialize(PropWriteStream& propWriteStream)
{
	propWriteStream.addByte(CONDITIONATTR_TYPE);
	propWriteStream.addType((int32_t)conditionType);

	propWriteStream.addByte(CONDITIONATTR_ID);
	propWriteStream.addType((int32_t)id);

	propWriteStream.addByte(CONDITIONATTR_TICKS);
	propWriteStream.addType((int32_t)ticks);

	propWriteStream.addByte(CONDITIONATTR_BUFF);
	propWriteStream.addType((int32_t)buff ? 1 : 0);

	propWriteStream.addByte(CONDITIONATTR_SUBID);
	propWriteStream.addType((int32_t)subId);
	return true;
}

void Condition::setTicks(int32_t _ticks)
{
	ticks = _ticks;
	if(_ticks > 0)
		endTime = OTSYS_TIME() + _ticks;
}

bool Condition::startCondition(Creature*)
{
	if(ticks > 0)
		endTime = OTSYS_TIME() + ticks;

	return true;
}

bool Condition::executeCondition(Creature* creature, int32_t interval)
{
	if(interval > 0)
	{
		bool tmp = false;
		creature->onTickCondition(getType(), interval, tmp);
	}

	if(ticks == -1)
		return true;

	ticks = std::max((int32_t)0, (ticks - interval));
	return (endTime >= OTSYS_TIME());
}

Condition* Condition::createCondition(ConditionId_t _id, ConditionType_t _type, int32_t _ticks, int32_t param/* = 0*/, bool _buff/* = false*/, uint32_t _subId/* = 0*/)
{
	switch((int32_t)_type)
	{
		case CONDITION_FIRE:
		case CONDITION_ENERGY:
		case CONDITION_POISON:
		case CONDITION_FREEZING:
		case CONDITION_DAZZLED:
		case CONDITION_CURSED:
		case CONDITION_DROWN:
		case CONDITION_BLEEDING:
			return new ConditionDamage(_id, _type, _buff, _subId);

		case CONDITION_HASTE:
		case CONDITION_PARALYZE:
			return new ConditionSpeed(_id, _type, _ticks, _buff, _subId, param);

		case CONDITION_OUTFIT:
			return new ConditionOutfit(_id, _type, _ticks, _buff, _subId);

		case CONDITION_LIGHT:
			return new ConditionLight(_id, _type, _ticks, _buff, _subId, param & 0xFF, (param & 0xFF00) >> 8);

		case CONDITION_REGENERATION:
			return new ConditionRegeneration(_id, _type, _ticks, _buff, _subId);

		case CONDITION_SOUL:
			return new ConditionSoul(_id, _type, _ticks, _buff, _subId);

		case CONDITION_MANASHIELD:
			return new ConditionManaShield(_id, _type, _ticks, _buff, _subId);

		case CONDITION_ATTRIBUTES:
			return new ConditionAttributes(_id, _type, _ticks, _buff, _subId);

		case CONDITION_INVISIBLE:
		case CONDITION_HUNTING:
		case CONDITION_INFIGHT:
		case CONDITION_MUTED:
		case CONDITION_EXHAUST:
		case CONDITION_DRUNK:
		case CONDITION_PACIFIED:
		case CONDITION_GAMEMASTER:
		case CONDITION_SPELLCOOLDOWN:
			return new ConditionGeneric(_id, _type, _ticks, _buff, _subId);

		default:
			break;
	}

	return NULL;
}

Condition* Condition::createCondition(PropStream& propStream)
{
	uint8_t attr = 0;
	if(!propStream.getByte(attr) || attr != CONDITIONATTR_TYPE)
		return NULL;

	uint32_t _type = 0;
	if(!propStream.getLong(_type))
		return NULL;

	if(!propStream.getByte(attr) || attr != CONDITIONATTR_ID)
		return NULL;

	uint32_t _id = 0;
	if(!propStream.getLong(_id))
		return NULL;

	if(!propStream.getByte(attr) || attr != CONDITIONATTR_TICKS)
		return NULL;

	uint32_t _ticks = 0;
	if(!propStream.getLong(_ticks))
		return NULL;

	if(!propStream.getByte(attr) || attr != CONDITIONATTR_BUFF)
		return NULL;

	uint32_t _buff = 0;
	if(!propStream.getLong(_buff))
		return NULL;

	if(!propStream.getByte(attr) || attr != CONDITIONATTR_SUBID)
		return NULL;

	uint32_t _subId = 0;
	if(!propStream.getLong(_subId))
		return NULL;

	return createCondition((ConditionId_t)_id, (ConditionType_t)_type, _ticks, 0, (_buff != 0), _subId);
}

bool Condition::updateCondition(const Condition* addCondition)
{
	return conditionType == addCondition->getType() && (ticks != -1 || addCondition->getTicks() < 1)
		&& (addCondition->getTicks() < 0 || endTime <= (OTSYS_TIME() + addCondition->getTicks()));
}

Icons_t Condition::getIcons() const
{
	if(buff)
		return ICON_BUFF;

	return ICON_NONE;
}

ConditionGeneric::ConditionGeneric(ConditionId_t _id, ConditionType_t _type, int32_t _ticks, bool _buff, uint32_t _subId):
Condition(_id, _type, _ticks, _buff, _subId)
{
	// TODO: get rid of this?
}

void ConditionGeneric::addCondition(Creature*, const Condition* addCondition)
{
	if(updateCondition(addCondition))
		setTicks(addCondition->getTicks());
}

Icons_t ConditionGeneric::getIcons() const
{
	Icons_t icon = Condition::getIcons();
	if(icon != ICON_NONE)
		return icon;

	switch(conditionType)
	{
		case CONDITION_INFIGHT:
			return ICON_SWORDS;

		case CONDITION_DRUNK:
			return ICON_DRUNK;

		default:
			break;
	}

	return ICON_NONE;
}

ConditionManaShield::ConditionManaShield(ConditionId_t _id, ConditionType_t _type, int32_t _ticks, bool _buff, uint32_t _subId):
ConditionGeneric(_id, _type, _ticks, _buff, _subId)
{
	//
}

Icons_t ConditionManaShield::getIcons() const
{
	Icons_t icon = Condition::getIcons();
	if(icon != ICON_NONE)
		return icon;

	return ICON_MANASHIELD;
}

ConditionAttributes::ConditionAttributes(ConditionId_t _id, ConditionType_t _type, int32_t _ticks, bool _buff, uint32_t _subId):
ConditionGeneric(_id, _type, _ticks, _buff, _subId)
{
	currentSkill = currentStat = 0;
	memset(skills, 0, sizeof(skills));
	memset(skillsPercent, 0, sizeof(skillsPercent));
	memset(stats, 0, sizeof(stats));
	memset(statsPercent, 0, sizeof(statsPercent));
}

void ConditionAttributes::addCondition(Creature* creature, const Condition* addCondition)
{
	if(!updateCondition(addCondition))
		return;

	setTicks(addCondition->getTicks());
	const ConditionAttributes& conditionAttrs = static_cast<const ConditionAttributes&>(*addCondition);
	endCondition(creature, CONDITIONEND_ABORT);

	//Apply the new one
	memcpy(skills, conditionAttrs.skills, sizeof(skills));
	memcpy(skillsPercent, conditionAttrs.skillsPercent, sizeof(skillsPercent));
	memcpy(stats, conditionAttrs.stats, sizeof(stats));
	memcpy(statsPercent, conditionAttrs.statsPercent, sizeof(statsPercent));
	if(Player* player = creature->getPlayer())
	{
		updatePercentSkills(player);
		updateSkills(player);
		updatePercentStats(player);
		updateStats(player);
	}
}

bool ConditionAttributes::unserializeProp(ConditionAttr_t attr, PropStream& propStream)
{
	switch(attr)
	{
		case CONDITIONATTR_SKILLS:
		{
			int32_t value = 0;
			if(!propStream.getType(value))
				return false;

			skills[currentSkill++] = value;
			return true;
		}

		case CONDITIONATTR_STATS:
		{
			int32_t value = 0;
			if(!propStream.getType(value))
				return false;

			stats[currentStat++] = value;
			return true;
		}

		default:
			break;
	}

	return ConditionGeneric::unserializeProp(attr, propStream);
}

bool ConditionAttributes::serialize(PropWriteStream& propWriteStream)
{
	if(!ConditionGeneric::serialize(propWriteStream))
		return false;

	for(int32_t i = SKILL_FIRST; i <= SKILL_LAST; ++i)
	{
		propWriteStream.addByte(CONDITIONATTR_SKILLS);
		propWriteStream.addType(skills[i]);
	}

	for(int32_t i = STAT_FIRST; i <= STAT_LAST; ++i)
	{
		propWriteStream.addByte(CONDITIONATTR_STATS);
		propWriteStream.addType(stats[i]);
	}

	return true;
}

bool ConditionAttributes::startCondition(Creature* creature)
{
	if(Player* player = creature->getPlayer())
	{
		updatePercentSkills(player);
		updateSkills(player);
		updatePercentStats(player);
		updateStats(player);
	}

	return Condition::startCondition(creature);
}

void ConditionAttributes::updatePercentSkills(Player* player)
{
	for(int32_t i = SKILL_FIRST; i <= SKILL_LAST; ++i)
	{
		if(skillsPercent[i])
			skills[i] += (int32_t)(player->getSkill((skills_t)i, SKILL_LEVEL) * ((skillsPercent[i] - 100) / 100.f));
	}
}

void ConditionAttributes::updatePercentStats(Player* player)
{
	for(int32_t i = STAT_FIRST; i <= STAT_LAST; ++i)
	{
		if(statsPercent[i])
			stats[i] += (int32_t)(player->getDefaultStats((stats_t)i)  * ((statsPercent[i] - 100) / 100.f));
	}
}

void ConditionAttributes::updateSkills(Player* player)
{
	bool needUpdateSkills = false;
	for(int32_t i = SKILL_FIRST; i <= SKILL_LAST; ++i)
	{
		if(!skills[i])
			continue;

		player->setVarSkill((skills_t)i, skills[i]);
		if(!needUpdateSkills)
			needUpdateSkills = true;
	}

	if(needUpdateSkills)
		player->sendSkills();
}

void ConditionAttributes::updateStats(Player* player)
{
	bool needUpdateStats = false;
	for(int32_t i = STAT_FIRST; i <= STAT_LAST; ++i)
	{
		if(!stats[i])
			continue;

		player->setVarStats((stats_t)i, stats[i]);
		if(!needUpdateStats)
			needUpdateStats = true;
	}

	if(needUpdateStats)
		player->sendStats();
}

bool ConditionAttributes::executeCondition(Creature* creature, int32_t interval)
{
	return ConditionGeneric::executeCondition(creature, interval);
}

void ConditionAttributes::endCondition(Creature* creature, ConditionEnd_t)
{
	Player* player = creature->getPlayer();
	if(!player)
		return;

	bool needUpdateSkills = false;
	for(int32_t i = SKILL_FIRST; i <= SKILL_LAST; ++i)
	{
		if(!skills[i])
			continue;

		needUpdateSkills = true;
		player->setVarSkill((skills_t)i, -skills[i]);
	}

	if(needUpdateSkills)
		player->sendSkills();

	bool needUpdateStats = false;
	for(int32_t i = STAT_FIRST; i <= STAT_LAST; ++i)
	{
		if(!stats[i])
			continue;

		needUpdateStats = true;
		player->setVarStats((stats_t)i, -stats[i]);
	}

	if(needUpdateStats)
		player->sendStats();
}

bool ConditionAttributes::setParam(ConditionParam_t param, int32_t value)
{
	bool ret = ConditionGeneric::setParam(param, value);
	switch(param)
	{
		case CONDITIONPARAM_SKILL_MELEE:
			skills[SKILL_CLUB] = skills[SKILL_AXE] = skills[SKILL_SWORD] = value;
			return true;

		case CONDITIONPARAM_SKILL_FIST:
			skills[SKILL_FIST] = value;
			return true;

		case CONDITIONPARAM_SKILL_CLUB:
			skills[SKILL_CLUB] = value;
			return true;

		case CONDITIONPARAM_SKILL_SWORD:
			skills[SKILL_SWORD] = value;
			return true;

		case CONDITIONPARAM_SKILL_AXE:
			skills[SKILL_AXE] = value;
			return true;

		case CONDITIONPARAM_SKILL_DISTANCE:
			skills[SKILL_DIST] = value;
			return true;

		case CONDITIONPARAM_SKILL_SHIELD:
			skills[SKILL_SHIELD] = value;
			return true;

		case CONDITIONPARAM_SKILL_FISHING:
			skills[SKILL_FISH] = value;
			return true;

		case CONDITIONPARAM_STAT_MAXHEALTH:
			stats[STAT_MAXHEALTH] = value;
			return true;

		case CONDITIONPARAM_STAT_MAXMANA:
			stats[STAT_MAXMANA] = value;
			return true;

		case CONDITIONPARAM_STAT_SOUL:
			stats[STAT_SOUL] = value;
			return true;

		case CONDITIONPARAM_STAT_MAGICLEVEL:
			stats[STAT_MAGICLEVEL] = value;
			return true;

		case CONDITIONPARAM_STAT_MAXHEALTHPERCENT:
			statsPercent[STAT_MAXHEALTH] = std::max((int32_t)0, value);
			return true;

		case CONDITIONPARAM_STAT_MAXMANAPERCENT:
			statsPercent[STAT_MAXMANA] = std::max((int32_t)0, value);
			return true;

		case CONDITIONPARAM_STAT_SOULPERCENT:
			statsPercent[STAT_SOUL] = std::max((int32_t)0, value);
			return true;

		case CONDITIONPARAM_STAT_MAGICLEVELPERCENT:
			statsPercent[STAT_MAGICLEVEL] = std::max((int32_t)0, value);
			return true;

		case CONDITIONPARAM_SKILL_MELEEPERCENT:
			skillsPercent[SKILL_CLUB] = skillsPercent[SKILL_AXE] = skillsPercent[SKILL_SWORD] = std::max((int32_t)0, value);
			return true;

		case CONDITIONPARAM_SKILL_FISTPERCENT:
			skillsPercent[SKILL_FIST] = std::max((int32_t)0, value);
			return true;

		case CONDITIONPARAM_SKILL_CLUBPERCENT:
			skillsPercent[SKILL_CLUB] = std::max((int32_t)0, value);
			return true;

		case CONDITIONPARAM_SKILL_SWORDPERCENT:
			skillsPercent[SKILL_SWORD] = std::max((int32_t)0, value);
			return true;

		case CONDITIONPARAM_SKILL_AXEPERCENT:
			skillsPercent[SKILL_AXE] = std::max((int32_t)0, value);
			return true;

		case CONDITIONPARAM_SKILL_DISTANCEPERCENT:
			skillsPercent[SKILL_DIST] = std::max((int32_t)0, value);
			return true;

		case CONDITIONPARAM_SKILL_SHIELDPERCENT:
			skillsPercent[SKILL_SHIELD] = std::max((int32_t)0, value);
			return true;

		case CONDITIONPARAM_SKILL_FISHINGPERCENT:
			skillsPercent[SKILL_FISH] = std::max((int32_t)0, value);
			return true;

		default:
			break;
	}

	return ret;
}

ConditionRegeneration::ConditionRegeneration(ConditionId_t _id, ConditionType_t _type, int32_t _ticks, bool _buff, uint32_t _subId):
ConditionGeneric(_id, _type, _ticks, _buff, _subId)
{
	internalHealthTicks = internalManaTicks = healthGain = manaGain = 0;
	healthTicks = manaTicks = 1000;
}

void ConditionRegeneration::addCondition(Creature*, const Condition* addCondition)
{
	if(!updateCondition(addCondition))
		return;

	setTicks(addCondition->getTicks());
	const ConditionRegeneration& conditionRegen = static_cast<const ConditionRegeneration&>(*addCondition);

	healthTicks = conditionRegen.healthTicks;
	manaTicks = conditionRegen.manaTicks;

	healthGain = conditionRegen.healthGain;
	manaGain = conditionRegen.manaGain;
}

bool ConditionRegeneration::unserializeProp(ConditionAttr_t attr, PropStream& propStream)
{
	switch(attr)
	{
		case CONDITIONATTR_HEALTHTICKS:
		{
			uint32_t value = 0;
			if(!propStream.getType(value))
				return false;

			healthTicks = value;
			return true;
		}

		case CONDITIONATTR_HEALTHGAIN:
		{
			uint32_t value = 0;
			if(!propStream.getType(value))
				return false;

			healthGain = value;
			return true;
		}

		case CONDITIONATTR_MANATICKS:
		{
			uint32_t value = 0;
			if(!propStream.getType(value))
				return false;

			manaTicks = value;
			return true;
		}

		case CONDITIONATTR_MANAGAIN:
		{
			uint32_t value = 0;
			if(!propStream.getType(value))
				return false;

			manaGain = value;
			return true;
		}

		default:
			break;
	}

	return ConditionGeneric::unserializeProp(attr, propStream);
}

bool ConditionRegeneration::serialize(PropWriteStream& propWriteStream)
{
	if(!ConditionGeneric::serialize(propWriteStream))
		return false;

	propWriteStream.addByte(CONDITIONATTR_HEALTHTICKS);
	propWriteStream.addType(healthTicks);

	propWriteStream.addByte(CONDITIONATTR_HEALTHGAIN);
	propWriteStream.addType(healthGain);

	propWriteStream.addByte(CONDITIONATTR_MANATICKS);
	propWriteStream.addType(manaTicks);

	propWriteStream.addByte(CONDITIONATTR_MANAGAIN);
	propWriteStream.addType(manaGain);
	return true;
}

bool ConditionRegeneration::executeCondition(Creature* creature, int32_t interval)
{
	internalManaTicks += interval;
	internalHealthTicks += interval;
	if(creature->getZone() != ZONE_PROTECTION)
	{
		if(internalHealthTicks >= healthTicks)
		{
			internalHealthTicks = 0;
			if(healthGain && creature->getHealth() < creature->getMaxHealth())
			{
				if(getSubId() != 0)
					g_game.combatChangeHealth(COMBAT_HEALING, creature, creature, healthGain);
				else
					creature->changeHealth(healthGain);
			}
		}

		if(internalManaTicks >= manaTicks)
		{
			internalManaTicks = 0;
			if(manaGain && creature->getMana() < creature->getMaxMana())
			{
				if(getSubId() != 0)
					g_game.combatChangeMana(creature, creature, manaGain);
				else
					creature->changeMana(manaGain);
			}
		}
	}

	return ConditionGeneric::executeCondition(creature, interval);
}

bool ConditionRegeneration::setParam(ConditionParam_t param, int32_t value)
{
	bool ret = ConditionGeneric::setParam(param, value);
	switch(param)
	{
		case CONDITIONPARAM_HEALTHGAIN:
			healthGain = value;
			return true;

		case CONDITIONPARAM_HEALTHTICKS:
			healthTicks = value;
			return true;

		case CONDITIONPARAM_MANAGAIN:
			manaGain = value;
			return true;

		case CONDITIONPARAM_MANATICKS:
			manaTicks = value;
			return true;

		default:
			break;
	}

	return ret;
}

ConditionSoul::ConditionSoul(ConditionId_t _id, ConditionType_t _type, int32_t _ticks, bool _buff, uint32_t _subId):
ConditionGeneric(_id, _type, _ticks, _buff, _subId)
{
	internalSoulTicks = soulTicks = soulGain = 0;
}

void ConditionSoul::addCondition(Creature*, const Condition* addCondition)
{
	if(!updateCondition(addCondition))
		return;

	setTicks(addCondition->getTicks());
	const ConditionSoul& conditionSoul = static_cast<const ConditionSoul&>(*addCondition);

	soulTicks = conditionSoul.soulTicks;
	soulGain = conditionSoul.soulGain;
}

bool ConditionSoul::unserializeProp(ConditionAttr_t attr, PropStream& propStream)
{
	switch(attr)
	{
		case CONDITIONATTR_SOULGAIN:
		{
			uint32_t value = 0;
			if(!propStream.getType(value))
				return false;

			soulGain = value;
			return true;
		}

		case CONDITIONATTR_SOULTICKS:
		{
			uint32_t value = 0;
			if(!propStream.getType(value))
				return false;

			soulTicks = value;
			return true;
		}

		default:
			break;
	}

	return ConditionGeneric::unserializeProp(attr, propStream);
}

bool ConditionSoul::serialize(PropWriteStream& propWriteStream)
{
	if(!ConditionGeneric::serialize(propWriteStream))
		return false;

	propWriteStream.addByte(CONDITIONATTR_SOULGAIN);
	propWriteStream.addType(soulGain);

	propWriteStream.addByte(CONDITIONATTR_SOULTICKS);
	propWriteStream.addType(soulTicks);

	return true;
}

bool ConditionSoul::executeCondition(Creature* creature, int32_t interval)
{
	internalSoulTicks += interval;
	if(Player* player = creature->getPlayer())
	{
		if(player->getZone() != ZONE_PROTECTION && internalSoulTicks >= soulTicks)
		{
			internalSoulTicks = 0;
			player->changeSoul(soulGain);
		}
	}

	return ConditionGeneric::executeCondition(creature, interval);
}

bool ConditionSoul::setParam(ConditionParam_t param, int32_t value)
{
	bool ret = ConditionGeneric::setParam(param, value);
	switch(param)
	{
		case CONDITIONPARAM_SOULGAIN:
			soulGain = value;
			return true;

		case CONDITIONPARAM_SOULTICKS:
			soulTicks = value;
			return true;

		default:
			break;
	}

	return ret;
}

ConditionDamage::ConditionDamage(ConditionId_t _id, ConditionType_t _type, bool _buff, uint32_t _subId):
Condition(_id, _type, 0, _buff, _subId)
{
	tickInterval = 2000;
	delayed = forceUpdate = false;
	owner = minDamage = maxDamage = startDamage = periodDamage = periodDamageTick = 0;
}

bool ConditionDamage::setParam(ConditionParam_t param, int32_t value)
{
	bool ret = Condition::setParam(param, value);
	switch(param)
	{
		case CONDITIONPARAM_OWNER:
			owner = value;
			return true;

		case CONDITIONPARAM_FORCEUPDATE:
			forceUpdate = (value != 0);
			return true;

		case CONDITIONPARAM_DELAYED:
			delayed = (value != 0);
			return true;

		case CONDITIONPARAM_MAXVALUE:
			maxDamage = std::abs(value);
			break;

		case CONDITIONPARAM_MINVALUE:
			minDamage = std::abs(value);
			break;

		case CONDITIONPARAM_STARTVALUE:
			startDamage = std::abs(value);
			break;

		case CONDITIONPARAM_TICKINTERVAL:
			tickInterval = std::abs(value);
			break;

		case CONDITIONPARAM_PERIODICDAMAGE:
			periodDamage = value;
			break;

		case CONDITIONPARAM_FIELD:
			field = (value != 0);
			break;

		default:
			break;
	}

	return ret;
}

bool ConditionDamage::unserializeProp(ConditionAttr_t attr, PropStream& propStream)
{
	switch(attr)
	{
		case CONDITIONATTR_DELAYED:
		{
			bool value = false;
			if(!propStream.getType(value))
				return false;

			delayed = value;
			return true;
		}

		case CONDITIONATTR_PERIODDAMAGE:
		{
			int32_t value = 0;
			if(!propStream.getType(value))
				return false;

			periodDamage = value;
			return true;
		}

		case CONDITIONATTR_OWNER:
		{
			uint32_t value = 0;
			if(!propStream.getType(value))
				return false;

			owner = value;
			return true;
		}

		case CONDITIONATTR_INTERVALDATA:
		{
			IntervalInfo damageInfo;
			if(!propStream.getType(damageInfo))
				return false;

			damageList.push_back(damageInfo);
			if(getTicks() != -1)
				setTicks(getTicks() + damageInfo.interval);

			return true;
		}

		default:
			break;
	}

	return Condition::unserializeProp(attr, propStream);
}

bool ConditionDamage::serialize(PropWriteStream& propWriteStream)
{
	if(!Condition::serialize(propWriteStream))
		return false;

	propWriteStream.addByte(CONDITIONATTR_DELAYED);
	propWriteStream.addType(delayed);

	propWriteStream.addByte(CONDITIONATTR_PERIODDAMAGE);
	propWriteStream.addType(periodDamage);

	propWriteStream.addByte(CONDITIONATTR_OWNER);
	propWriteStream.addType(owner);

	for(DamageList::const_iterator it = damageList.begin(); it != damageList.end(); ++it)
	{
		propWriteStream.addByte(CONDITIONATTR_INTERVALDATA);
		propWriteStream.addType(*it);
	}

	return true;
}

bool ConditionDamage::updateCondition(const ConditionDamage* addCondition)
{
	if(addCondition->doForceUpdate())
		return true;

	if(getTicks() == -1 && addCondition->getTicks() > 0)
		return false;

	if(addCondition->getTicks() <= getTicks())
		return false;

	if(addCondition->getTotalDamage() < getTotalDamage())
		return false;

	if(addCondition->periodDamage < periodDamage)
		return false;

	return true;
}

bool ConditionDamage::addDamage(int32_t rounds, int32_t time, int32_t value)
{
	if(rounds == -1) //periodic damage
	{
		setParam(CONDITIONPARAM_TICKINTERVAL, time);
		setParam(CONDITIONPARAM_TICKS, -1);

		periodDamage = value;
		return true;
	}

	if(periodDamage > 0)
		return false;

	//rounds, time, damage
	for(int32_t i = 0; i < rounds; ++i)
	{
		IntervalInfo damageInfo;
		damageInfo.interval = time;
		damageInfo.timeLeft = time;
		damageInfo.value = value;

		damageList.push_back(damageInfo);
		if(getTicks() != -1)
			setTicks(getTicks() + damageInfo.interval);
	}

	return true;
}

bool ConditionDamage::init()
{
	if(periodDamage)
		return true;

	if(!damageList.empty())
		return true;

	setTicks(0);
	int32_t amount = random_range(minDamage, maxDamage);
	if(!amount)
		return false;

	if(startDamage > maxDamage)
		startDamage = maxDamage;
	else if(!startDamage)
		startDamage = std::max((int32_t)1, (int32_t)std::ceil(((float)amount / 20.0)));

	std::list<int32_t> list;
	ConditionDamage::generateDamageList(amount, startDamage, list);
	for(std::list<int32_t>::iterator it = list.begin(); it != list.end(); ++it)
		addDamage(1, tickInterval, -(*it));

	return !damageList.empty();
}

bool ConditionDamage::startCondition(Creature* creature)
{
	if(!Condition::startCondition(creature) || !init())
		return false;

	if(delayed)
		return true;

	int32_t damage = 0;
	return !getNextDamage(damage) || doDamage(creature, damage);
}

bool ConditionDamage::executeCondition(Creature* creature, int32_t interval)
{
	if(periodDamage)
	{
		periodDamageTick += interval;
		if(periodDamageTick >= tickInterval)
		{
			periodDamageTick = 0;
			doDamage(creature, periodDamage);
		}
	}
	else if(!damageList.empty())
	{
		bool remove = getTicks() != -1;
		creature->onTickCondition(getType(), interval, remove);

		IntervalInfo& damageInfo = damageList.front();
		damageInfo.timeLeft -= interval;
		if(damageInfo.timeLeft <= 0)
		{
			int32_t damage = damageInfo.value;
			if(remove)
				damageList.pop_front();
			else
				damageInfo.timeLeft = damageInfo.interval;

			doDamage(creature, damage);
		}

		if(!remove)
		{
			if(getTicks() > 0)
				endTime += interval;

			interval = 0;
		}
	}

	return Condition::executeCondition(creature, interval);
}

bool ConditionDamage::getNextDamage(int32_t& damage)
{
	if(periodDamage)
	{
		damage = periodDamage;
		return true;
	}

	if(damageList.empty())
		return false;

	IntervalInfo& damageInfo = damageList.front();
	damage = damageInfo.value;
	if(getTicks() != -1)
		damageList.pop_front();

	return true;
}

bool ConditionDamage::doDamage(Creature* creature, int32_t damage)
{
	if(creature->isSuppress(getType()))
		return true;

	Creature* attacker = g_game.getCreatureByID(owner);
	if(damage < 0 && attacker && attacker->getPlayer() && creature->getPlayer() && creature->getPlayer()->getSkull() != SKULL_BLACK)
		damage = damage / 2;

	CombatType_t combatType = Combat::ConditionToDamageType(conditionType);
	if(g_game.combatBlockHit(combatType, attacker, creature, damage, false, false, field))
		return false;

	return g_game.combatChangeHealth(combatType, attacker, creature, damage);
}

void ConditionDamage::addCondition(Creature* creature, const Condition* addCondition)
{
	if(addCondition->getType() != conditionType)
		return;

	const ConditionDamage& conditionDamage = static_cast<const ConditionDamage&>(*addCondition);
	if(!updateCondition(&conditionDamage))
		return;

	setTicks(addCondition->getTicks());
	owner = conditionDamage.owner;
	maxDamage = conditionDamage.maxDamage;
	minDamage = conditionDamage.minDamage;
	startDamage = conditionDamage.startDamage;
	tickInterval = conditionDamage.tickInterval;
	periodDamage = conditionDamage.periodDamage;

	int32_t nextTimeLeft = tickInterval;
	if(!damageList.empty()) //save previous timeLeft
	{
		IntervalInfo& damageInfo = damageList.front();
		nextTimeLeft = damageInfo.timeLeft;
		damageList.clear();
	}

	damageList = conditionDamage.damageList;
	if(!init())
		return;

	if(!damageList.empty()) //restore last timeLeft
	{
		IntervalInfo& damageInfo = damageList.front();
		damageInfo.timeLeft = nextTimeLeft;
	}

	if(!delayed)
	{
		int32_t damage = 0;
		if(getNextDamage(damage))
			doDamage(creature, damage);
	}
}

int32_t ConditionDamage::getTotalDamage() const
{
	int32_t result = 0;
	if(!damageList.empty())
	{
		for(DamageList::const_iterator it = damageList.begin(); it != damageList.end(); ++it)
			result += it->value;
	}
	else
		result = maxDamage + (maxDamage - minDamage) / 2;

	return std::abs(result);
}

Icons_t ConditionDamage::getIcons() const
{
	Icons_t icon = Condition::getIcons();
	if(icon != ICON_NONE)
		return icon;

	switch(conditionType)
	{
		case CONDITION_FIRE:
			return ICON_BURN;

		case CONDITION_ENERGY:
			return ICON_ENERGY;

		case CONDITION_POISON:
			return ICON_POISON;

		case CONDITION_FREEZING:
			return ICON_FREEZING;

		case CONDITION_DAZZLED:
			return ICON_DAZZLED;

		case CONDITION_CURSED:
			return ICON_CURSED;

		case CONDITION_DROWN:
			return ICON_DROWNING;

		case CONDITION_BLEEDING:
			return ICON_BLEED;

		default:
			break;
	}

	return ICON_NONE;
}

void ConditionDamage::generateDamageList(int32_t amount, int32_t start, std::list<int32_t>& list)
{
	amount = std::abs(amount);
	start = std::abs(start);
	if(start >= amount)
	{
		list.push_back(start);
		return;
	}

	int32_t sum = 0, med = 0;
	float x1, x2;
	for(int32_t i = start; i > 0; --i)
	{
		med = ((start + 1 - i) * amount) / start;
		x1 = std::fabs(1.0 - (((float)sum) + i) / med);
		x2 = std::fabs(1.0 - (((float)sum) / med));

		while(x1 < x2)
		{
			sum += i;
			list.push_back(i);

			x1 = std::fabs(1.0 - (((float)sum) + i) / med);
			x2 = std::fabs(1.0 - (((float)sum) / med));
		}
	}
}

ConditionSpeed::ConditionSpeed(ConditionId_t _id, ConditionType_t _type, int32_t _ticks, bool _buff, uint32_t _subId, int32_t changeSpeed):
ConditionOutfit(_id, _type, _ticks, _buff, _subId)
{
	speedDelta = changeSpeed;
	mina = minb = maxa = maxb = 0.0f;
}

void ConditionSpeed::setFormulaVars(float _mina, float _minb, float _maxa, float _maxb)
{
	mina = _mina;
	minb = _minb;
	maxa = _maxa;
	maxb = _maxb;
}

void ConditionSpeed::getFormulaValues(int32_t var, int32_t& min, int32_t& max) const
{
	min = (int32_t)std::ceil(var * 1.f * mina + minb);
	max = (int32_t)std::ceil(var * 1.f * maxa + maxb);
}

bool ConditionSpeed::setParam(ConditionParam_t param, int32_t value)
{
	bool ret = ConditionOutfit::setParam(param, value);
	switch(param)
	{
		case CONDITIONPARAM_SPEED:
		{
			speedDelta = value;
			if(value > 0)
				conditionType = CONDITION_HASTE;
			else
				conditionType = CONDITION_PARALYZE;

			return true;
		}

		default:
			break;
	}

	return ret;
}

bool ConditionSpeed::unserializeProp(ConditionAttr_t attr, PropStream& propStream)
{
	switch(attr)
	{
		case CONDITIONATTR_SPEEDDELTA:
		{
			int32_t value = 0;
			if(!propStream.getType(value))
				return false;

			speedDelta = value;
			return true;
		}

		case CONDITIONATTR_FORMULA_MINA:
		{
			float value = 0;
			if(!propStream.getType(value))
				return false;

			mina = value;
			return true;
		}

		case CONDITIONATTR_FORMULA_MINB:
		{
			float value = 0;
			if(!propStream.getType(value))
				return false;

			minb = value;
			return true;
		}

		case CONDITIONATTR_FORMULA_MAXA:
		{
			float value = 0;
			if(!propStream.getType(value))
				return false;

			maxa = value;
			return true;
		}

		case CONDITIONATTR_FORMULA_MAXB:
		{
			float value = 0;
			if(!propStream.getType(value))
				return false;

			maxb = value;
			return true;
		}

		default:
			break;
	}

	return ConditionOutfit::unserializeProp(attr, propStream);
}

bool ConditionSpeed::serialize(PropWriteStream& propWriteStream)
{
	if(!ConditionOutfit::serialize(propWriteStream))
		return false;

	propWriteStream.addByte(CONDITIONATTR_SPEEDDELTA);
	propWriteStream.addType(speedDelta);

	propWriteStream.addByte(CONDITIONATTR_FORMULA_MINA);
	propWriteStream.addType(mina);

	propWriteStream.addByte(CONDITIONATTR_FORMULA_MINB);
	propWriteStream.addType(minb);

	propWriteStream.addByte(CONDITIONATTR_FORMULA_MAXA);
	propWriteStream.addType(maxa);

	propWriteStream.addByte(CONDITIONATTR_FORMULA_MAXB);
	propWriteStream.addType(maxb);
	return true;
}

bool ConditionSpeed::startCondition(Creature* creature)
{
	if(!speedDelta)
	{
		int32_t min, max;
		getFormulaValues(creature->getBaseSpeed(), min, max);
		speedDelta = random_range(min, max);
	}

	g_game.changeSpeed(creature, speedDelta);
	return ConditionOutfit::startCondition(creature);
}

void ConditionSpeed::endCondition(Creature* creature, ConditionEnd_t reason)
{
	ConditionOutfit::endCondition(creature, reason);
	g_game.changeSpeed(creature, -speedDelta);
}

void ConditionSpeed::addCondition(Creature* creature, const Condition* addCondition)
{
	if(conditionType != addCondition->getType() || (ticks == -1 && addCondition->getTicks() > 0))
		return;

	setTicks(addCondition->getTicks());
	const ConditionSpeed& conditionSpeed = static_cast<const ConditionSpeed&>(*addCondition);
	int32_t oldSpeedDelta = speedDelta;

	mina = conditionSpeed.mina;
	maxa = conditionSpeed.maxa;
	minb = conditionSpeed.minb;
	maxb = conditionSpeed.maxb;

	speedDelta = conditionSpeed.speedDelta;
	outfits = conditionSpeed.outfits;

	changeOutfit(creature);
	if(!speedDelta)
	{
		int32_t min, max;
		getFormulaValues(creature->getBaseSpeed(), min, max);
		speedDelta = random_range(min, max);
	}

	int32_t newSpeedChange = speedDelta - oldSpeedDelta;
	if(newSpeedChange)
		g_game.changeSpeed(creature, newSpeedChange);
}

Icons_t ConditionSpeed::getIcons() const
{
	Icons_t icon = Condition::getIcons();
	if(icon != ICON_NONE)
		return icon;

	switch(conditionType)
	{
		case CONDITION_HASTE:
			return ICON_HASTE;

		case CONDITION_PARALYZE:
			return ICON_PARALYZE;

		default:
			break;
	}

	return ICON_NONE;
}

ConditionOutfit::ConditionOutfit(ConditionId_t _id, ConditionType_t _type, int32_t _ticks, bool _buff, uint32_t _subId):
Condition(_id, _type, _ticks, _buff, _subId)
{
	//
}

bool ConditionOutfit::unserializeProp(ConditionAttr_t attr, PropStream& propStream)
{
	Outfit_t outfit;
	switch(attr)
	{
		case CONDITIONATTR_OUTFIT:
		{
			if(!propStream.getType(outfit))
				return false;

			outfits.push_back(outfit);
			return true;
		}

		default:
			break;
	}

	return Condition::unserializeProp(attr, propStream);
}

bool ConditionOutfit::serialize(PropWriteStream& propWriteStream)
{
	if(!Condition::serialize(propWriteStream))
		return false;

	for(std::vector<Outfit_t>::const_iterator it = outfits.begin(); it != outfits.end(); ++it)
	{
		propWriteStream.addByte(CONDITIONATTR_OUTFIT);
		propWriteStream.addType(*it);
	}

	return true;
}

bool ConditionOutfit::startCondition(Creature* creature)
{
	changeOutfit(creature);
	return Condition::startCondition(creature);
}

void ConditionOutfit::changeOutfit(Creature* creature, int32_t index/* = -1*/)
{
	if(outfits.empty())
		return;

	if(index == -1)
		index = random_range(0, outfits.size() - 1);

	g_game.internalCreatureChangeOutfit(creature, outfits[index], true);
}

void ConditionOutfit::endCondition(Creature* creature, ConditionEnd_t)
{
	if(!outfits.empty())
		g_game.internalCreatureChangeOutfit(creature, creature->getDefaultOutfit(), true);
}

void ConditionOutfit::addCondition(Creature* creature, const Condition* addCondition)
{
	if(!updateCondition(addCondition))
		return;

	setTicks(addCondition->getTicks());
	const ConditionOutfit& conditionOutfit = static_cast<const ConditionOutfit&>(*addCondition);

	outfits = conditionOutfit.outfits;
	changeOutfit(creature);
}

ConditionLight::ConditionLight(ConditionId_t _id, ConditionType_t _type, int32_t _ticks, bool _buff, uint32_t _subId, int32_t lightLevel, int32_t lightColor):
Condition(_id, _type, _ticks, _buff, _subId)
{
	lightInfo.level = lightLevel;
	lightInfo.color = lightColor;
	internalLightTicks = lightChangeInterval = 0;
}

bool ConditionLight::startCondition(Creature* creature)
{
	internalLightTicks = 0;
	lightChangeInterval = ticks / lightInfo.level;

	creature->setCreatureLight(lightInfo);
	g_game.changeLight(creature);
	return Condition::startCondition(creature);
}

bool ConditionLight::executeCondition(Creature* creature, int32_t interval)
{
	internalLightTicks += interval;
	if(internalLightTicks >= lightChangeInterval)
	{
		LightInfo creatureLight;
		creature->getCreatureLight(creatureLight);

		internalLightTicks = 0;
		if(creatureLight.level > 0)
		{
			--creatureLight.level;
			creature->setCreatureLight(creatureLight);
			g_game.changeLight(creature);
		}
	}

	return Condition::executeCondition(creature, interval);
}

void ConditionLight::endCondition(Creature* creature, ConditionEnd_t)
{
	creature->resetLight();
	g_game.changeLight(creature);
}

void ConditionLight::addCondition(Creature* creature, const Condition* addCondition)
{
	if(updateCondition(addCondition))
	{
		setTicks(addCondition->getTicks());
		const ConditionLight& conditionLight = static_cast<const ConditionLight&>(*addCondition);

		lightInfo.level = conditionLight.lightInfo.level;
		lightInfo.color = conditionLight.lightInfo.color;

		lightChangeInterval = getTicks() / lightInfo.level;
		internalLightTicks = 0;

		creature->setCreatureLight(lightInfo);
		g_game.changeLight(creature);
	}
}

bool ConditionLight::setParam(ConditionParam_t param, int32_t value)
{
	bool ret = Condition::setParam(param, value);
	switch(param)
	{
		case CONDITIONPARAM_LIGHT_LEVEL:
			lightInfo.level = value;
			return true;

		case CONDITIONPARAM_LIGHT_COLOR:
			lightInfo.color = value;
			return true;

		default:
			break;
	}

	return ret;
}

bool ConditionLight::unserializeProp(ConditionAttr_t attr, PropStream& propStream)
{
	switch(attr)
	{
		case CONDITIONATTR_LIGHTCOLOR:
		{
			uint32_t value = 0;
			if(!propStream.getType(value))
				return false;

			lightInfo.color = value;
			return true;
		}

		case CONDITIONATTR_LIGHTLEVEL:
		{
			uint32_t value = 0;
			if(!propStream.getType(value))
				return false;

			lightInfo.level = value;
			return true;
		}

		case CONDITIONATTR_LIGHTTICKS:
		{
			uint32_t value = 0;
			if(!propStream.getType(value))
				return false;

			internalLightTicks = value;
			return true;
		}

		case CONDITIONATTR_LIGHTINTERVAL:
		{
			uint32_t value = 0;
			if(!propStream.getType(value))
				return false;

			lightChangeInterval = value;
			return true;
		}

		default:
			break;
	}

	return Condition::unserializeProp(attr, propStream);
}

bool ConditionLight::serialize(PropWriteStream& propWriteStream)
{
	if(!Condition::serialize(propWriteStream))
		return false;

	propWriteStream.addByte(CONDITIONATTR_LIGHTCOLOR);
	propWriteStream.addType(lightInfo.color);

	propWriteStream.addByte(CONDITIONATTR_LIGHTLEVEL);
	propWriteStream.addType(lightInfo.level);

	propWriteStream.addByte(CONDITIONATTR_LIGHTTICKS);
	propWriteStream.addType(internalLightTicks);

	propWriteStream.addByte(CONDITIONATTR_LIGHTINTERVAL);
	propWriteStream.addType(lightChangeInterval);

	return true;
}
