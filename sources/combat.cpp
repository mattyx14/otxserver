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

#include "combat.h"
#include "tools.h"

#include "game.h"
#include "configmanager.h"

#include "creature.h"
#include "player.h"
#include "weapons.h"
#include "tile.h"

extern Game g_game;
extern Weapons* g_weapons;
extern ConfigManager g_config;

Combat::Combat()
{
	params.valueCallback = NULL;
	params.tileCallback = NULL;
	params.targetCallback = NULL;
	area = NULL;

	formulaType = FORMULA_UNDEFINED;
	mina = minb = maxa = maxb = minl = maxl = minm = maxm = minc = maxc = 0;
}

Combat::~Combat()
{
	for(std::list<const Condition*>::iterator it = params.conditionList.begin(); it != params.conditionList.end(); ++it)
		delete (*it);

	params.conditionList.clear();
	delete area;

	delete params.valueCallback;
	delete params.tileCallback;
	delete params.targetCallback;
}

bool Combat::getMinMaxValues(Creature* creature, Creature* target, CombatParams& _params, int32_t& min, int32_t& max) const
{
	if(creature)
	{
		if(creature->getCombatValues(min, max))
			return true;

		if(Player* player = creature->getPlayer())
		{
			if(params.valueCallback)
			{
				params.valueCallback->getMinMaxValues(player, _params, min, max);
				return true;
			}

			min = max = 0;
			switch(formulaType)
			{
				case FORMULA_LEVELMAGIC:
				{
					min = (int32_t)((player->getLevel() / minl + player->getMagicLevel() * minm) * 1. * mina + minb);
					max = (int32_t)((player->getLevel() / maxl + player->getMagicLevel() * maxm) * 1. * maxa + maxb);
					if(minc && std::abs(min) < std::abs(minc))
						min = minc;

					if(maxc && std::abs(max) < std::abs(maxc))
						max = maxc;

					player->increaseCombatValues(min, max, params.useCharges, true);
					return true;
				}

				case FORMULA_SKILL:
				{
					bool crit = false;
					Item* item = player->getWeapon(false);
					if(const Weapon* weapon = g_weapons->getWeapon(item))
					{
						_params.element.type = item->getElementType();
						if(_params.element.type != COMBAT_NONE)
						{
							_params.element.damage = weapon->getWeaponElementDamage(player, item, true);
							_params.element.damage = random_range((int32_t)0, (int32_t)(_params.element.damage * maxa + maxb), DISTRO_NORMAL);
						}

						max = (int32_t)(weapon->getWeaponDamage(player, target, item, crit, true) * maxa + maxb);
						if(params.useCharges && item->hasCharges() && g_config.getBool(ConfigManager::REMOVE_WEAPON_CHARGES))
							g_game.transformItem(item, item->getID(), std::max((int32_t)0, ((int32_t)item->getCharges()) - 1));
					}
					else
						max = (int32_t)maxb;

					if(crit)
						min = max;
					else
						min = (int32_t)minb;

					if(maxc && std::abs(max) < std::abs(maxc))
						max = maxc;

					return true;
				}

				case FORMULA_VALUE:
				{
					min = (int32_t)minb;
					max = (int32_t)maxb;
					return true;
				}

				default:
					break;
			}

			return false;
		}
	}

	if(formulaType != FORMULA_VALUE)
		return false;

	min = (int32_t)mina;
	max = (int32_t)maxa;
	return true;
}

void Combat::getCombatArea(const Position& centerPos, const Position& targetPos, const CombatArea* area, std::list<Tile*>& list)
{
	if(area)
		area->getList(centerPos, targetPos, list);
	else if(targetPos.z < MAP_MAX_LAYERS)
	{
		Tile* tile = g_game.getTile(targetPos);
		if(!tile)
		{
			tile = new StaticTile(targetPos.x, targetPos.y, targetPos.z);
			g_game.setTile(tile);
		}

		list.push_back(tile);
	}
}

CombatType_t Combat::ConditionToDamageType(ConditionType_t type)
{
	switch(type)
	{
		case CONDITION_FIRE:
			return COMBAT_FIREDAMAGE;

		case CONDITION_ENERGY:
			return COMBAT_ENERGYDAMAGE;

		case CONDITION_POISON:
			return COMBAT_EARTHDAMAGE;

		case CONDITION_FREEZING:
			return COMBAT_ICEDAMAGE;

		case CONDITION_DAZZLED:
			return COMBAT_HOLYDAMAGE;

		case CONDITION_CURSED:
			return COMBAT_DEATHDAMAGE;

		case CONDITION_DROWN:
			return COMBAT_DROWNDAMAGE;

		case CONDITION_BLEEDING:
			return COMBAT_PHYSICALDAMAGE;

		default:
			break;
	}

	return COMBAT_NONE;
}

ConditionType_t Combat::DamageToConditionType(CombatType_t type)
{
	switch(type)
	{
		case COMBAT_FIREDAMAGE:
			return CONDITION_FIRE;

		case COMBAT_ENERGYDAMAGE:
			return CONDITION_ENERGY;

		case COMBAT_EARTHDAMAGE:
			return CONDITION_POISON;

		case COMBAT_ICEDAMAGE:
			return CONDITION_FREEZING;

		case COMBAT_HOLYDAMAGE:
			return CONDITION_DAZZLED;

		case COMBAT_DEATHDAMAGE:
			return CONDITION_CURSED;

		case COMBAT_PHYSICALDAMAGE:
			return CONDITION_BLEEDING;

		default:
			break;
	}

	return CONDITION_NONE;
}

ReturnValue Combat::canDoCombat(const Creature* caster, const Tile* tile, bool isAggressive, bool createItem)
{
	if(tile->hasProperty(BLOCKPROJECTILE) || tile->floorChange() || tile->getTeleportItem())
		return RET_NOTENOUGHROOM;

	if(caster)
	{
		bool success = true;
		CreatureEventList combatAreaEvents = const_cast<Creature*>(caster)->getCreatureEvents(CREATURE_EVENT_COMBAT_AREA);
		for(CreatureEventList::iterator it = combatAreaEvents.begin(); it != combatAreaEvents.end(); ++it)
		{
			if(!(*it)->executeCombatArea(const_cast<Creature*>(caster), const_cast<Tile*>(tile), isAggressive) && success)
				success = false;
		}

		if(!success)
			return RET_NOTPOSSIBLE;

		if(caster->getPosition().z < tile->getPosition().z)
			return RET_FIRSTGODOWNSTAIRS;

		if(caster->getPosition().z > tile->getPosition().z)
			return RET_FIRSTGOUPSTAIRS;

		if(!isAggressive)
			return RET_NOERROR;

		const Player* player = caster->getPlayer();
		if(player && player->hasFlag(PlayerFlag_IgnoreProtectionZone))
			return RET_NOERROR;
	}

	return isAggressive && tile->hasFlag(TILESTATE_PROTECTIONZONE) ?
		RET_ACTIONNOTPERMITTEDINPROTECTIONZONE : RET_NOERROR;
}

ReturnValue Combat::canDoCombat(const Creature* attacker, const Creature* target, bool isAggressive)
{
	if(!attacker)
		return RET_NOERROR;

	bool success = true;
	CreatureEventList combatEvents = const_cast<Creature*>(attacker)->getCreatureEvents(CREATURE_EVENT_COMBAT);
	for(CreatureEventList::iterator it = combatEvents.begin(); it != combatEvents.end(); ++it)
	{
		if(!(*it)->executeCombat(const_cast<Creature*>(attacker), const_cast<Creature*>(target), isAggressive) && success)
			success = false;
	}

	if(!success)
		return RET_NOTPOSSIBLE;
	
	if(g_config.getBool(ConfigManager::MONSTER_ATTACK_MONSTER))
	{
		if(target->getType() == CREATURETYPE_MONSTER && attacker->getType() == CREATURETYPE_MONSTER && !target->isPlayerSummon() && !attacker->isPlayerSummon())
			return RET_NOTPOSSIBLE;

		if(!attacker->isSummon() && !target->isSummon()) {
			if(attacker->getType() == CREATURETYPE_MONSTER && target->getType() == CREATURETYPE_MONSTER)
				return RET_NOTPOSSIBLE;
		}
	}
	
	if (attacker->isSummon())
	{
		if (target == attacker->getMaster())
		{
			return RET_NOTPOSSIBLE;
		}
	}

	bool checkZones = false;
	if(const Player* targetPlayer = target->getPlayer())
	{
		if(!targetPlayer->isAttackable())
			return RET_YOUMAYNOTATTACKTHISPLAYER;

		const Player* attackerPlayer = NULL;
		if((attackerPlayer = attacker->getPlayer()) || (attackerPlayer = attacker->getPlayerMaster()))
		{
			checkZones = true;
			if((g_game.getWorldType(attackerPlayer, targetPlayer) == WORLDTYPE_OPTIONAL && !Combat::isInPvpZone(attacker, target)
				&& !attackerPlayer->isEnemy(targetPlayer, true)) || isProtected(const_cast<Player*>(attackerPlayer),
				const_cast<Player*>(targetPlayer)) || (g_config.getBool(ConfigManager::CANNOT_ATTACK_SAME_LOOKFEET)
				&& attackerPlayer->getDefaultOutfit().lookFeet == targetPlayer->getDefaultOutfit().lookFeet)
				|| !attackerPlayer->canSeeCreature(targetPlayer))
				return RET_YOUMAYNOTATTACKTHISPLAYER;
		}
	}
	else if(target->getMonster())
	{
		if(!target->isAttackable())
			return RET_YOUMAYNOTATTACKTHISCREATURE;

		const Player* attackerPlayer = NULL;
		if((attackerPlayer = attacker->getPlayer()) || (attackerPlayer = attacker->getPlayerMaster()))
		{
			if(attackerPlayer->hasFlag(PlayerFlag_CannotAttackMonster))
				return RET_YOUMAYNOTATTACKTHISCREATURE;

			if(target->isPlayerSummon())
			{
				checkZones = true;
				if(g_game.getWorldType(attackerPlayer, target->getPlayerMaster()) == WORLDTYPE_OPTIONAL &&
					!Combat::isInPvpZone(attacker, target) && !attackerPlayer->isEnemy(target->getPlayerMaster(), true))
					return RET_YOUMAYNOTATTACKTHISCREATURE;
			}
		}
	}

	return checkZones && (target->getTile()->hasFlag(TILESTATE_OPTIONALZONE) ||
		(attacker->getTile()->hasFlag(TILESTATE_OPTIONALZONE)
		&& !target->getTile()->hasFlag(TILESTATE_OPTIONALZONE) &&
		!target->getTile()->hasFlag(TILESTATE_PROTECTIONZONE))) ?
		RET_ACTIONNOTPERMITTEDINANOPVPZONE : RET_NOERROR;
}

ReturnValue Combat::canTargetCreature(const Player* player, const Creature* target)
{
	if(player == target)
		return RET_YOUMAYNOTATTACKTHISPLAYER;

	Player* tmpPlayer = const_cast<Player*>(player);
	bool deny = false;

	CreatureEventList targetEvents = tmpPlayer->getCreatureEvents(CREATURE_EVENT_TARGET);
	for(CreatureEventList::iterator it = targetEvents.begin(); it != targetEvents.end(); ++it)
	{
		if(!(*it)->executeAction(tmpPlayer, const_cast<Creature*>(target)) && !deny)
			deny = true;
	}

	if(deny)
		return RET_NEEDEXCHANGE;

	if(!player->hasFlag(PlayerFlag_IgnoreProtectionZone))
	{
		if(player->getZone() == ZONE_PROTECTION)
			return RET_YOUMAYNOTATTACKAPERSONWHILEINPROTECTIONZONE;

		if(target->getZone() == ZONE_PROTECTION)
			return RET_YOUMAYNOTATTACKAPERSONINPROTECTIONZONE;

		if(target->getPlayer() || target->isPlayerSummon())
		{
			if(player->getZone() == ZONE_OPTIONAL)
				return RET_ACTIONNOTPERMITTEDINANOPVPZONE;

			if(target->getZone() == ZONE_OPTIONAL)
				return RET_YOUMAYNOTATTACKAPERSONINPROTECTIONZONE;
		}
	}

	if(player->hasFlag(PlayerFlag_CannotUseCombat))
		return target->getPlayer() ? RET_YOUMAYNOTATTACKTHISPLAYER : RET_YOUMAYNOTATTACKTHISCREATURE;

	if(target->getPlayer() && !Combat::isInPvpZone(player, target) && player->getSkullType(target->getPlayer()) == SKULL_NONE)
	{
		if(player->getSecureMode() == SECUREMODE_ON)
			return RET_TURNSECUREMODETOATTACKUNMARKEDPLAYERS;

		if(g_config.getBool(ConfigManager::USE_BLACK_SKULL))
		{
			if(player->getSkull() == SKULL_BLACK)
				return RET_YOUMAYNOTATTACKTHISPLAYER;
		}
	}
	
	if (target->getNpc())
		return RET_YOUMAYNOTATTACKTHISCREATURE;

	return Combat::canDoCombat(player, target, true);
}

bool Combat::isInPvpZone(const Creature* attacker, const Creature* target)
{
	return attacker->getZone() == ZONE_HARDCORE && target->getZone() == ZONE_HARDCORE;
}

bool Combat::isProtected(Player* attacker, Player* target)
{
	if(attacker->hasFlag(PlayerFlag_CannotAttackPlayer))
		return true;

	if(attacker->hasCustomFlag(PlayerCustomFlag_GamemasterPrivileges))
		return false;

	if(attacker->getZone() == ZONE_HARDCORE && target->getZone() == ZONE_HARDCORE && g_config.getBool(ConfigManager::PVP_TILE_IGNORE_PROTECTION))
		return false;
	
	if (target->checkLoginDelay())
		return true;

	return target->isProtected() || attacker->isProtected() || (attacker->checkLoginDelay() && !attacker->hasBeenAttacked(target->getID()));
}

void Combat::setPlayerCombatValues(formulaType_t _type, double _mina, double _minb, double _maxa, double _maxb, double _minl, double _maxl, double _minm, double _maxm, int32_t _minc, int32_t _maxc)
{
	formulaType = _type; mina = _mina; minb = _minb; maxa = _maxa; maxb = _maxb;
	minl = _minl; maxl = _maxl; minm = _minm; maxm = _maxm; minc = _minc; maxc = _maxc;
}

bool Combat::setParam(CombatParam_t param, uint32_t value)
{
	switch(param)
	{
		case COMBATPARAM_COMBATTYPE:
			params.combatType = (CombatType_t)value;
			return true;

		case COMBATPARAM_EFFECT:
			params.effects.impact = (MagicEffect_t)value;
			return true;

		case COMBATPARAM_DISTANCEEFFECT:
			params.effects.distance = (ShootEffect_t)value;
			return true;

		case COMBATPARAM_BLOCKEDBYARMOR:
			params.blockedByArmor = (value != 0);
			return true;

		case COMBATPARAM_BLOCKEDBYSHIELD:
			params.blockedByShield = (value != 0);
			return true;

		case COMBATPARAM_TARGETCASTERORTOPMOST:
			params.targetCasterOrTopMost = (value != 0);
			return true;

		case COMBATPARAM_TARGETPLAYERSORSUMMONS:
			params.targetPlayersOrSummons = (value != 0);
			return true;

		case COMBATPARAM_DIFFERENTAREADAMAGE:
			params.differentAreaDamage = (value != 0);
			return true;

		case COMBATPARAM_CREATEITEM:
			params.itemId = value;
			return true;

		case COMBATPARAM_AGGRESSIVE:
			params.isAggressive = (value != 0);
			return true;

		case COMBATPARAM_DISPEL:
			params.dispelType = (ConditionType_t)value;
			return true;

		case COMBATPARAM_USECHARGES:
			params.useCharges = (value != 0);
			return true;

		case COMBATPARAM_HITEFFECT:
			params.effects.hit = (MagicEffect_t)value;
			return true;

		case COMBATPARAM_HITCOLOR:
			params.effects.color = (Color_t)value;
			return true;

		case COMBATPARAM_ELEMENTDAMAGE:
			params.element.damage = value;
			break;

		case COMBATPARAM_ELEMENTTYPE:
			params.element.type = (CombatType_t)value;
			break;

		default:
			break;
	}

	return false;
}

bool Combat::setCallback(CallBackParam_t key)
{
	switch(key)
	{
		case CALLBACKPARAM_LEVELMAGICVALUE:
		{
			delete params.valueCallback;
			params.valueCallback = new ValueCallback(FORMULA_LEVELMAGIC);
			return true;
		}

		case CALLBACKPARAM_SKILLVALUE:
		{
			delete params.valueCallback;
			params.valueCallback = new ValueCallback(FORMULA_SKILL);
			return true;
		}

		case CALLBACKPARAM_TARGETTILECALLBACK:
		{
			delete params.tileCallback;
			params.tileCallback = new TileCallback();
			break;
		}

		case CALLBACKPARAM_TARGETCREATURECALLBACK:
		{
			delete params.targetCallback;
			params.targetCallback = new TargetCallback();
			break;
		}

		default:
			std::clog << "Combat::setCallback - Unknown callback type: " << (uint32_t)key << std::endl;
			break;
	}

	return false;
}

CallBack* Combat::getCallback(CallBackParam_t key)
{
	switch(key)
	{
		case CALLBACKPARAM_LEVELMAGICVALUE:
		case CALLBACKPARAM_SKILLVALUE:
			return params.valueCallback;

		case CALLBACKPARAM_TARGETTILECALLBACK:
			return params.tileCallback;

		case CALLBACKPARAM_TARGETCREATURECALLBACK:
			return params.targetCallback;

		default:
			break;
	}

	return NULL;
}

bool Combat::CombatHealthFunc(Creature* caster, Creature* target, const CombatParams& params, void* data)
{
	int32_t change = 0;
	if(Combat2Var* var = (Combat2Var*)data)
	{
		change = var->change;
		if(!change)
			change = random_range(var->minChange, var->maxChange, DISTRO_NORMAL);
	}

	if(g_game.combatBlockHit(params.combatType, caster, target, change, params.blockedByShield, params.blockedByArmor, params.itemId != 0))
		return false;

	CombatParams _params = params;
	if(_params.element.damage && _params.element.type != COMBAT_NONE)
		g_game.combatBlockHit(_params.element.type, caster, target, _params.element.damage, params.blockedByShield, params.blockedByArmor, params.itemId != 0, true);

	if(g_config.getBool(ConfigManager::USE_BLACK_SKULL))
	{
		if(caster && caster->getPlayer() && target->getPlayer() && target->getSkull() != SKULL_BLACK)
		{
			_params.element.damage /= 2;
			if(change < 0)
				change /= 2;
		}
	}
	else
	{
		if(caster && caster->getPlayer() && target->getPlayer())
		{
			_params.element.damage /= 2;
			if(change < 0)
				change /= 2;
		}
	}

	if(!g_game.combatChangeHealth(_params, caster, target, change, false))
		return false;

	CombatConditionFunc(caster, target, params, NULL);
	CombatDispelFunc(caster, target, params, NULL);
	return true;
}

bool Combat::CombatManaFunc(Creature* caster, Creature* target, const CombatParams& params, void* data)
{
	int32_t change = 0;
	if(Combat2Var* var = (Combat2Var*)data)
	{
		change = var->change;
		if(!change)
			change = random_range(var->minChange, var->maxChange, DISTRO_NORMAL);
	}

	if(g_game.combatBlockHit(COMBAT_MANADRAIN, caster, target, change, false, false, params.itemId != 0))
		return false;

	if(g_config.getBool(ConfigManager::USE_BLACK_SKULL))
	{
		if(change < 0 && caster && caster->getPlayer() && target->getPlayer() && target->getSkull() != SKULL_BLACK)
			change /= 2;
	}
	else
	{
		if(change < 0 && caster && caster->getPlayer() && target->getPlayer())
			change /= 2;
	}

	if(!g_game.combatChangeMana(caster, target, change))
		return false;

	CombatConditionFunc(caster, target, params, NULL);
	CombatDispelFunc(caster, target, params, NULL);
	return true;
}

bool Combat::CombatConditionFunc(Creature* caster, Creature* target, const CombatParams& params, void*)
{
	if(params.conditionList.empty())
		return false;

	bool result = true;
	for(std::list<const Condition*>::const_iterator it = params.conditionList.begin(); it != params.conditionList.end(); ++it)
	{
		if(caster != target && target->isImmune((*it)->getType()))
			continue;

		Condition* tmp = (*it)->clone();
		if(caster)
		{
			tmp->setParam(CONDITIONPARAM_OWNER, caster->getID());
			if(params.isAggressive)
				caster->onTargetDrain(target, 0);
			else
				caster->onTargetGain(target, 0);
		}

		//TODO: infight condition until all aggressive conditions has ended [?]
		if(!target->addCombatCondition(tmp) && result)
			result = false;
	}

	return result;
}

bool Combat::CombatDispelFunc(Creature* caster, Creature* target, const CombatParams& params, void*)
{
	if(!target->hasCondition(params.dispelType, -1, false))
		return false;

	if(params.dispelType == CONDITION_INVISIBLE)
	{
		if(Player* player = target->getPlayer())
		{
			Player* casterPlayer = NULL;
			if(caster)
				casterPlayer = caster->getPlayer();

			Item* item = player->getEquippedItem(SLOT_RING);
			if(item && item->getID() == ITEM_STEALTH_RING && (g_game.getWorldType(casterPlayer, player)
				== WORLDTYPE_HARDCORE || player->getTile()->hasFlag(TILESTATE_HARDCOREZONE)) && random_range(1, 100) <= 10)
				g_game.internalRemoveItem(NULL, item);
		}
	}

	target->removeCondition(caster, params.dispelType);
	return true;
}

bool Combat::CombatNullFunc(Creature* caster, Creature* target, const CombatParams& params, void*)
{
	CombatConditionFunc(caster, target, params, NULL);
	CombatDispelFunc(caster, target, params, NULL);
	return true;
}

void Combat::combatTileEffects(const SpectatorVec& list, Creature* caster, Tile* tile, const CombatParams& params)
{
	if(!tile)
		return;

	if(params.itemId)
	{
		Player* player = NULL;
		if(caster)
		{
			if(caster->getPlayer())
				player = caster->getPlayer();
			else if(caster->isPlayerSummon())
				player = caster->getPlayerMaster();
		}

		uint32_t itemId = params.itemId;
		if(player)
		{
			bool pzLock = false;
			if(((g_game.getWorldType(player) == WORLDTYPE_OPTIONAL || g_config.getBool(ConfigManager::OPTIONAL_PROTECTION))
				&& !tile->hasFlag(TILESTATE_HARDCOREZONE)) || tile->hasFlag(TILESTATE_OPTIONALZONE))
			{
				switch(itemId)
				{
					case ITEM_FIREFIELD:
						itemId = ITEM_FIREFIELD_SAFE;
						break;
					case ITEM_POISONFIELD:
						itemId = ITEM_POISONFIELD_SAFE;
						break;
					case ITEM_ENERGYFIELD:
						itemId = ITEM_ENERGYFIELD_SAFE;
						break;
					case ITEM_MAGICWALL:
						itemId = ITEM_MAGICWALL_SAFE;
						break;
					case ITEM_WILDGROWTH:
						itemId = ITEM_WILDGROWTH_SAFE;
						break;
					default:
						break;
				}
			}
			else if(params.isAggressive && !Item::items[itemId].blockPathFind)
				pzLock = true;

			player->addInFightTicks(pzLock);
		}

		if(Item* item = Item::CreateItem(itemId))
		{
			if(caster)
				item->setOwner(caster->getID());

			if(g_game.internalAddItem(caster, tile, item) == RET_NOERROR)
				g_game.startDecay(item);
			else
				delete item;
		}
	}

	if(params.tileCallback)
		params.tileCallback->onTileCombat(caster, tile);

	if(params.effects.impact != MAGIC_EFFECT_NONE && (!caster || !caster->isGhost()
		|| g_config.getBool(ConfigManager::GHOST_SPELL_EFFECTS)))
		g_game.addMagicEffect(list, tile->getPosition(), params.effects.impact);
}

void Combat::postCombatEffects(Creature* caster, const Position& pos, const CombatParams& params)
{
	if(caster && params.effects.distance != SHOOT_EFFECT_NONE)
		addDistanceEffect(caster, caster->getPosition(), pos, params.effects.distance);
}

void Combat::addDistanceEffect(Creature* caster, const Position& fromPos, const Position& toPos, ShootEffect_t effect)
{
	if(effect == SHOOT_EFFECT_WEAPONTYPE)
	{
		switch(caster->getWeaponType())
		{
			case WEAPON_AXE:
				effect = SHOOT_EFFECT_WHIRLWINDAXE;
				break;

			case WEAPON_SWORD:
				effect = SHOOT_EFFECT_WHIRLWINDSWORD;
				break;

			case WEAPON_CLUB:
				effect = SHOOT_EFFECT_WHIRLWINDCLUB;
				break;

			case WEAPON_FIST:
				effect = SHOOT_EFFECT_LARGEROCK;
				break;

			default:
				effect = SHOOT_EFFECT_NONE;
				break;
		}
	}

	if(caster && effect != SHOOT_EFFECT_NONE)
		g_game.addDistanceEffect(fromPos, toPos, effect);
}

void Combat::CombatFunc(Creature* caster, const Position& pos, const CombatArea* area,
	const CombatParams& params, COMBATFUNC func, void* data)
{
	std::list<Tile*> tileList;
	if(caster)
		getCombatArea(caster->getPosition(), pos, area, tileList);
	else
		getCombatArea(pos, pos, area, tileList);

	Combat2Var* var = (Combat2Var*)data;
	if(var && !params.differentAreaDamage)
		var->change = random_range(var->minChange, var->maxChange, DISTRO_NORMAL);

	uint32_t maxX = 0, maxY = 0, diff;
	//calculate the max viewable range
	for(std::list<Tile*>::iterator it = tileList.begin(); it != tileList.end(); ++it)
	{
		diff = std::abs((*it)->getPosition().x - pos.x);
		if(diff > maxX)
			maxX = diff;

		diff = std::abs((*it)->getPosition().y - pos.y);
		if(diff > maxY)
			maxY = diff;
	}

	SpectatorVec list;
	g_game.getSpectators(list, pos, true, true, maxX + Map::maxViewportX, maxX + Map::maxViewportX,
		maxY + Map::maxViewportY, maxY + Map::maxViewportY);

	Tile* tile = NULL;
	for(std::list<Tile*>::iterator it = tileList.begin(); it != tileList.end(); ++it)
	{
		if(!(tile = (*it)) || canDoCombat(caster, (*it), params.isAggressive, params.itemId != 0) != RET_NOERROR)
			continue;

		bool skip = true;
		if(CreatureVector* creatures = tile->getCreatures())
		{
			for(CreatureVector::iterator cit = creatures->begin(), cend = creatures->end(); skip && cit != cend; ++cit)
			{
				if(params.targetPlayersOrSummons && !(*cit)->getPlayer() && !(*cit)->isPlayerSummon())
					continue;

				if(params.targetCasterOrTopMost)
				{
					if(caster && caster->getTile() == tile)
					{
						if((*cit) == caster)
							skip = false;
					}
					else if((*cit) == tile->getTopCreature())
						skip = false;

					if(skip)
						continue;
				}

				if(!params.isAggressive || (caster != (*cit) && Combat::canDoCombat(caster, (*cit), true) == RET_NOERROR))
				{
					func(caster, (*cit), params, (void*)var);
					if(params.targetCallback)
						params.targetCallback->onTargetCombat(caster, (*cit));
				}
			}
		}

		combatTileEffects(list, caster, tile, params);
	}

	postCombatEffects(caster, pos, params);
}

void Combat::doCombat(Creature* caster, Creature* target) const
{
	//target combat callback function
	if(params.combatType != COMBAT_NONE)
	{
		if(params.isAggressive && (caster == target || Combat::canDoCombat(caster, target, true) != RET_NOERROR))
			return;

		int32_t minChange = 0, maxChange = 0;
		CombatParams _params = params;

		getMinMaxValues(caster, target, _params, minChange, maxChange);
		if(params.combatType != COMBAT_MANADRAIN)
			doCombatHealth(caster, target, minChange, maxChange, _params, false);
		else
			doCombatMana(caster, target, minChange, maxChange, _params, false);
	}
	else
		doCombatDefault(caster, target, params);
}

void Combat::doCombat(Creature* caster, const Position& pos) const
{
	//area combat callback function
	if(params.combatType != COMBAT_NONE)
	{
		int32_t minChange = 0, maxChange = 0;
		CombatParams _params = params;

		getMinMaxValues(caster, NULL, _params, minChange, maxChange);
		if(params.combatType != COMBAT_MANADRAIN)
			doCombatHealth(caster, pos, area, minChange, maxChange, _params);
		else
			doCombatMana(caster, pos, area, minChange, maxChange, _params);
	}
	else
		CombatFunc(caster, pos, area, params, CombatNullFunc, NULL);
}

void Combat::doCombatHealth(Creature* caster, Creature* target, int32_t minChange, int32_t maxChange, const CombatParams& params, bool check/* = true*/)
{
	if(check && params.isAggressive && (caster == target || Combat::canDoCombat(caster, target, true) != RET_NOERROR))
		return;

	Combat2Var var;
	var.minChange = minChange;
	var.maxChange = maxChange;

	CombatHealthFunc(caster, target, params, (void*)&var);
	if(params.targetCallback)
		params.targetCallback->onTargetCombat(caster, target);

	if(params.effects.impact != MAGIC_EFFECT_NONE && (!caster || !caster->isGhost()
		|| g_config.getBool(ConfigManager::GHOST_SPELL_EFFECTS)))
		g_game.addMagicEffect(target->getPosition(), params.effects.impact);

	if(caster && params.effects.distance != SHOOT_EFFECT_NONE)
		addDistanceEffect(caster, caster->getPosition(), target->getPosition(), params.effects.distance);
}

void Combat::doCombatHealth(Creature* caster, const Position& pos, const CombatArea* area,
	int32_t minChange, int32_t maxChange, const CombatParams& params)
{
	Combat2Var var;
	var.minChange = minChange;
	var.maxChange = maxChange;
	CombatFunc(caster, pos, area, params, CombatHealthFunc, (void*)&var);
}

void Combat::doCombatMana(Creature* caster, Creature* target, int32_t minChange, int32_t maxChange, const CombatParams& params, bool check/* = true*/)
{
	if(check && params.isAggressive && (caster == target || Combat::canDoCombat(caster, target, true) != RET_NOERROR))
		return;

	Combat2Var var;
	var.minChange = minChange;
	var.maxChange = maxChange;

	CombatManaFunc(caster, target, params, (void*)&var);
	if(params.targetCallback)
		params.targetCallback->onTargetCombat(caster, target);

	if(params.effects.impact != MAGIC_EFFECT_NONE && (!caster || !caster->isGhost()
		|| g_config.getBool(ConfigManager::GHOST_SPELL_EFFECTS)))
		g_game.addMagicEffect(target->getPosition(), params.effects.impact);

	if(caster && params.effects.distance != SHOOT_EFFECT_NONE)
		addDistanceEffect(caster, caster->getPosition(), target->getPosition(), params.effects.distance);
}

void Combat::doCombatMana(Creature* caster, const Position& pos, const CombatArea* area,
	int32_t minChange, int32_t maxChange, const CombatParams& params)
{
	Combat2Var var;
	var.minChange = minChange;
	var.maxChange = maxChange;
	CombatFunc(caster, pos, area, params, CombatManaFunc, (void*)&var);
}

void Combat::doCombatCondition(Creature* caster, const Position& pos, const CombatArea* area,
	const CombatParams& params)
{
	CombatFunc(caster, pos, area, params, CombatConditionFunc, NULL);
}

void Combat::doCombatCondition(Creature* caster, Creature* target, const CombatParams& params, bool check/* = true*/)
{
	if(check && params.isAggressive && (caster == target || Combat::canDoCombat(caster, target, true) != RET_NOERROR))
		return;

	CombatConditionFunc(caster, target, params, NULL);
	if(params.targetCallback)
		params.targetCallback->onTargetCombat(caster, target);

	if(params.effects.impact != MAGIC_EFFECT_NONE && (!caster || !caster->isGhost()
		|| g_config.getBool(ConfigManager::GHOST_SPELL_EFFECTS)))
		g_game.addMagicEffect(target->getPosition(), params.effects.impact);

	if(caster && params.effects.distance != SHOOT_EFFECT_NONE)
		addDistanceEffect(caster, caster->getPosition(), target->getPosition(), params.effects.distance);
}

void Combat::doCombatDispel(Creature* caster, const Position& pos, const CombatArea* area,
	const CombatParams& params)
{
	CombatFunc(caster, pos, area, params, CombatDispelFunc, NULL);
}

void Combat::doCombatDispel(Creature* caster, Creature* target, const CombatParams& params, bool check/* = true*/)
{
	if(check && params.isAggressive && (caster == target || Combat::canDoCombat(caster, target, true) != RET_NOERROR))
		return;

	CombatDispelFunc(caster, target, params, NULL);
	if(params.targetCallback)
		params.targetCallback->onTargetCombat(caster, target);

	if(params.effects.impact != MAGIC_EFFECT_NONE && (!caster || !caster->isGhost()
		|| g_config.getBool(ConfigManager::GHOST_SPELL_EFFECTS)))
		g_game.addMagicEffect(target->getPosition(), params.effects.impact);

	if(caster && params.effects.distance != SHOOT_EFFECT_NONE)
		addDistanceEffect(caster, caster->getPosition(), target->getPosition(), params.effects.distance);
}

void Combat::doCombatDefault(Creature* caster, Creature* target, const CombatParams& params)
{
	if(params.isAggressive && (caster == target || Combat::canDoCombat(caster, target, true) != RET_NOERROR))
		return;

	SpectatorVec list;
	g_game.getSpectators(list, target->getPosition(), true, true);

	CombatNullFunc(caster, target, params, NULL);

	combatTileEffects(list, caster, target->getTile(), params);
	if(params.targetCallback)
		params.targetCallback->onTargetCombat(caster, target);

	if(params.effects.impact != MAGIC_EFFECT_NONE && (!caster || !caster->isGhost()
		|| g_config.getBool(ConfigManager::GHOST_SPELL_EFFECTS)))
		g_game.addMagicEffect(target->getPosition(), params.effects.impact);

	if(caster && params.effects.distance != SHOOT_EFFECT_NONE)
		addDistanceEffect(caster, caster->getPosition(), target->getPosition(), params.effects.distance);
}

//**********************************************************

void ValueCallback::getMinMaxValues(Player* player, CombatParams& params, int32_t& min, int32_t& max) const
{
	//"onGetPlayerMinMaxValues"(cid, ...)
	if(!m_interface->reserveEnv())
	{
		std::clog << "[Error - ValueCallback::getMinMaxValues] Callstack overflow." << std::endl;
		return;
	}

	ScriptEnviroment* env = m_interface->getEnv();
	if(!env->setCallbackId(m_scriptId, m_interface))
		return;

	m_interface->pushFunction(m_scriptId);
	lua_State* L = m_interface->getState();
	lua_pushnumber(L, env->addThing(player));

	int32_t parameters = 1;
	switch(type)
	{
		case FORMULA_LEVELMAGIC:
		{
			//"onGetPlayerMinMaxValues"(cid, level, magLevel)
			lua_pushnumber(L, player->getLevel());
			lua_pushnumber(L, player->getMagicLevel());

			parameters += 2;
			break;
		}

		case FORMULA_SKILL:
		{
			//"onGetPlayerMinMaxValues"(cid, level, skill, attack, element, factor)
			lua_pushnumber(L, player->getLevel());
			if(Item* weapon = player->getWeapon(false))
			{
				lua_pushnumber(L, player->getWeaponSkill(weapon));
				if(params.useCharges && weapon->hasCharges() && g_config.getBool(ConfigManager::REMOVE_WEAPON_CHARGES))
					g_game.transformItem(weapon, weapon->getID(), std::max(0, weapon->getCharges() - 1));

				uint32_t attack = weapon->getAttack() + weapon->getExtraAttack();
				if(weapon->getWeaponType() == WEAPON_AMMO)
				{
					if(Item* bow = player->getWeapon(true))
						attack += bow->getAttack() + bow->getExtraAttack();
				}

				if(weapon->getElementType() != COMBAT_NONE)
				{
					attack -= weapon->getElementDamage();
					lua_pushnumber(L, attack);

					lua_pushnumber(L, weapon->getElementDamage());
					params.element.type = weapon->getElementType();
				}
				else
				{
					lua_pushnumber(L, attack);
					lua_pushnumber(L, 0);
				}
			}
			else
			{
				lua_pushnumber(L, player->getSkill(SKILL_FIST, SKILL_LEVEL));
				lua_pushnumber(L, g_config.getNumber(ConfigManager::FIST_BASE_ATTACK));
				lua_pushnumber(L, 0);
			}

			lua_pushnumber(L, player->getAttackFactor());
			parameters += 5;
			break;
		}

		default:
		{
			std::clog << "[Warning - ValueCallback::getMinMaxValues] Unknown callback type" << std::endl;
			return;
		}
	}

	int32_t args = lua_gettop(L);
	if(!lua_pcall(L, parameters, 3, 0))
	{
		params.element.damage = LuaInterface::popNumber(L);
		max = LuaInterface::popNumber(L);
		min = LuaInterface::popNumber(L);
		player->increaseCombatValues(min, max, params.useCharges, type != FORMULA_SKILL);
	}
	else
		LuaInterface::error(NULL, std::string(LuaInterface::popString(L)));

	if((lua_gettop(L) + parameters + 1) != args)
		LuaInterface::error(__FUNCTION__, "Stack size changed!");

	env->resetCallback();
	m_interface->releaseEnv();
}

//**********************************************************

void TileCallback::onTileCombat(Creature* creature, Tile* tile) const
{
	//"onTileCombat"(cid, pos)
	if(m_interface->reserveEnv())
	{
		ScriptEnviroment* env = m_interface->getEnv();
		if(!env->setCallbackId(m_scriptId, m_interface))
			return;

		m_interface->pushFunction(m_scriptId);
		lua_State* L = m_interface->getState();

		lua_pushnumber(L, creature ? env->addThing(creature) : 0);
		m_interface->pushPosition(L, tile->getPosition(), 0);

		m_interface->callFunction(2);
		env->resetCallback();
		m_interface->releaseEnv();
	}
	else
		std::clog << "[Error - TileCallback::onTileCombat] Call stack overflow." << std::endl;
}

//**********************************************************

void TargetCallback::onTargetCombat(Creature* creature, Creature* target) const
{
	//"onTargetCombat"(cid, target)
	if(m_interface->reserveEnv())
	{
		ScriptEnviroment* env = m_interface->getEnv();
		if(!env->setCallbackId(m_scriptId, m_interface))
			return;

		uint32_t cid = 0;
		if(creature)
			cid = env->addThing(creature);

		m_interface->pushFunction(m_scriptId);
		lua_State* L = m_interface->getState();

		lua_pushnumber(L, cid);
		lua_pushnumber(L, env->addThing(target));

		int32_t size = lua_gettop(L);
		if(lua_pcall(L, 2, 0 /*nReturnValues*/, 0) != 0)
			LuaInterface::error(NULL, std::string(LuaInterface::popString(L)));

		if((lua_gettop(L) + 2 /*nParams*/ + 1) != size)
			LuaInterface::error(__FUNCTION__, "Stack size changed!");

		env->resetCallback();
		m_interface->releaseEnv();
	}
	else
	{
		std::clog << "[Error - TargetCallback::onTargetCombat] Call stack overflow." << std::endl;
		return;
	}
}

//**********************************************************

void CombatArea::clear()
{
	for(CombatAreas::iterator it = areas.begin(); it != areas.end(); ++it)
		delete it->second;

	areas.clear();
}

CombatArea::CombatArea(const CombatArea& rhs)
{
	hasExtArea = rhs.hasExtArea;
	for(CombatAreas::const_iterator it = rhs.areas.begin(); it != rhs.areas.end(); ++it)
		areas[it->first] = new MatrixArea(*it->second);
}

bool CombatArea::getList(const Position& centerPos, const Position& targetPos, std::list<Tile*>& list) const
{
	Tile* tile = g_game.getTile(targetPos);
	if(tile)
	{
		if(tile->hasProperty(BLOCKPROJECTILE))
			return false;
		if(tile->hasFlag(TILESTATE_FLOORCHANGE))
			return false;
		if(tile->getTeleportItem())
			return false;
	}

	const MatrixArea* area = getArea(centerPos, targetPos);
	if(!area)
		return false;

	uint16_t tmpX = targetPos.x, tmpY = targetPos.y, centerY = 0, centerX = 0;
	size_t cols = area->getCols(), rows = area->getRows();
	area->getCenter(centerY, centerX);

	tmpX -= centerX;
	tmpY -= centerY;
	for(size_t y = 0; y < rows; ++y)
	{
		for(size_t x = 0; x < cols; ++x)
		{
			if(area->getValue(y, x) != 0)
			{
				if(targetPos.z < MAP_MAX_LAYERS && g_game.isSightClear(targetPos, Position(tmpX, tmpY, targetPos.z), true))
				{
					if(!(tile = g_game.getTile(tmpX, tmpY, targetPos.z)))
					{
						tile = new StaticTile(tmpX, tmpY, targetPos.z);
						g_game.setTile(tile);
					}

					list.push_back(tile);
				}
			}

			tmpX++;
		}

		tmpX -= cols;
		tmpY++;
	}

	return true;
}

void CombatArea::copyArea(const MatrixArea* input, MatrixArea* output, MatrixOperation_t op) const
{
	uint16_t centerY, centerX;
	input->getCenter(centerY, centerX);
	if(op == MATRIXOPERATION_COPY)
	{
		for(uint32_t y = 0; y < input->getRows(); ++y)
		{
			for(uint32_t x = 0; x < input->getCols(); ++x)
				(*output)[y][x] = (*input)[y][x];
		}

		output->setCenter(centerY, centerX);
	}
	else if(op == MATRIXOPERATION_MIRROR)
	{
		for(uint32_t y = 0; y < input->getRows(); ++y)
		{
			int32_t rx = 0;
			for(int32_t x = input->getCols() - 1; x >= 0; --x)
				(*output)[y][rx++] = (*input)[y][x];
		}

		output->setCenter(centerY, (input->getRows() - 1) - centerX);
	}
	else if(op == MATRIXOPERATION_FLIP)
	{
		for(uint32_t x = 0; x < input->getCols(); ++x)
		{
			int32_t ry = 0;
			for(int32_t y = input->getRows() - 1; y >= 0; --y)
				(*output)[ry++][x] = (*input)[y][x];
		}

		output->setCenter((input->getCols() - 1) - centerY, centerX);
	}
	else //rotation
	{
		uint16_t centerX, centerY;
		input->getCenter(centerY, centerX);

		int32_t rotateCenterX = (output->getCols() / 2) - 1, rotateCenterY = (output->getRows() / 2) - 1, angle = 0;
		switch(op)
		{
			case MATRIXOPERATION_ROTATE90:
				angle = 90;
				break;

			case MATRIXOPERATION_ROTATE180:
				angle = 180;
				break;

			case MATRIXOPERATION_ROTATE270:
				angle = 270;
				break;

			default:
				angle = 0;
				break;
		}

		double _angle = 3.1416 * angle / 180.0;
		float a = std::cos(_angle), b = std::sin(_angle);
		for(int32_t x = 0; x < (int32_t)input->getCols(); ++x)
		{
			for(int32_t y = 0; y < (int32_t)input->getRows(); ++y)
			{
				//calculate new coordinates using rotation center
				int32_t newX = x - centerX, newY = y - centerY,
					rotatedX = round(newX * a + newY * -b),
					rotatedY = round(newX * b + newY * a);
				//write in the output matrix using rotated coordinates
				(*output)[rotatedY + rotateCenterY][rotatedX + rotateCenterX] = (*input)[y][x];
			}
		}

		output->setCenter(rotateCenterY, rotateCenterX);
	}
}

MatrixArea* CombatArea::createArea(const std::list<uint32_t>& list, uint32_t rows)
{
	uint32_t cols = list.size() / rows;
	MatrixArea* area = new MatrixArea(rows, cols);

	uint16_t x = 0, y = 0;
	for(std::list<uint32_t>::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		if(*it == 1 || *it == 3)
			area->setValue(y, x, true);

		if(*it == 2 || *it == 3)
			area->setCenter(y, x);

		++x;
		if(cols != x)
			continue;

		x = 0;
		++y;
	}

	return area;
}

void CombatArea::setupArea(const std::list<uint32_t>& list, uint32_t rows)
{
	//NORTH
	MatrixArea* area = createArea(list, rows);
	areas[NORTH] = area;
	uint32_t maxOutput = std::max(area->getCols(), area->getRows()) << 1;

	//SOUTH
	MatrixArea* southArea = new MatrixArea(maxOutput, maxOutput);
	copyArea(area, southArea, MATRIXOPERATION_ROTATE180);
	areas[SOUTH] = southArea;

	//EAST
	MatrixArea* eastArea = new MatrixArea(maxOutput, maxOutput);
	copyArea(area, eastArea, MATRIXOPERATION_ROTATE90);
	areas[EAST] = eastArea;

	//WEST
	MatrixArea* westArea = new MatrixArea(maxOutput, maxOutput);
	copyArea(area, westArea, MATRIXOPERATION_ROTATE270);
	areas[WEST] = westArea;
}

void CombatArea::setupArea(int32_t length, int32_t spread)
{
	std::list<uint32_t> list;
	uint32_t rows = length;

	int32_t cols = 1;
	if(spread != 0)
		cols = (((length - length % spread) / spread) << 1) + 1;

	int32_t colSpread = cols;
	for(uint32_t y = 1; y <= rows; ++y)
	{
		int32_t mincol = cols - colSpread + 1, maxcol = cols - (cols - colSpread);
		for(int32_t x = 1; x <= cols; ++x)
		{
			if(y == rows && x == ((cols - cols % 2) / 2) + 1)
				list.push_back(3);
			else if(x >= mincol && x <= maxcol)
				list.push_back(1);
			else
				list.push_back(0);
		}

		if(spread > 0 && y % spread == 0)
			--colSpread;
	}

	setupArea(list, rows);
}

void CombatArea::setupArea(int32_t radius)
{
	int32_t area[13][13] =
	{
		{0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 8, 8, 7, 8, 8, 0, 0, 0, 0},
		{0, 0, 0, 8, 7, 6, 6, 6, 7, 8, 0, 0, 0},
		{0, 0, 8, 7, 6, 5, 5, 5, 6, 7, 8, 0, 0},
		{0, 8, 7, 6, 5, 4, 4, 4, 5, 6, 7, 8, 0},
		{0, 8, 6, 5, 4, 3, 2, 3, 4, 5, 6, 8, 0},
		{8, 7, 6, 5, 4, 2, 1, 2, 4, 5, 6, 7, 8},
		{0, 8, 6, 5, 4, 3, 2, 3, 4, 5, 6, 8, 0},
		{0, 8, 7, 6, 5, 4, 4, 4, 5, 6, 7, 8, 0},
		{0, 0, 8, 7, 6, 5, 5, 5, 6, 7, 8, 0, 0},
		{0, 0, 0, 8, 7, 6, 6, 6, 7, 8, 0, 0, 0},
		{0, 0, 0, 0, 8, 8, 7, 8, 8, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0}
	};

	std::list<uint32_t> list;
	for(int32_t y = 0; y < 13; ++y)
	{
		for(int32_t x = 0; x < 13; ++x)
		{
			if(area[y][x] == 1)
				list.push_back(3);
			else if(area[y][x] > 0 && area[y][x] <= radius)
				list.push_back(1);
			else
				list.push_back(0);
		}
	}

	setupArea(list, 13);
}

void CombatArea::setupExtArea(const std::list<uint32_t>& list, uint32_t rows)
{
	if(list.empty())
		return;

	//NORTH-WEST
	MatrixArea* area = createArea(list, rows);
	areas[NORTHWEST] = area;
	uint32_t maxOutput = std::max(area->getCols(), area->getRows()) << 1;

	//NORTH-EAST
	MatrixArea* neArea = new MatrixArea(maxOutput, maxOutput);
	copyArea(area, neArea, MATRIXOPERATION_MIRROR);
	areas[NORTHEAST] = neArea;

	//SOUTH-WEST
	MatrixArea* swArea = new MatrixArea(maxOutput, maxOutput);
	copyArea(area, swArea, MATRIXOPERATION_FLIP);
	areas[SOUTHWEST] = swArea;

	//SOUTH-EAST
	MatrixArea* seArea = new MatrixArea(maxOutput, maxOutput);
	copyArea(swArea, seArea, MATRIXOPERATION_MIRROR);
	areas[SOUTHEAST] = seArea;

	hasExtArea = true;
}

//**********************************************************

bool MagicField::isBlocking(const Creature* creature) const
{
	if(!isUnstepable())
		return Item::isBlocking(creature);

	if(!creature || !creature->getPlayer())
		return true;

	uint32_t ownerId = getOwner();
	if(!ownerId)
		return false;

	if(Creature* owner = g_game.getCreatureByID(ownerId))
		return creature->getPlayer()->getGuildEmblem(owner) != GUILDEMBLEM_NONE;

	return false;
}

void MagicField::onStepInField(Creature* creature)
{
	if(!creature)
		return;

	//remove magic walls/wild growth
	if(isUnstepable() || id == ITEM_MAGICWALL || id == ITEM_WILDGROWTH || id == ITEM_MAGICWALL_SAFE || id == ITEM_WILDGROWTH_SAFE || isBlocking(creature))
	{
		if(!creature->isGhost())
			g_game.internalRemoveItem(creature, this, 1);

		return;
	}

	const ItemType& it = items[id];
	if(!it.condition)
		return;

	uint32_t ownerId = getOwner();
	Tile* tile = getTile();

	Condition* condition = it.condition->clone();
	if(ownerId && !tile->hasFlag(TILESTATE_HARDCOREZONE))
	{
		if(Creature* owner = g_game.getCreatureByID(ownerId))
		{
			Player* ownerPlayer = owner->getPlayer();
			if(!ownerPlayer && owner->isPlayerSummon())
				ownerPlayer = owner->getPlayerMaster();

			bool harmful = true;
			if(ownerPlayer && (tile->hasFlag(TILESTATE_OPTIONALZONE) || g_game.getWorldType(
				ownerPlayer, creature->getPlayer()) == WORLDTYPE_OPTIONAL))
				harmful = false;
			else if(Player* player = creature->getPlayer())
			{
				if(ownerPlayer && Combat::isProtected(ownerPlayer, player))
					harmful = false;
			}

			if(!harmful || (OTSYS_TIME() - createTime) <= (uint32_t)g_config.getNumber(
				ConfigManager::FIELD_OWNERSHIP) || creature->hasBeenAttacked(ownerId))
				condition->setParam(CONDITIONPARAM_OWNER, ownerId);
		}
	}

	creature->addCondition(condition);
}
