local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_PHYSICALDAMAGE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_MORTAREA)
setCombatParam(combat, COMBAT_PARAM_DISTANCEEFFECT, CONST_ANI_DEATH)
setCombatFormula(combat, COMBAT_FORMULA_LEVELMAGIC, -0.20, -0, -0.50, 0)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end
