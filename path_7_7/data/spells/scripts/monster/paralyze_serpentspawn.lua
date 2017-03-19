local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, CONST_ME_MAGIC_GREEN)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_GREEN_RINGS)
setCombatParam(combat, COMBAT_PARAM_DISTANCEEFFECT, CONST_ANI_POISON)

local condition = createConditionObject(CONDITION_PARALYZE)
setConditionParam(condition, CONDITION_PARAM_TICKS, 120*1000)
setConditionFormula(condition, 0, -110, 0, -150)
setCombatCondition(combat, condition)

local area = createCombatArea(AREA_CIRCLE2X2)
setCombatArea(combat, area)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end