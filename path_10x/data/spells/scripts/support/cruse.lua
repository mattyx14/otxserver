local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_DEATHDAMAGE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_MORTAREA)
setCombatParam(combat, COMBAT_PARAM_DISTANCEEFFECT, CONST_ANI_SUDDENDEATH)

function onGetFormulaValues(cid, level, maglevel)
	local min = level / 5 + maglevel * 1.4 + 8
	local max = level / 5 + maglevel * 2.2 + 14
	return -min, -max
local condition = createConditionObject(CONDITION_DEATH)
setConditionParam(condition, CONDITION_PARAM_DELAYED, 1)
addDamageCondition(condition, 10, 2000, -10)
setCombatCondition(combat, condition)
end
setCombatCallback(combat, CALLBACK_PARAM_LEVELMAGICVALUE, "onGetFormulaValues")


function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end