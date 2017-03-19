local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, CONST_ME_MAGIC_GREEN)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_POISONAREA)
setCombatParam(combat, COMBAT_PARAM_DISTANCEEFFECT, CONST_ANI_POISON)

local condition = createConditionObject(CONDITION_PARALYZE)
setConditionParam(condition, CONDITION_PARAM_TICKS, 30*1000)
setConditionFormula(condition, 0, -210, 0, -320)
setCombatCondition(combat, condition)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end