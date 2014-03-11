local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TARGETCASTERORTOPMOST, true)
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_ENERGYDAMAGE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_ENERGYHIT)
setCombatParam(combat, COMBAT_PARAM_DISTANCEEFFECT, CONST_ANI_ENERGYBALL)
setCombatFormula(combat, COMBAT_FORMULA_LEVELMAGIC, -1, 0, -1, -10, 5, 5, 1.2, 2, -20, -40)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end
