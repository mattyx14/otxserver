local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_LOSEENERGY)

local condition = createConditionObject(CONDITION_ATTRIBUTES)
	setConditionParam(condition, CONDITION_PARAM_TICKS, 40000)
	setConditionParam(condition, CONDITION_PARAM_SKILL_DISTANCEPERCENT, 50)
	setConditionParam(condition, CONDITION_PARAM_SKILL_SHIELDPERCENT, 50)
	setConditionParam(condition, CONDITION_PARAM_SKILL_MELEEPERCENT, 50)
	setConditionParam(condition, CONDITION_PARAM_SKILL_FISTPERCENT, 50)
	setCombatCondition(combat, condition)

local area = createCombatArea(AREA_CIRCLE3X3)
setCombatArea(combat, area)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end