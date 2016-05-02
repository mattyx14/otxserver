local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_BLUE)
setCombatParam(combat, COMBAT_PARAM_AGGRESSIVE, false)

local condition = createConditionObject(CONDITION_ATTRIBUTES)
setConditionParam(condition, CONDITION_PARAM_TICKS, 10000)
setConditionParam(condition, CONDITION_PARAM_SKILL_DISTANCEPERCENT, 150)
setConditionParam(condition, CONDITION_PARAM_BUFF, true)
setCombatCondition(combat, condition)

local speed = createConditionObject(CONDITION_PARALYZE)
setConditionParam(speed, CONDITION_PARAM_TICKS, 10000)
setConditionFormula(speed, -0.7, 56, -0.7, 56)
setCombatCondition(combat, speed)

local exhaust = createConditionObject(CONDITION_EXHAUST)
setConditionParam(exhaust, CONDITION_PARAM_SUBID, 2)
setConditionParam(exhaust, CONDITION_PARAM_TICKS, 10000)
setCombatCondition(combat, exhaust)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end
