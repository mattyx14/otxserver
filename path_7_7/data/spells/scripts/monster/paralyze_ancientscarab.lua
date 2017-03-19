local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, CONST_ME_MAGIC_GREEN)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_POISONAREA)
setCombatParam(combat, COMBAT_PARAM_DISTANCEEFFECT, CONST_ANI_POISON)

local condition = createConditionObject(CONDITION_PARALYZE)
setConditionParam(condition, CONDITION_PARAM_TICKS, 25*1000)
setConditionFormula(condition, 0, -160, 0, -200)
setCombatCondition(combat, condition)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end