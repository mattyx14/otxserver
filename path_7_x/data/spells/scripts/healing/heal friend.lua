local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_HEALING)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_GREEN)
setCombatParam(combat, COMBAT_PARAM_AGGRESSIVE, false)
setCombatParam(combat, COMBAT_PARAM_DISPEL, CONDITION_PARALYZE)
--setHealingFormula(combat, COMBAT_FORMULA_LEVELMAGIC, 5, 5, 10, 14)
setCombatFormula(combat, COMBAT_FORMULA_LEVELMAGIC, 1.0, -30, 1.35, 0)

function onCastSpell(cid, var)
	doSendMagicEffect(getCreaturePosition(cid), CONST_ME_MAGIC_BLUE)
	return doCombat(cid, combat, var)
end
