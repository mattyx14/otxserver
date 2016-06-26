local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_ENERGYDAMAGE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_ENERGYAREA)
setCombatFormula(combat, COMBAT_FORMULA_LEVELMAGIC, -0.25, -0, -0.55, 0)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end
