local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_BLUE)
setCombatParam(combat, COMBAT_PARAM_AGGRESSIVE, false)

local condition = createConditionObject(CONDITION_HASTE)
setConditionParam(condition, CONDITION_PARAM_TICKS, 10000)
setConditionFormula(condition, 0.8, -72, 0.8, -72)
setCombatCondition(combat, condition)

local disable = createConditionObject(CONDITION_PACIFIED)
setConditionParam(disable, CONDITION_PARAM_TICKS, 10000)
setCombatCondition(combat, disable)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end
