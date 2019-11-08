local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_NONE)
setCombatParam(combat, COMBAT_PARAM_AGGRESSIVE, 0)

local condition = createConditionObject(CONDITION_INVISIBLE)
setConditionParam(condition, CONDITION_PARAM_TICKS, 10000)

function onCastSpell(cid, var)
local creature = Creature(cid)
local hp, cpos = (getCreatureHealth(cid)/getCreatureMaxHealth(cid))*100, creature:getPosition()

if isCreature(cid) == true and getCreatureName(cid) == "Zavarash" and (hp < 100) then
doSetItemOutfit(cid, 1548, -1)
doAddCondition(cid, condition)
creature:setHiddenHealth(creature)
end
if getTileItemById(cpos,1490).uid ~= 0 or getTileItemById(cpos,1496).uid ~= 0 or getTileItemById(cpos,1503).uid ~= 0 then
doSetCreatureOutfit(cid, {lookType=12,lookHead=0,lookAddons=0,lookLegs=57,lookBody=15,lookFeet=85}, -1)
doRemoveCondition(cid, CONDITION_INVISIBLE)
creature:setHiddenHealth(false)
return false
end
	return doCombat(cid, combat, var)
end