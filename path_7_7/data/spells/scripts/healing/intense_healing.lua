local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_HEALING)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_BLUE)
setCombatParam(combat, COMBAT_PARAM_AGGRESSIVE, 0)
setCombatParam(combat, COMBAT_PARAM_DISPEL, CONDITION_PARALYZE)

function onGetFormulaValues(cid, level, maglevel)
	if (((level * 2) + (maglevel * 3)) * 0.3) < 35 then
		min = 35
	else
		min = ((level * 2) + (maglevel * 3)) * 0.3
	end
	if (((level * 2) + (maglevel * 3)) * 0.7) < 45 then
		max = 45
	else
		max = ((level * 2) + (maglevel * 3)) * 0.7
	end
	return min, max
end

setCombatCallback(combat, CALLBACK_PARAM_LEVELMAGICVALUE, "onGetFormulaValues")

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end
