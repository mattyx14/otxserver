local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_GREEN_RINGS)
setCombatParam(combat, COMBAT_PARAM_DISTANCEEFFECT, CONST_ANI_POISON)

local condition = createConditionObject(CONDITION_PARALYZE)
setConditionParam(condition, CONDITION_PARAM_TICKS, 20000)
setConditionFormula(condition, -0.6, 0, -0.8, 0)
addCombatCondition(combat, condition)

	local area = createCombatArea(AREA_CIRCLE2X2)
	setCombatArea(combat, area)
	addCombatCondition(combat, condition)


function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end