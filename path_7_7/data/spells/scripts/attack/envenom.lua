local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_EARTHDAMAGE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_GREEN_RINGS)
setCombatParam(combat, COMBAT_PARAM_DISTANCEEFFECT, CONST_ANI_POISON)

local condition = createConditionObject(CONDITION_POISON)
setConditionParam(condition, CONDITION_PARAM_DELAYED, 1)
--[[
addDamageCondition(condition, 3, 2000, -25)
addDamageCondition(condition, 3, 3000, -5)
addDamageCondition(condition, 4, 4000, -4)
addDamageCondition(condition, 6, 6000, -3)
addDamageCondition(condition, 9, 8000, -2)
addDamageCondition(condition, 12, 10000, -1)
]]--
setConditionParam(condition, CONDITION_PARAM_MINVALUE, 20)
setConditionParam(condition, CONDITION_PARAM_MAXVALUE, 70)
setConditionParam(condition, CONDITION_PARAM_STARTVALUE, 5)
setConditionParam(condition, CONDITION_PARAM_TICKINTERVAL, 6000)
setConditionParam(condition, CONDITION_PARAM_FORCEUPDATE, true)

setCombatCondition(combat, condition)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end