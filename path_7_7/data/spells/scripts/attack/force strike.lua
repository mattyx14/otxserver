local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_PHYSICALDAMAGE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_MORTAREA)
setCombatFormula(combat, COMBAT_FORMULA_LEVELMAGIC, -0.20, -0, -0.50, 0)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end
