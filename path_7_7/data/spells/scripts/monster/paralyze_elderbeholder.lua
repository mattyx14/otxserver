local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, CONST_ME_MAGIC_GREEN)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_RED)

local condition = createConditionObject(CONDITION_PARALYZE)
setConditionParam(condition, CONDITION_PARAM_TICKS, 20*1000)
setConditionFormula(condition, 0, -140, 0, -220)
setCombatCondition(combat, condition)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end