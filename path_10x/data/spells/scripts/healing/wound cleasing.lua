local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_HEALING)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_BLUE)
setCombatParam(combat, COMBAT_PARAM_AGGRESSIVE, false)
setCombatParam(combat, COMBAT_PARAM_DISPEL, CONDITION_PARALYZE)

setCombatCallback(combat, CALLBACK_PARAM_LEVELMAGICVALUE, "onGetFormulaValues")
function onGetFormulaValues(cid, level, maglevel)
	local min = level / 5 + maglevel * 4 + 25
	local max = level / 5 + maglevel * 8 + 50
	return min, max
end

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