local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_GREEN)
setCombatParam(combat, COMBAT_PARAM_AGGRESSIVE, false)

local condition = createConditionObject(CONDITION_HASTE)
setConditionParam(condition, CONDITION_PARAM_TICKS, 33000)
setConditionFormula(condition, 0.3, -24, 0.3, -24)
setCombatCondition(combat, condition)

function onCastSpell(cid, var)
	if isPlayer(cid) == true then
	if exhaustion.check(cid, 88774) then
		return false
	else
		return doRemoveCondition(cid, CONDITION_PARALYZE), doCombat(cid, combat, var)
	end
	else
		return doRemoveCondition(cid, CONDITION_PARALYZE), doCombat(cid, combat, var)
	end
end
