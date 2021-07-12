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
#include "const.h"
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <boost/algorithm/string.hpp>

#include "spells.h"
#include "tools.h"

#include "house.h"
#include "housetile.h"
#include "combat.h"

#include "monsters.h"
#include "configmanager.h"
#include "game.h"

extern Game g_game;
extern Spells* g_spells;
extern Monsters g_monsters;
extern ConfigManager g_config;

Spells::Spells():
m_interface("Spell Interface")
{
	m_interface.initState();
	spellId = 0;
}

ReturnValue Spells::onPlayerSay(Player* player, const std::string& words)
{
	std::string reWords = words;
	trimString(reWords);

	InstantSpell* instantSpell = getInstantSpell(reWords);
	if(!instantSpell)
		return RET_NOTPOSSIBLE;

	size_t size = instantSpell->getWords().length();
	std::string param = reWords.substr(size, reWords.length() - size), reParam = "";
	if(instantSpell->getHasParam() && !param.empty() && param[0] == ' ')
	{
		size_t quote = param.find('"', 1);
		if(quote != std::string::npos)
		{
			size_t tmp = param.find('"', quote + 1);
			if(tmp == std::string::npos)
				tmp = param.length();

			reParam = param.substr(quote + 1, tmp - quote - 1);
		}
		else if(param.find(' ', 1) == std::string::npos)
			reParam = param.substr(1, param.length());

		trimString(reParam);
	}

	Position pos = player->getPosition();
	if(!instantSpell->castInstant(player, reParam))
		return RET_NEEDEXCHANGE;

	MessageClasses type = MSG_SPEAK_SAY;
	if(g_config.getBool(ConfigManager::EMOTE_SPELLS))
		type = MSG_SPEAK_MONSTER_SAY;

	if(!g_config.getBool(ConfigManager::SPELL_NAME_INSTEAD_WORDS))
	{
		if(g_config.getBool(ConfigManager::UNIFIED_SPELLS))
		{
			reWords = instantSpell->getWords();
			if(instantSpell->getHasParam())
				reWords += " \"" + reParam + "\"";
		}

		return g_game.internalCreatureSay(player, type, reWords, player->isGhost()) ?
			RET_NOERROR : RET_NOTPOSSIBLE;
	}

	std::string ret = instantSpell->getName();
	if(param.length())
	{
		trimString(param);
		size_t tmp = 0, rtmp = param.length();
		if(param[0] == '"')
			tmp = 1;

		if(param[rtmp] == '"')
			rtmp -= 1;

		ret += ": " + param.substr(tmp, rtmp);
	}

	return g_game.internalCreatureSay(player, type, ret, player->isGhost(),
		NULL, &pos) ? RET_NOERROR : RET_NOTPOSSIBLE;
}

void Spells::clear()
{
	for(RunesMap::iterator rit = runes.begin(); rit != runes.end(); ++rit)
		delete rit->second;

	runes.clear();
	for(InstantsMap::iterator it = instants.begin(); it != instants.end(); ++it)
		delete it->second;

	instants.clear();

	instantsWithParam.clear();
	spellId = 0;
	m_interface.reInitState();
}

Event* Spells::getEvent(const std::string& nodeName)
{
	std::string tmpNodeName = asLowerCaseString(nodeName);
	if(tmpNodeName == "rune")
		return new RuneSpell(&m_interface);

	if(tmpNodeName == "instant")
		return new InstantSpell(&m_interface);

	if(tmpNodeName == "conjure")
		return new ConjureSpell(&m_interface);

	return NULL;
}

bool Spells::registerEvent(Event* event, xmlNodePtr, bool override)
{
	if(InstantSpell* instant = dynamic_cast<InstantSpell*>(event))
	{
		std::string lowerWords = asLowerCaseString(instant->getWords());
		InstantsMap::iterator it = instants.find(lowerWords);
		instant->setId(spellId++);
		if(it == instants.end())
		{
			instants[lowerWords] = instant;
			if(instant->getHasParam()) {

				InstantsMap::iterator itt = instantsWithParam.find(lowerWords);
				if(itt == instantsWithParam.end()) {
					instantsWithParam[lowerWords] = instant;
				}
			}
			return true;
		}
		if(override)
		{
			delete it->second;
			it->second = instant;
			return true;
		}

		std::clog << "[Warning - Spells::registerEvent] Duplicate registered instant spell with words: " << instant->getWords() << std::endl;
		return false;
	}

	if(RuneSpell* rune = dynamic_cast<RuneSpell*>(event))
	{
		RunesMap::iterator it = runes.find(rune->getRuneItemId());
		rune->setId(spellId++);
		if(it == runes.end())
		{
			runes[rune->getRuneItemId()] = rune;
			return true;
		}

		if(override)
		{
			delete it->second;
			it->second = rune;
			return true;
		}

		std::clog << "[Warning - Spells::registerEvent] Duplicate registered rune with id: " << rune->getRuneItemId() << std::endl;
		return false;
	}

	return false;
}

Spell* Spells::getSpellByName(const std::string& name)
{
	Spell* spell;
	if((spell = getRuneSpellByName(name)) || (spell = getInstantSpellByName(name)))
		return spell;

	return NULL;
}

RuneSpell* Spells::getRuneSpell(uint32_t id)
{
	RunesMap::iterator it = runes.find(id);
	if(it != runes.end())
		return it->second;

	return NULL;
}

RuneSpell* Spells::getRuneSpellByName(const std::string& name)
{
	for(RunesMap::iterator it = runes.begin(); it != runes.end(); ++it)
	{
		if(boost::algorithm::iequals(it->second->getName(), name))
			return it->second;
	}

	return NULL;
}

InstantSpell* Spells::getInstantSpell(const std::string& words)
{
	InstantSpell* result = NULL;

	std::string lower = words;
	boost::to_lower(lower);

	//First let's search on all nom-param spells
	auto search2 = instants.find(lower);
	if(search2 != instants.end()) {
		result = search2->second;
	}

	//If we didn't find anything, we search on the param-spells
	if(result == NULL) {

		for(InstantsMap::iterator it = instantsWithParam.begin(); it != instantsWithParam.end(); ++it) {
			InstantSpell* instantSpell = it->second;
			const std::string& instantSpellWords = instantSpell->getWords();
			size_t spellLen = instantSpellWords.length();

			if(strncasecmp(instantSpellWords.c_str(), words.c_str(), spellLen) == 0) {
				if(!result || spellLen > result->getWords().length()) {
					result = instantSpell;
				}
			}
		}
	}

	if (result && words.length() > result->getWords().length())
	{
		std::string param = words.substr(result->getWords().length(), words.length());
		if (param[0] != ' ' || (param.length() > 1 && (!result->getHasParam() || param.find(' ', 1) != std::string::npos) && param[1] != '"'))
			return NULL;
	}

	return result;
}

uint32_t Spells::getInstantSpellCount(const Player* player)
{
	uint32_t count = 0;
	for(InstantsMap::iterator it = instants.begin(); it != instants.end(); ++it)
	{
		if(it->second->canCast(player))
			++count;
	}

	return count;
}

InstantSpell* Spells::getInstantSpellByIndex(const Player* player, uint32_t index)
{
	uint32_t count = 0;
	for(InstantsMap::iterator it = instants.begin(); it != instants.end(); ++it)
	{
		InstantSpell* instantSpell = it->second;
		if(!instantSpell->canCast(player))
			continue;

		if(count == index)
			return instantSpell;

		++count;
	}

	return NULL;
}

InstantSpell* Spells::getInstantSpellByName(const std::string& name)
{
	const char* cName = name.c_str();
	for (InstantsMap::iterator it = instants.begin(); it != instants.end(); ++it)
	{
		if (strcasecmp(cName, it->second->getName().c_str()) == 0)
			return it->second;
	}

	return NULL;
}

Position Spells::getCasterPosition(Creature* creature, Direction dir)
{
	return getNextPosition(dir, creature->getPosition());
}

bool BaseSpell::castSpell(Creature* creature)
{
	if(!creature)
		return false;

	bool success = true;
	CreatureEventList castEvents = creature->getCreatureEvents(CREATURE_EVENT_CAST);
	for(CreatureEventList::iterator it = castEvents.begin(); it != castEvents.end(); ++it)
	{
		if(!(*it)->executeCast(creature) && success)
			success = false;
	}

	return success;
}

bool BaseSpell::castSpell(Creature* creature, Creature* target)
{
	if(!creature || !target)
		return false;

	bool success = true;
	CreatureEventList castEvents = creature->getCreatureEvents(CREATURE_EVENT_CAST);
	for(CreatureEventList::iterator it = castEvents.begin(); it != castEvents.end(); ++it)
	{
		if(!(*it)->executeCast(creature, target) && success)
			success = false;
	}

	return success;
}

CombatSpell::CombatSpell(Combat* _combat, bool _needTarget, bool _needDirection) :
	Event(&g_spells->getInterface())
{
	combat =_combat;
	needTarget = _needTarget;
	needDirection = _needDirection;
}

CombatSpell::~CombatSpell()
{
	delete combat;
}

bool CombatSpell::loadScriptCombat()
{
	if(m_interface->reserveEnv())
	{
		ScriptEnviroment* env = m_interface->getEnv();
		combat = env->getCombatObject(env->getLastCombatId());

		env->resetCallback();
		m_interface->releaseEnv();
	}

	return combat != NULL;
}

bool CombatSpell::castSpell(Creature* creature)
{
	if(!BaseSpell::castSpell(creature))
		return false;

	if(isScripted())
	{
		LuaVariant var;
		var.type = VARIANT_POSITION;
		if(needDirection)
			var.pos = Spells::getCasterPosition(creature, creature->getDirection());
		else
			var.pos = creature->getPosition();

		return executeCastSpell(creature, var);
	}

	Position pos;
	if(needDirection)
		pos = Spells::getCasterPosition(creature, creature->getDirection());
	else
		pos = creature->getPosition();

	combat->doCombat(creature, pos);
	return true;
}

bool CombatSpell::castSpell(Creature* creature, Creature* target)
{
	if(!BaseSpell::castSpell(creature, target))
		return false;

	if(isScripted())
	{
		LuaVariant var;
		if(combat->hasArea())
		{
			var.type = VARIANT_POSITION;
			if(needTarget)
				var.pos = target->getPosition();
			else if(needDirection)
				var.pos = Spells::getCasterPosition(creature, creature->getDirection());
			else
				var.pos = creature->getPosition();
		}
		else
		{
			var.type = VARIANT_NUMBER;
			var.number = target->getID();
		}

		return executeCastSpell(creature, var);
	}

	if(combat->hasArea())
	{
		if(!needTarget)
			return castSpell(creature);

		combat->doCombat(creature, target->getPosition());
	}
	else
		combat->doCombat(creature, target);

	return true;
}

bool CombatSpell::executeCastSpell(Creature* creature, const LuaVariant& var)
{
	//onCastSpell(cid, var)
	if(m_interface->reserveEnv())
	{
		ScriptEnviroment* env = m_interface->getEnv();
		if(m_scripted == EVENT_SCRIPT_BUFFER)
		{
			env->setRealPos(creature->getPosition());
			std::ostringstream scriptstream;

			scriptstream << "local cid = " << env->addThing(creature) << std::endl;
			env->streamVariant(scriptstream, "var", var);

			scriptstream << *m_scriptData;
			bool result = true;
			if(m_interface->loadBuffer(scriptstream.str()))
			{
				lua_State* L = m_interface->getState();
				result = m_interface->getGlobalBool(L, "_result", true);
			}

			m_interface->releaseEnv();
			return result;
		}
		else
		{
			#ifdef __DEBUG_LUASCRIPTS__
			char desc[60];
			sprintf(desc, "onCastSpell - %s", creature->getName().c_str());
			env->setEvent(desc);
			#endif

			env->setScriptId(m_scriptId, m_interface);
			env->setRealPos(creature->getPosition());
			lua_State* L = m_interface->getState();

			m_interface->pushFunction(m_scriptId);
			lua_pushnumber(L, env->addThing(creature));
			m_interface->pushVariant(L, var);

			bool result = m_interface->callFunction(2);
			m_interface->releaseEnv();
			return result;
		}
	}
	else
	{
		std::clog << "[Error - CombatSpell::executeCastSpell] Call stack overflow." << std::endl;
		return false;
	}
}

Spell::Spell()
{
	spellId = 0;
	level = 0;
	magLevel = 0;
	mana = 0;
	exhaustedGroup = "none";
	manaPercent = 0;
	soul = 0;
	range = -1;
	exhaustion = 1000;
	needTarget = false;
	needWeapon = false;
	selfTarget = false;
	blockingSolid = false;
	blockingCreature = false;
	enabled = true;
	premium = false;
	isAggressive = true;
	learnable = false;

	for(int32_t i = SKILL_FIRST; i <= SKILL_LAST; ++i)
		skills[i] = 10;
}

bool Spell::configureSpell(xmlNodePtr p)
{
	int32_t intValue;
	std::string strValue;
	if(readXMLString(p, "name", strValue))
	{
		name = strValue;
		const char* reservedList[] =
		{
			"melee", "physical", "poison", "earth", "fire", "ice", "freeze", "energy", "drown", "death", "curse", "holy",
			"lifedrain", "manadrain", "healing", "speed", "outfit", "invisible", "drunk", "firefield", "poisonfield",
			"energyfield", "firecondition", "poisoncondition", "energycondition", "drowncondition", "freezecondition",
			"cursecondition"
		};

		for(uint32_t i = 0; i < sizeof(reservedList) / sizeof(const char*); ++i)
		{
			if(boost::algorithm::iequals(reservedList[i], name))
			{
				std::clog << "[Error - Spell::configureSpell] Spell is using a reserved name: " << reservedList[i] << std::endl;
				return false;
			}
		}
	}
	else
	{
		std::clog << "[Error - Spell::configureSpell] Spell without name." << std::endl;
		return false;
	}

	if(readXMLInteger(p, "lvl", intValue) || readXMLInteger(p, "level", intValue))
		level = intValue;

	if(readXMLInteger(p, "maglv", intValue) || readXMLInteger(p, "magiclevel", intValue))
		magLevel = intValue;

	if(readXMLString(p, "skill", strValue) || readXMLString(p, "skills", strValue) || readXMLInteger(p, "skillpoints", intValue))
	{
		std::vector<std::string> strVector = explodeString(strValue, ";"), tmpVector;
		for(std::vector<std::string>::iterator it = strVector.begin(); it != strVector.end(); ++it)
		{
			tmpVector = explodeString(*it, ",");
			if(tmpVector.size() > 1)
			{
				intValue = atoi(tmpVector[0].c_str());
				if(!intValue)
					skills[getSkillId(tmpVector[0])] = atoi(tmpVector[1].c_str());
				else
					skills[intValue] = atoi(tmpVector[1].c_str());
			}
		}
	}

	if(readXMLInteger(p, "mana", intValue))
		mana = intValue;

	if(readXMLInteger(p, "manapercent", intValue))
		manaPercent = intValue;

	if(readXMLInteger(p, "soul", intValue))
		soul = intValue;

	if(readXMLInteger(p, "exhaustion", intValue))
		exhaustion = intValue;

	if(readXMLString(p, "enabled", strValue))
		enabled = booleanString(strValue);

	if(readXMLString(p, "prem", strValue) || readXMLString(p, "premium", strValue))
		premium = booleanString(strValue);

	if(readXMLString(p, "needtarget", strValue))
		needTarget = booleanString(strValue);

	if(readXMLString(p, "needweapon", strValue))
		needWeapon = booleanString(strValue);

	if(readXMLString(p, "selftarget", strValue))
		selfTarget = booleanString(strValue);

	if(readXMLString(p, "needlearn", strValue))
		learnable = booleanString(strValue);

	if(readXMLInteger(p, "range", intValue))
		range = intValue;

	if(readXMLString(p, "blocktype", strValue))
	{
		std::string tmpStrValue = asLowerCaseString(strValue);
		if(tmpStrValue == "all")
			blockingCreature = blockingSolid = true;
		else if(tmpStrValue == "solid")
			blockingSolid = true;
		else if(tmpStrValue == "creature")
			blockingCreature = true;
		else
			std::clog << "[Warning - Spell::configureSpell] Blocktype \"" << strValue << "\" does not exist." << std::endl;
	}
	else if(readXMLString(p, "blocking", strValue))
		blockingCreature = blockingSolid = booleanString(strValue);

	if(readXMLString(p, "aggressive", strValue))
		isAggressive = booleanString(strValue);

	if(readXMLString(p, "exhaustedGroup", strValue))
	{
		std::string tmpStrValue = asLowerCaseString(strValue);
		if(tmpStrValue == "attack" || tmpStrValue == "attacking" || tmpStrValue == "heal" || tmpStrValue == "healing" || tmpStrValue == "support"
			|| tmpStrValue == "supporting" || tmpStrValue == "special" || tmpStrValue == "ultimate")
				exhaustedGroup = tmpStrValue;
		else
			std::clog << "[Warning - Spell::configureSpell] exhaustedGroup \"" << strValue << "\" does not exist." << std::endl;
	}
	else
	{
		if(!g_config.getBool(ConfigManager::NO_ATTACKHEALING_SIMULTANEUS))
			exhaustedGroup = isAggressive ? "attack" : "heal";
	}

	std::string error;
	for(xmlNodePtr vocationNode = p->children; vocationNode; vocationNode = vocationNode->next)
	{
		if(!parseVocationNode(vocationNode, vocSpellMap, vocStringVec, error))
			std::clog << "[Warning - Spell::configureSpell] Spell: " << name << ", " << error << std::endl;
	}

	return true;
}

bool Spell::checkSpell(Player* player) const
{
	if(player->hasFlag(PlayerFlag_CannotUseSpells))
		return false;

	if(player->hasFlag(PlayerFlag_IgnoreSpellCheck))
		return true;

	if(!isEnabled())
		return false;

	if(isAggressive)
	{
		if(!player->hasFlag(PlayerFlag_IgnoreProtectionZone) && player->getZone() == ZONE_PROTECTION)
		{
			player->sendCancelMessage(RET_ACTIONNOTPERMITTEDINPROTECTIONZONE);
			return false;
		}

		if(player->checkLoginDelay())
		{
			player->sendCancelMessage(RET_YOUMAYNOTATTACKIMMEDIATELYAFTERLOGGINGIN);
			player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
			return false;
		}
	}

	if(!player->hasFlag(PlayerFlag_HasNoExhaustion))
	{
		if(player->hasCondition(CONDITION_EXHAUST, EXHAUST_SPELLGROUP_NONE) && exhaustedGroup == "none")
		{
			player->sendCancelMessage(RET_YOUAREEXHAUSTED);
			if(isInstant())
				player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);

			return false;
		}
		if(g_config.getBool(ConfigManager::NO_ATTACKHEALING_SIMULTANEUS))
		{
			if((player->hasCondition(CONDITION_EXHAUST, EXHAUST_SPELLGROUP_ATTACK) && (exhaustedGroup == "heal" || exhaustedGroup == "healing")) ||
				(player->hasCondition(CONDITION_EXHAUST, EXHAUST_SPELLGROUP_HEALING) && (exhaustedGroup == "attack" || exhaustedGroup == "attack")))
			{
				player->sendCancelMessage(RET_YOUAREEXHAUSTED);
				if(isInstant())
					player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);

				return false;
			}
		}

		if(player->hasCondition(CONDITION_EXHAUST, EXHAUST_SPELLGROUP_ATTACK) && (exhaustedGroup == "attack" || exhaustedGroup == "attacking" ))
		{
			player->sendCancelMessage(RET_YOUAREEXHAUSTED);
			if(isInstant())
				player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);

			return false;
		}
		if(player->hasCondition(CONDITION_EXHAUST, EXHAUST_SPELLGROUP_HEALING) && (exhaustedGroup == "heal" || exhaustedGroup == "healing"))
		{
			player->sendCancelMessage(RET_YOUAREEXHAUSTED);
			if(isInstant())
				player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);

			return false;
		}
		if(player->hasCondition(CONDITION_EXHAUST, EXHAUST_SPELLGROUP_SUPPORT) && (exhaustedGroup == "support" || exhaustedGroup == "supporting"))
		{
			player->sendCancelMessage(RET_YOUAREEXHAUSTED);
			if(isInstant())
				player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);

			return false;
		}
		if(player->hasCondition(CONDITION_EXHAUST, EXHAUST_SPELLGROUP_SPECIAL) && (exhaustedGroup == "special" || exhaustedGroup == "ultimate"))
		{
			player->sendCancelMessage(RET_YOUAREEXHAUSTED);
			if(isInstant())
				player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);

			return false;
		}

	}

	if(isPremium() && !player->isPremium())
	{
		player->sendCancelMessage(RET_YOUNEEDPREMIUMACCOUNT);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	if(g_config.getBool(ConfigManager::USE_RUNE_REQUIREMENTS))
	{
		if((int32_t)player->getLevel() < level)
		{
			player->sendCancelMessage(RET_NOTENOUGHLEVEL);
			player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
			return false;
		}

		if((int32_t)player->getMagicLevel() < magLevel)
		{
			player->sendCancelMessage(RET_NOTENOUGHMAGICLEVEL);
			player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
			return false;
		}
	}

	for(int16_t i = SKILL_FIRST; i <= SKILL_LAST; ++i)
	{
		if((int32_t)player->getSkill((skills_t)i, SKILL_LEVEL) < skills[i])
		{
			player->sendCancelMessage(RET_NOTENOUGHSKILL);
			player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
			return false;
		}
	}

	if(player->getMana() < getManaCost(player) && !player->hasFlag(PlayerFlag_HasInfiniteMana))
	{
		player->sendCancelMessage(RET_NOTENOUGHMANA);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	if(player->getSoul() < soul && !player->hasFlag(PlayerFlag_HasInfiniteSoul))
	{
		player->sendCancelMessage(RET_NOTENOUGHSOUL);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	if(isInstant() && isLearnable() && !player->hasLearnedInstantSpell(getName()))
	{
		player->sendCancelMessage(RET_YOUNEEDTOLEARNTHISSPELL);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	if(!vocSpellMap.empty())
	{
		if(vocSpellMap.find(player->getVocationId()) == vocSpellMap.end())
		{
			player->sendCancelMessage(RET_YOURVOCATIONCANNOTUSETHISSPELL);
			player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
			return false;
		}
	}

	if(needWeapon)
	{
		switch(player->getWeaponType())
		{
			case WEAPON_SWORD:
			case WEAPON_CLUB:
			case WEAPON_AXE:
			case WEAPON_FIST:
				break;

			default:
			{
				player->sendCancelMessage(RET_YOUNEEDAWEAPONTOUSETHISSPELL);
				player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
				return false;
			}
		}
	}

	return true;
}

bool Spell::checkInstantSpell(Player* player, Creature* creature)
{
	if(!checkSpell(player))
		return false;

	const Position& toPos = creature->getPosition();
	const Position& playerPos = player->getPosition();
	if(playerPos.z > toPos.z)
	{
		player->sendCancelMessage(RET_FIRSTGOUPSTAIRS);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	if(playerPos.z < toPos.z)
	{
		player->sendCancelMessage(RET_FIRSTGODOWNSTAIRS);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	Tile* tile = g_game.getTile(toPos);
	if(!tile)
	{
		tile = new StaticTile(toPos.x, toPos.y, toPos.z);
		g_game.setTile(tile);
	}

	ReturnValue ret;
	if((ret = Combat::canDoCombat(player, tile, isAggressive, false)) != RET_NOERROR)
	{
		player->sendCancelMessage(ret);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	if(blockingCreature && creature)
	{
		player->sendCancelMessage(RET_NOTENOUGHROOM);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	if(blockingSolid && tile->hasProperty(BLOCKSOLID))
	{
		player->sendCancelMessage(RET_NOTENOUGHROOM);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	if(!needTarget)
	{
		if(g_config.getBool(ConfigManager::USE_BLACK_SKULL))
		{
			if(!isAggressive || player->getSkull() != SKULL_BLACK)
				return true;
		}
		else
		{
			if(!isAggressive)
				return true;
		}

		player->sendCancelMessage(RET_YOUMAYNOTCASTAREAONBLACKSKULL);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	if(!creature)
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	Player* targetPlayer = creature->getPlayer();
	if(!isAggressive || !targetPlayer || Combat::isInPvpZone(player, targetPlayer)
		|| player->getSkullType(targetPlayer) != SKULL_NONE)
		return true;

	if(player->getSecureMode() == SECUREMODE_ON)
	{
		player->sendCancelMessage(RET_TURNSECUREMODETOATTACKUNMARKEDPLAYERS);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	if(g_config.getBool(ConfigManager::USE_BLACK_SKULL))
	{
		if(player->getSkull() == SKULL_BLACK)
		{
			player->sendCancelMessage(RET_YOUMAYNOTATTACKTHISPLAYER);
			player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
			return false;
		}
	}

	return true;
}

bool Spell::checkInstantSpell(Player* player, const Position& toPos)
{
	if(!checkSpell(player))
		return false;

	if(toPos.x == 0xFFFF)
		return true;

	const Position& playerPos = player->getPosition();
	if(playerPos.z > toPos.z)
	{
		player->sendCancelMessage(RET_FIRSTGOUPSTAIRS);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	if(playerPos.z < toPos.z)
	{
		player->sendCancelMessage(RET_FIRSTGODOWNSTAIRS);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	Tile* tile = g_game.getTile(toPos);
	if(!tile)
	{
		tile = new StaticTile(toPos.x, toPos.y, toPos.z);
		g_game.setTile(tile);
	}

	ReturnValue ret;
	if((ret = Combat::canDoCombat(player, tile, isAggressive, false)) != RET_NOERROR)
	{
		player->sendCancelMessage(ret);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	if(blockingCreature && tile->getTopVisibleCreature(player))
	{
		player->sendCancelMessage(RET_NOTENOUGHROOM);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	if(blockingSolid && tile->hasProperty(BLOCKSOLID))
	{
		player->sendCancelMessage(RET_NOTENOUGHROOM);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	if(g_config.getBool(ConfigManager::USE_BLACK_SKULL))
	{
		if(player->getSkull() == SKULL_BLACK && isAggressive && range == -1) // CHECKME: -1 is (usually?) an area spell
		{
			player->sendCancelMessage(RET_YOUMAYNOTCASTAREAONBLACKSKULL);
			player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
			return false;
		}
	}

	return true;
}

bool Spell::checkRuneSpell(Player* player, const Position& toPos)
{
	if(!checkSpell(player))
		return false;

	if(toPos.x == 0xFFFF)
		return true;

	const Position& playerPos = player->getPosition();
	if(playerPos.z > toPos.z)
	{
		player->sendCancelMessage(RET_FIRSTGOUPSTAIRS);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	if(playerPos.z < toPos.z)
	{
		player->sendCancelMessage(RET_FIRSTGODOWNSTAIRS);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	Tile* tile = g_game.getTile(toPos);
	if(!tile)
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	if(range != -1 && !g_game.canThrowObjectTo(playerPos, toPos, true, range, range))
	{
		player->sendCancelMessage(RET_DESTINATIONOUTOFREACH);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	ReturnValue ret;
	if((ret = Combat::canDoCombat(player, tile, isAggressive, false)) != RET_NOERROR)
	{
		player->sendCancelMessage(ret);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	Creature* targetCreature = tile->getTopVisibleCreature(player);
	
	if (needTarget && targetCreature == player && isAggressive) {
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}
	
	if(blockingCreature && targetCreature && isAggressive)
	{
		player->sendCancelMessage(RET_NOTENOUGHROOM);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	if(blockingSolid && tile->hasProperty(BLOCKSOLID))
	{
		player->sendCancelMessage(RET_NOTENOUGHROOM);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	if(!needTarget)
	{
		if(g_config.getBool(ConfigManager::USE_BLACK_SKULL))
		{
			if(!isAggressive || player->getSkull() != SKULL_BLACK)
				return true;
		}
		else
		{
			if(!isAggressive)
				return true;
		}

		player->sendCancelMessage(RET_YOUMAYNOTCASTAREAONBLACKSKULL);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	if(!targetCreature)
	{
		player->sendCancelMessage(RET_CANONLYUSETHISRUNEONCREATURES);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	Player* targetPlayer = targetCreature->getPlayer();
	if(!isAggressive || !targetPlayer || Combat::isInPvpZone(player, targetPlayer)
		|| player->getSkullType(targetPlayer) != SKULL_NONE)
		return true;

	if(player->getSecureMode() == SECUREMODE_ON)
	{
		player->sendCancelMessage(RET_TURNSECUREMODETOATTACKUNMARKEDPLAYERS);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	if(g_config.getBool(ConfigManager::USE_BLACK_SKULL))
	{
		if(player->getSkull() != SKULL_BLACK)
			return true;
	}

	player->sendCancelMessage(RET_YOUMAYNOTATTACKTHISPLAYER);
	player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
	return false;
}

void Spell::postSpell(Player* player) const
{
	if(!player->hasFlag(PlayerFlag_HasNoExhaustion) && exhaustion > 0)
	{
		if(exhaustedGroup == "none")
			player->addExhaust(exhaustion, EXHAUST_SPELLGROUP_NONE);
		else if(exhaustedGroup == "attack" || exhaustedGroup == "attacking")
			player->addExhaust(exhaustion, EXHAUST_SPELLGROUP_ATTACK);
		else if(exhaustedGroup == "heal" || exhaustedGroup == "healing")
			player->addExhaust(exhaustion, EXHAUST_SPELLGROUP_HEALING);
		else if(exhaustedGroup == "support" || exhaustedGroup == "supporting")
			player->addExhaust(exhaustion, EXHAUST_SPELLGROUP_SUPPORT);
		else if(exhaustedGroup == "special" || exhaustedGroup == "ultimate")
			player->addExhaust(exhaustion, EXHAUST_SPELLGROUP_SPECIAL);
	}

	if(isAggressive && !player->hasFlag(PlayerFlag_NotGainInFight))
		player->addInFightTicks(false);

	postSpell(player, (uint32_t)getManaCost(player), (uint32_t)getSoulCost());
}

void Spell::postSpell(Player* player, uint32_t manaCost, uint32_t soulCost) const
{
	if(manaCost > 0)
	{
		player->changeMana(-(int32_t)manaCost);
		if(!player->hasFlag(PlayerFlag_NotGainMana) && (player->getZone() != ZONE_HARDCORE
			|| g_config.getBool(ConfigManager::PVPZONE_ADDMANASPENT)))
			player->addManaSpent(manaCost);
	}

	if(soulCost > 0)
		player->changeSoul(-(int32_t)soulCost);
}

int32_t Spell::getManaCost(const Player* player) const
{
	if(player && manaPercent)
		return (int32_t)std::floor((double)(player->getMaxMana() * manaPercent) / 100.);

	return mana;
}

ReturnValue Spell::CreateIllusion(Creature* creature, const Outfit_t& outfit, int32_t time)
{
	ConditionOutfit* outfitCondition = new ConditionOutfit(CONDITIONID_COMBAT, CONDITION_OUTFIT, time, false, 0);
	if(!outfitCondition)
		return RET_NOTPOSSIBLE;

	outfitCondition->addOutfit(outfit);
	creature->addCondition(outfitCondition);
	return RET_NOERROR;
}

ReturnValue Spell::CreateIllusion(Creature* creature, const std::string& name, int32_t time)
{
	uint32_t mId = g_monsters.getIdByName(name);
	if(!mId)
		return RET_CREATUREDOESNOTEXIST;

	const MonsterType* mType = g_monsters.getMonsterType(mId);
	if(!mType)
		return RET_CREATUREDOESNOTEXIST;

	Player* player = creature->getPlayer();
	if(player && !player->hasFlag(PlayerFlag_CanIllusionAll) && !mType->isIllusionable)
		return RET_NOTPOSSIBLE;

	return CreateIllusion(creature, mType->outfit, time);
}

ReturnValue Spell::CreateIllusion(Creature* creature, uint32_t itemId, int32_t time)
{
	const ItemType& it = Item::items[itemId];
	if(!it.id)
		return RET_NOTPOSSIBLE;

	Outfit_t outfit;
	outfit.lookTypeEx = itemId;
	return CreateIllusion(creature, outfit, time);
}

InstantSpell::InstantSpell(LuaInterface* _interface) : TalkAction(_interface)
{
	needDirection = false;
	hasParam = false;
	checkLineOfSight = true;
	casterTargetOrDirection = false;
	limitRange = 0;
	function = NULL;
}

bool InstantSpell::configureEvent(xmlNodePtr p)
{
	if(!Spell::configureSpell(p))
		return false;

	if(!TalkAction::configureEvent(p))
		return false;

	std::string strValue;
	if(readXMLString(p, "param", strValue) || readXMLString(p, "params", strValue))
		hasParam = booleanString(strValue);

	if(readXMLString(p, "direction", strValue))
		needDirection = booleanString(strValue);

	if(readXMLString(p, "casterTargetOrDirection", strValue))
		casterTargetOrDirection = booleanString(strValue);

	if(readXMLString(p, "blockwalls", strValue))
		checkLineOfSight = booleanString(strValue);

	int32_t intValue;
	if(readXMLInteger(p, "limitRange", intValue))
		limitRange = intValue;

	return true;
}

bool InstantSpell::loadFunction(const std::string& functionName)
{
	std::string tmpFunctionName = asLowerCaseString(functionName);
	if(tmpFunctionName == "summonmonster")
		function = SummonMonster;
	else if(tmpFunctionName == "searchplayer")
	{
		isAggressive = false;
		function = SearchPlayer;
	}
	else if(tmpFunctionName == "levitate")
	{
		isAggressive = false;
		function = Levitate;
	}
	else if(tmpFunctionName == "illusion")
	{
		isAggressive = false;
		function = Illusion;
	}
	else
	{
		std::clog << "[Warning - InstantSpell::loadFunction] Function \"" << functionName << "\" does not exist." << std::endl;
		return false;
	}

	m_scripted = EVENT_SCRIPT_FALSE;
	return true;
}

bool InstantSpell::castInstant(Player* player, const std::string& param)
{
	LuaVariant var;
	if(selfTarget)
	{
		var.type = VARIANT_NUMBER;
		var.number = player->getID();
		if(!checkInstantSpell(player, player))
			return false;
	}
	else if(needTarget || casterTargetOrDirection)
	{
		Creature* target = NULL;
		if(hasParam)
		{
			Player* targetPlayer = NULL;
			ReturnValue ret = g_game.getPlayerByNameWildcard(param, targetPlayer);

			target = targetPlayer;
			if(limitRange && target && !Position::areInRange(Position(limitRange,
				limitRange, 0), target->getPosition(), player->getPosition()))
				target = NULL;

			if((!target || target->getHealth() <= 0) && !casterTargetOrDirection)
			{
				player->sendCancelMessage(ret);
				player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
				return false;
			}
		}
		else
		{
			target = player->getAttackedCreature();
			if(limitRange && target && !Position::areInRange(Position(limitRange,
				limitRange, 0), target->getPosition(), player->getPosition()))
				target = NULL;

			if((!target || target->getHealth() <= 0) && !casterTargetOrDirection)
			{
				player->sendCancelMessage(RET_YOUCANONLYUSEITONCREATURES);
				player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
				return false;
			}
		}

		if(target)
		{
			bool canSee = player->canSeeCreature(target);
			if(!canSee || !canThrowSpell(player, target))
			{
				player->sendCancelMessage(canSee ? RET_CREATUREISNOTREACHABLE : RET_PLAYERWITHTHISNAMEISNOTONLINE);
				player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
				return false;
			}

			var.type = VARIANT_NUMBER;
			var.number = target->getID();
			if(!checkInstantSpell(player, target))
				return false;
		}
		else
		{
			var.type = VARIANT_POSITION;
			var.pos = Spells::getCasterPosition(player, player->getDirection());
			if(!checkInstantSpell(player, var.pos))
				return false;
		}
	}
	else if(hasParam)
	{
		var.type = VARIANT_STRING;
		var.text = param;
		if(!checkSpell(player))
			return false;
	}
	else
	{
		var.type = VARIANT_POSITION;
		if(needDirection)
			var.pos = Spells::getCasterPosition(player, player->getDirection());
		else
			var.pos = player->getPosition();

		if(!checkInstantSpell(player, var.pos))
			return false;
	}
	
	if(!BaseSpell::castSpell(player)) {
        player->sendCancelMessage(RET_NOTPOSSIBLE);
        player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
        return false;
    }

	if(!internalCastSpell(player, var))
		return false;

	Spell::postSpell(player);
	return true;
}

bool InstantSpell::canThrowSpell(const Creature* creature, const Creature* target) const
{
	const Position& fromPos = creature->getPosition();
	const Position& toPos = target->getPosition();
	return (!(fromPos.z != toPos.z || (range == -1 && !g_game.canThrowObjectTo(fromPos, toPos, checkLineOfSight))
		|| (range != -1 && !g_game.canThrowObjectTo(fromPos, toPos, checkLineOfSight, range, range))));
}

bool InstantSpell::castSpell(Creature* creature)
{
	if(!BaseSpell::castSpell(creature))
		return false;

	LuaVariant var;
	if(casterTargetOrDirection)
	{
		Creature* target = creature->getAttackedCreature();
		if(target && target->getHealth() > 0)
		{
			if(!creature->canSeeCreature(target) || !canThrowSpell(creature, target))
				return false;

			var.type = VARIANT_NUMBER;
			var.number = target->getID();
			return internalCastSpell(creature, var);
		}

		return false;
	}

	if(needDirection)
	{
		var.type = VARIANT_POSITION;
		var.pos = Spells::getCasterPosition(creature, creature->getDirection());
	}
	else
	{
		var.type = VARIANT_POSITION;
		var.pos = creature->getPosition();
	}

	return internalCastSpell(creature, var);
}

bool InstantSpell::castSpell(Creature* creature, Creature* target)
{
	if(!BaseSpell::castSpell(creature, target))
		return false;

	if(!needTarget)
		return castSpell(creature);

	LuaVariant var;
	var.type = VARIANT_NUMBER;
	var.number = target->getID();
	return internalCastSpell(creature, var);
}

bool InstantSpell::internalCastSpell(Creature* creature, const LuaVariant& var)
{
	if(isScripted())
		return executeCastSpell(creature, var);

	return function ? function(this, creature, var.text) : false;
}

bool InstantSpell::executeCastSpell(Creature* creature, const LuaVariant& var)
{
	//onCastSpell(cid, var)
	if(m_interface->reserveEnv())
	{
		ScriptEnviroment* env = m_interface->getEnv();
		if(m_scripted == EVENT_SCRIPT_BUFFER)
		{
			env->setRealPos(creature->getPosition());
			std::ostringstream scriptstream;

			scriptstream << "local cid = " << env->addThing(creature) << std::endl;
			env->streamVariant(scriptstream, "var", var);

			scriptstream << *m_scriptData;
			bool result = true;
			if(m_interface->loadBuffer(scriptstream.str()))
			{
				lua_State* L = m_interface->getState();
				result = m_interface->getGlobalBool(L, "_result", true);
			}

			m_interface->releaseEnv();
			return result;
		}
		else
		{
			#ifdef __DEBUG_LUASCRIPTS__
			char desc[60];
			sprintf(desc, "onCastSpell - %s", creature->getName().c_str());
			env->setEvent(desc);
			#endif

			env->setScriptId(m_scriptId, m_interface);
			env->setRealPos(creature->getPosition());
			lua_State* L = m_interface->getState();

			m_interface->pushFunction(m_scriptId);
			lua_pushnumber(L, env->addThing(creature));
			m_interface->pushVariant(L, var);

			bool result = m_interface->callFunction(2);
			m_interface->releaseEnv();
			return result;
		}
	}
	else
	{
		std::clog << "[Error - InstantSpell::executeCastSpell] Call stack overflow." << std::endl;
		return false;
	}
}

bool InstantSpell::SearchPlayer(const InstantSpell*, Creature* creature, const std::string& param)
{
	Player* player = creature->getPlayer();
	if(!player || player->isRemoved())
		return false;

	Player* targetPlayer = NULL;
	ReturnValue ret = g_game.getPlayerByNameWildcard(param, targetPlayer);
	if(ret != RET_NOERROR || !targetPlayer || targetPlayer->isRemoved())
	{
		player->sendCancelMessage(ret);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	if(targetPlayer->hasCustomFlag(PlayerCustomFlag_NotSearchable) && !player->hasCustomFlag(PlayerCustomFlag_GamemasterPrivileges))
	{
		player->sendCancelMessage(RET_PLAYERWITHTHISNAMEISNOTONLINE);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	std::ostringstream ss;
	ss << targetPlayer->getName() << " " << g_game.getSearchString(player->getPosition(), targetPlayer->getPosition(), true, true) << ".";
	player->sendTextMessage(MSG_INFO_DESCR, ss.str());

	player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_WRAPS_BLUE);
	return true;
}

bool InstantSpell::SummonMonster(const InstantSpell* spell, Creature* creature, const std::string& param)
{
	Player* player = creature->getPlayer();
	if(!player)
		return false;

	MonsterType* mType = g_monsters.getMonsterType(param);
	if(!mType)
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	int32_t manaCost = (int32_t)(mType->manaCost * g_config.getDouble(ConfigManager::RATE_MONSTER_MANA));
	if(!player->hasFlag(PlayerFlag_CanSummonAll))
	{
		if(g_config.getBool(ConfigManager::USE_BLACK_SKULL))
		{
			if(player->getSkull() == SKULL_BLACK)
			{
				player->sendCancelMessage(RET_NOTPOSSIBLE);
				player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
				return false;
			}
		}

		if(!mType->isSummonable)
		{
			player->sendCancelMessage(RET_NOTPOSSIBLE);
			player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
			return false;
		}

		if(player->getMana() < manaCost)
		{
			player->sendCancelMessage(RET_NOTENOUGHMANA);
			player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
			return false;
		}

		if((int32_t)player->getSummonCount() >= g_config.getNumber(ConfigManager::MAX_PLAYER_SUMMONS))
		{
			player->sendCancel("You cannot summon more creatures.");
			player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
			return false;
		}
	}

	ReturnValue ret = g_game.placeSummon(creature, param);
	if(ret == RET_NOERROR)
	{
		spell->postSpell(player, (uint32_t)manaCost, (uint32_t)spell->getSoulCost());
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_WRAPS_BLUE);//
		return true;
	}

	player->sendCancelMessage(ret);
	player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
	return false;
}

bool InstantSpell::Levitate(const InstantSpell*, Creature* creature, const std::string& param)
{
	Player* player = creature->getPlayer();
	if(!player)
		return false;

	uint16_t floor = 7;
	ReturnValue ret = RET_NOERROR;
	const Position& position = creature->getPosition();
	Position destination = Spells::getCasterPosition(creature, creature->getDirection());

	std::string tmpParam = asLowerCaseString(param);
	if(tmpParam == "up")
		floor = 8;
	else if(tmpParam != "down")
		ret = RET_NOTPOSSIBLE;

	if(ret == RET_NOERROR)
	{
		ret = RET_NOTPOSSIBLE;
		if(position.z != floor)
		{
			Tile* tmpTile = NULL;
			if(floor != 7)
			{
				tmpTile = g_game.getTile(position.x, position.y, position.z - 1);
				destination.z--;
			}
			else
			{
				tmpTile = g_game.getTile(destination);
				destination.z++;
			}

			if(!tmpTile || (!tmpTile->ground && !tmpTile->hasProperty(IMMOVABLEBLOCKSOLID)))
			{
				Tile* tile = player->getTile();
				tmpTile = g_game.getTile(destination);
				if(tile && tmpTile && tmpTile->ground && !tmpTile->hasProperty(IMMOVABLEBLOCKSOLID) &&
					!tmpTile->floorChange() && tile->hasFlag(TILESTATE_HOUSE) == tmpTile->hasFlag(TILESTATE_HOUSE)
					&& tile->hasFlag(TILESTATE_PROTECTIONZONE) == tmpTile->hasFlag(TILESTATE_PROTECTIONZONE))
					ret = g_game.internalMoveCreature(NULL, player, tile, tmpTile, FLAG_IGNOREBLOCKITEM | FLAG_IGNOREBLOCKCREATURE);
			}
		}
	}

	if(ret == RET_NOERROR)
	{
		g_game.addMagicEffect(player->getPosition(), MAGIC_EFFECT_TELEPORT, player->isGhost());
		return true;
	}

	player->sendCancelMessage(ret);
	g_game.addMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF, player->isGhost());
	return false;
}

bool InstantSpell::Illusion(const InstantSpell*, Creature* creature, const std::string& param)
{
	Player* player = creature->getPlayer();
	if(!player)
		return false;

	ReturnValue ret = CreateIllusion(creature, param, 60000);
	if(ret == RET_NOERROR)
	{
		g_game.addMagicEffect(player->getPosition(), MAGIC_EFFECT_WRAPS_RED);
		return true;
	}

	player->sendCancelMessage(ret);
	player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
	return (ret == RET_NOERROR);
}

bool InstantSpell::canCast(const Player* player) const
{
	if(player->hasFlag(PlayerFlag_CannotUseSpells))
		return false;

	if(player->hasFlag(PlayerFlag_IgnoreSpellCheck) || (!isLearnable() && (vocSpellMap.empty()
		|| vocSpellMap.find(player->getVocationId()) != vocSpellMap.end())))
		return true;

	return player->hasLearnedInstantSpell(getName());
}

ConjureSpell::ConjureSpell(LuaInterface* _interface):
	InstantSpell(_interface)
{
	isAggressive = false;
	conjureId = 0;
	conjureCount = 1;
	conjureReagentId = 0;
}

bool ConjureSpell::configureEvent(xmlNodePtr p)
{
	if(!InstantSpell::configureEvent(p))
		return false;

	int32_t intValue;
	if(readXMLInteger(p, "conjureId", intValue))
		conjureId = intValue;

	if(readXMLInteger(p, "conjureCount", intValue))
		conjureCount = intValue;
	else if(conjureId != 0)
	{
		//load the default charge from items.xml
		const ItemType& it = Item::items[conjureId];
		if(it.charges != 0)
			conjureCount = it.charges;
	}

	if(readXMLInteger(p, "reagentId", intValue))
		conjureReagentId = intValue;

	return true;
}

bool ConjureSpell::loadFunction(const std::string& functionName)
{
	std::string tmpFunctionName = asLowerCaseString(functionName);
	if(tmpFunctionName == "conjureitem" || tmpFunctionName == "conjurerune")
		function = ConjureItem;
	else
	{
		std::clog << "[Warning - ConjureSpell::loadFunction] Function \"" << functionName << "\" does not exist." << std::endl;
		return false;
	}

	m_scripted = EVENT_SCRIPT_FALSE;
	return true;
}

ReturnValue ConjureSpell::internalConjureItem(Player* player, uint32_t conjureId, uint32_t conjureCount,
	bool transform/* = false*/, uint32_t reagentId/* = 0*/)
{
	if(!transform)
	{
		Item* newItem = Item::CreateItem(conjureId, conjureCount);
		if(!newItem)
			return RET_NOTPOSSIBLE;

		ReturnValue ret = g_game.internalPlayerAddItem(player, player, newItem, true);
		if(ret != RET_NOERROR)
			delete newItem;
		else
			g_game.startDecay(newItem);
		return ret;
	}

	if(!reagentId)
		return RET_NOTPOSSIBLE;

	std::list<Container*> containers;
	Item *item = NULL, *fromItem = NULL;
	for(int32_t i = SLOT_FIRST; i < SLOT_LAST; ++i)
	{
		if(!(item = player->getInventoryItem((slots_t)i)))
			continue;

		if(!fromItem && item->getID() == reagentId)
			fromItem = item;
		else if(Container* container = item->getContainer())
			containers.push_back(container);
	}

	if(!fromItem)
	{
		for(std::list<Container*>::iterator cit = containers.begin(); cit != containers.end(); ++cit)
		{
			for(ItemList::const_reverse_iterator it = (*cit)->getReversedItems(); it != (*cit)->getReversedEnd(); ++it)
			{
				if((*it)->getID() == reagentId)
				{
					fromItem = (*it);
					break;
				}

				if(Container* tmp = (*it)->getContainer())
					containers.push_back(tmp);
			}
		}
	}

	if(!fromItem)
		return RET_YOUNEEDAMAGICITEMTOCASTSPELL;

	if((fromItem->isStackable() || fromItem->hasCharges()) && fromItem->getSubType() > 1)
	{
		item = Item::CreateItem(conjureId, conjureCount);
		ReturnValue ret = g_game.internalPlayerAddItem(NULL, player, item, false);
		if(ret != RET_NOERROR)
			return ret;

		g_game.transformItem(fromItem, reagentId, (int32_t)(fromItem->getItemCount() - 1));
	}
	else
		g_game.transformItem(fromItem, conjureId, conjureCount);

	g_game.startDecay(item);
	return RET_NOERROR;
}

bool ConjureSpell::ConjureItem(const ConjureSpell* spell, Creature* creature, const std::string&)
{
	Player* player = creature->getPlayer();
	if(!player)
		return false;

	if(!player->hasFlag(PlayerFlag_IgnoreSpellCheck) && player->getZone() == ZONE_HARDCORE)
	{
		player->sendCancelMessage(RET_CANNOTCONJUREITEMHERE);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	ReturnValue result = RET_NOTPOSSIBLE;
	if(spell->getReagentId() != 0)
	{
		if((result = internalConjureItem(player, spell->getConjureId(), spell->getConjureCount(), true, spell->getReagentId())) == RET_NOERROR)
		{
			spell->postSpell(player);
			g_game.addMagicEffect(player->getPosition(), MAGIC_EFFECT_WRAPS_RED);
			return true;
		}
	}
	else if((result = internalConjureItem(player, spell->getConjureId(), spell->getConjureCount())) == RET_NOERROR)
	{
		spell->postSpell(player);
		g_game.addMagicEffect(player->getPosition(), MAGIC_EFFECT_WRAPS_RED);
		return true;
	}

	player->sendCancelMessage(result);
	player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
	return false;
}

bool ConjureSpell::castInstant(Player* player, const std::string& param)
{
	if(!checkSpell(player))
		return false;

	if(!isScripted())
		return function ? function(this, player, param) : false;

	LuaVariant var;
	var.type = VARIANT_STRING;
	var.text = param;
	return executeCastSpell(player, var);
}

RuneSpell::RuneSpell(LuaInterface* _interface):
Action(_interface)
{
	runeId = 0;
	function = NULL;
	hasCharges = allowFarUse = true;
}

bool RuneSpell::configureEvent(xmlNodePtr p)
{
	if(!Spell::configureSpell(p))
		return false;

	if(!Action::configureEvent(p))
		return false;

	int32_t intValue;
	if(readXMLInteger(p, "id", intValue))
		runeId = intValue;
	else
	{
		std::clog << "Error: [RuneSpell::configureSpell] Rune spell without id." << std::endl;
		return false;
	}

	std::string strValue;
	if(readXMLString(p, "charges", strValue))
		hasCharges = booleanString(strValue);

	ItemType& it = Item::items.getItemType(runeId);
	if(g_config.getBool(ConfigManager::USE_RUNE_REQUIREMENTS))
	{
		if(level && level != it.runeLevel)
			it.runeLevel = level;

		if(magLevel && magLevel != it.runeMagLevel)
			it.runeMagLevel = magLevel;
	}

	it.vocationString = parseVocationString(vocStringVec);
	return true;
}

bool RuneSpell::loadFunction(const std::string& functionName)
{
	std::string tmpFunctionName = asLowerCaseString(functionName);
	if(tmpFunctionName == "chameleon")
		function = Illusion;
	else if(tmpFunctionName == "convince")
		function = Convince;
	else if(tmpFunctionName == "soulfire")
		function = Soulfire;
	else
	{
		std::clog << "[Warning - RuneSpell::loadFunction] Function \"" << functionName << "\" does not exist." << std::endl;
		return false;
	}

	m_scripted = EVENT_SCRIPT_FALSE;
	return true;
}

bool RuneSpell::Illusion(const RuneSpell*, Creature* creature, Item*, const Position&, const Position& posTo)
{
	Player* player = creature->getPlayer();
	if(!player)
		return false;

	Thing* thing = g_game.internalGetThing(player, posTo, 0, 0, STACKPOS_MOVE);
	if(!thing)
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	Item* illusionItem = thing->getItem();
	if(!illusionItem || !illusionItem->isMovable())
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	ReturnValue ret = CreateIllusion(creature, illusionItem->getID(), 60000);
	if(ret == RET_NOERROR)
	{
		g_game.addMagicEffect(player->getPosition(), MAGIC_EFFECT_WRAPS_RED);
		return true;
	}

	player->sendCancelMessage(ret);
	player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
	return false;
}

bool RuneSpell::Convince(const RuneSpell* spell, Creature* creature, Item*, const Position&, const Position& posTo)
{
	Player* player = creature->getPlayer();
	if(!player)
		return false;

	if(!player->hasFlag(PlayerFlag_CanConvinceAll))
	{
		if(g_config.getBool(ConfigManager::USE_BLACK_SKULL))
		{
			if(player->getSkull() == SKULL_BLACK)
			{
				player->sendCancelMessage(RET_NOTPOSSIBLE);
				player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
				return false;
			}
		}

		if((int32_t)player->getSummonCount() >= g_config.getNumber(ConfigManager::MAX_PLAYER_SUMMONS))
		{
			player->sendCancelMessage(RET_NOTPOSSIBLE);
			player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
			return false;
		}
	}

	Thing* thing = g_game.internalGetThing(player, posTo, 0, 0, STACKPOS_LOOK);
	if(!thing)
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	Creature* convinceCreature = thing->getCreature();
	if(!convinceCreature)
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	if(!player->hasFlag(PlayerFlag_CanConvinceAll) && convinceCreature->getPlayerMaster())
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	int32_t manaCost = 0;
	if(Monster* monster = convinceCreature->getMonster())
		manaCost = (int32_t)(monster->getManaCost() * g_config.getDouble(ConfigManager::RATE_MONSTER_MANA));

	if(!player->hasFlag(PlayerFlag_HasInfiniteMana) && player->getMana() < manaCost)
	{
		player->sendCancelMessage(RET_NOTENOUGHMANA);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	if(!convinceCreature->convinceCreature(creature))
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	spell->postSpell(player, (uint32_t)manaCost, (uint32_t)spell->getSoulCost());
	g_game.addMagicEffect(player->getPosition(), MAGIC_EFFECT_WRAPS_RED);
	return true;
}

bool RuneSpell::Soulfire(const RuneSpell* spell, Creature* creature, Item*, const Position&, const Position& posTo)
{
	Player* player = creature->getPlayer();
	if(!player)
		return false;

	Thing* thing = g_game.internalGetThing(player, posTo, 0, 0, STACKPOS_LOOK);
	if(!thing)
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	Creature* target = thing->getCreature();
	if(!target || !target->getPlayer())
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	ConditionDamage* soulfireCondition = new ConditionDamage(CONDITIONID_COMBAT, CONDITION_FIRE, false, 0);
	soulfireCondition->setParam(CONDITIONPARAM_SUBID, 1);
	soulfireCondition->setParam(CONDITIONPARAM_OWNER, player->getID());

	soulfireCondition->addDamage(std::ceil((player->getLevel() + player->getMagicLevel()) / 3.), 9000, -10);
	if(!target->addCondition(soulfireCondition))
	{
		player->sendCancelMessage(RET_NOTPOSSIBLE);
		player->sendMagicEffect(player->getPosition(), MAGIC_EFFECT_POFF);
		return false;
	}

	g_game.addDistanceEffect(player->getPosition(), posTo, SHOOT_EFFECT_FIRE);
	spell->postSpell(player, true, false);
	return true;
}

ReturnValue RuneSpell::canExecuteAction(const Player* player, const Position& toPos)
{
	if(player->hasFlag(PlayerFlag_CannotUseSpells))
		return RET_CANNOTUSETHISOBJECT;

	ReturnValue ret = Action::canExecuteAction(player, toPos);
	if(ret != RET_NOERROR)
		return ret;

	if(toPos.x == 0xFFFF)
	{
		if(needTarget)
			return RET_CANONLYUSETHISRUNEONCREATURES;

		if(!selfTarget)
			return RET_NOTENOUGHROOM;
	}

	return RET_NOERROR;
}

bool RuneSpell::executeUse(Player* player, Item* item, const PositionEx& posFrom,
	const PositionEx& posTo, bool, uint32_t creatureId)
{
	if(!checkRuneSpell(player, posTo))
		return false;

	bool result = false;
	if(isScripted())
	{
		LuaVariant var;
		if(needTarget)
		{
			if(!creatureId)
			{
				if(Tile* tileTo = g_game.getTile(posTo))
				{
					if(const Creature* creature = tileTo->getTopVisibleCreature(player))
						creatureId = creature->getID();
				}
			}

			var.type = VARIANT_NUMBER;
			var.number = creatureId;
		}
		else
		{
			var.type = VARIANT_POSITION;
			var.pos = posTo;
		}

		result = internalCastSpell(player, var);
	}
	else if(function)
		result = function(this, player, item, posFrom, posTo);

	if(result)
	{
		Spell::postSpell(player);
		if(hasCharges && item && g_config.getBool(ConfigManager::REMOVE_RUNE_CHARGES))
			g_game.transformItem(item, item->getID(), std::max((int32_t)0, ((int32_t)item->getItemCount()) - 1));
	}

	return result;
}

bool RuneSpell::castSpell(Creature* creature)
{
	if(!BaseSpell::castSpell(creature))
		return false;

	LuaVariant var;
	var.type = VARIANT_NUMBER;
	var.number = creature->getID();
	return internalCastSpell(creature, var);
}

bool RuneSpell::castSpell(Creature* creature, Creature* target)
{
	if(!BaseSpell::castSpell(creature, target))
		return false;

	LuaVariant var;
	var.type = VARIANT_NUMBER;
	var.number = target->getID();
	return internalCastSpell(creature, var);
}

bool RuneSpell::internalCastSpell(Creature* creature, const LuaVariant& var)
{
	return isScripted() ? executeCastSpell(creature, var) : false;
}

bool RuneSpell::executeCastSpell(Creature* creature, const LuaVariant& var)
{
	//onCastSpell(cid, var)
	if(m_interface->reserveEnv())
	{
		ScriptEnviroment* env = m_interface->getEnv();
		if(m_scripted == EVENT_SCRIPT_BUFFER)
		{
			env->setRealPos(creature->getPosition());
			std::ostringstream scriptstream;

			scriptstream << "local cid = " << env->addThing(creature) << std::endl;
			env->streamVariant(scriptstream, "var", var);

			scriptstream << *m_scriptData;
			bool result = true;
			if(m_interface->loadBuffer(scriptstream.str()))
			{
				lua_State* L = m_interface->getState();
				result = m_interface->getGlobalBool(L, "_result", true);
			}

			m_interface->releaseEnv();
			return result;
		}
		else
		{
			#ifdef __DEBUG_LUASCRIPTS__
			char desc[60];
			sprintf(desc, "onCastSpell - %s", creature->getName().c_str());
			env->setEvent(desc);
			#endif

			env->setScriptId(m_scriptId, m_interface);
			env->setRealPos(creature->getPosition());
			lua_State* L = m_interface->getState();

			m_interface->pushFunction(m_scriptId);
			lua_pushnumber(L, env->addThing(creature));
			m_interface->pushVariant(L, var);

			bool result = m_interface->callFunction(2);
			m_interface->releaseEnv();
			return result;
		}
	}
	else
	{
		std::clog << "[Error - RuneSpell::executeCastSpell] Call stack overflow." << std::endl;
		return false;
	}
}
