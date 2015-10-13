local combat = Combat()
combat:setParameter(COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_GREEN)
combat:setParameter(COMBAT_PARAM_AGGRESSIVE, 0)

local condition = Condition(CONDITION_HASTE)
condition:setParameter(CONDITION_PARAM_TICKS, 10000)
condition:setFormula(0.8, -72, 0.8, -72)
combat:setCondition(condition)

local exhaust = Condition(CONDITION_EXHAUST)
exhaust:setParameter(CONDITION_PARAM_SUBID, 1)
exhaust:setParameter(CONDITION_PARAM_TICKS, 10000)
combat:setCondition(exhaust)

local disable = Condition(CONDITION_PACIFIED)
disable:setParameter(CONDITION_PARAM_TICKS, 10000)
combat:setCondition(disable)

function onCastSpell(creature, var)
	return combat:execute(creature, var)
end
