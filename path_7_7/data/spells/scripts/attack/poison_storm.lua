local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_EARTHDAMAGE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_GREEN_RINGS)

local area = createCombatArea(AREA_CROSS6X6)
setCombatArea(combat, area)

local condition = createConditionObject(CONDITION_POISON)
setConditionParam(condition, CONDITION_PARAM_DELAYED, 1)
addDamageCondition(condition, 2, 4000, -45)
addDamageCondition(condition, 2, 4000, -40)
addDamageCondition(condition, 2, 4000, -35)
addDamageCondition(condition, 2, 4000, -30)
addDamageCondition(condition, 3, 5000, -20)
addDamageCondition(condition, 3, 5000, -10)
addDamageCondition(condition, 3, 5000, -7)
addDamageCondition(condition, 3, 5000, -5)
addDamageCondition(condition, 4, 5000, -4)
addDamageCondition(condition, 6, 5000, -3)
addDamageCondition(condition, 9, 5000, -2)
addDamageCondition(condition, 12, 5000, -1)
setCombatCondition(combat, condition)

function onGetFormulaValues(cid, level, maglevel)
	min = -((level * 2) + (maglevel * 3)) * 0.9
	max = -((level * 2) + (maglevel * 3)) * 1.5
	return min, max
end

setCombatCallback(combat, CALLBACK_PARAM_LEVELMAGICVALUE, "onGetFormulaValues")

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end