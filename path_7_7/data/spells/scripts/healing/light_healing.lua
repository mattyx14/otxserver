local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_HEALING)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_BLUE)
setCombatParam(combat, COMBAT_PARAM_AGGRESSIVE, 0)
setCombatParam(combat, COMBAT_PARAM_DISPEL, CONDITION_PARALYZE)

function onGetFormulaValues(cid, level, maglevel)
	if (((level * 2) + (maglevel * 3)) * 0.1) < 20 then
		min = 20
	else
		min = ((level * 2) + (maglevel * 3)) * 0.1
	end
	if (((level * 2) + (maglevel * 3)) * 0.35) < 25 then
		max = 25
	else
		max = ((level * 2) + (maglevel * 3)) * 0.35
	end
	return min, max
end

setCombatCallback(combat, CALLBACK_PARAM_LEVELMAGICVALUE, "onGetFormulaValues")

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end
