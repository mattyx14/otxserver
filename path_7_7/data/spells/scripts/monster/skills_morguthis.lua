local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_GREEN)
setCombatParam(combat, COMBAT_PARAM_AGGRESSIVE, 0)

local condition = createConditionObject(CONDITION_ATTRIBUTES)
	setConditionParam(condition, CONDITION_PARAM_TICKS, 5000)
	setConditionParam(condition, CONDITION_PARAM_SKILL_DISTANCEPERCENT, 160)
	setConditionParam(condition, CONDITION_PARAM_SKILL_SHIELDPERCENT, 160)
	setConditionParam(condition, CONDITION_PARAM_SKILL_MELEEPERCENT, 160)
	setConditionParam(condition, CONDITION_PARAM_SKILL_FISTPERCENT, 160)
	setCombatCondition(combat, condition)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end