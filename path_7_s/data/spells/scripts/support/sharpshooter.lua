local combat = Combat()
combat:setParameter(COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_GREEN)
combat:setParameter(COMBAT_PARAM_AGGRESSIVE, 0)

local condition = Condition(CONDITION_ATTRIBUTES)
condition:setParameter(CONDITION_PARAM_TICKS, 10000)
condition:setParameter(CONDITION_PARAM_SKILL_DISTANCEPERCENT, 150)
condition:setParameter(CONDITION_PARAM_SKILL_SHIELDPERCENT, -100)
condition:setParameter(CONDITION_PARAM_BUFF_SPELL, 1)
combat:setCondition(condition)

local speed = Condition(CONDITION_HASTE)
speed:setParameter(CONDITION_PARAM_TICKS, 10000)
speed:setFormula(-0.7, 56, -0.7, 56)
combat:setCondition(speed)

local exhaust = Condition(CONDITION_EXHAUST)
exhaust:setParameter(CONDITION_PARAM_SUBID, 2)
exhaust:setParameter(CONDITION_PARAM_TICKS, 10000)
combat:setCondition(exhaust)

function onCastSpell(creature, var)
	return combat:execute(creature, var)
end
