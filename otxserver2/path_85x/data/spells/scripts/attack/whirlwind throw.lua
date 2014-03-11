local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_PHYSICALDAMAGE)
setCombatParam(combat, COMBAT_PARAM_BLOCKARMOR, true)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_HITAREA)
setCombatParam(combat, COMBAT_PARAM_DISTANCEEFFECT, CONST_ANI_WEAPONTYPE)
setCombatParam(combat, COMBAT_PARAM_USECHARGES, true)

function onGetFormulaValues(cid, level, skill, attack, element, factor)
	local levelTotal, formula = level / 5, 0.0332
	local normal, elemental = -(skill * attack * formula + levelTotal), math.ceil((skill * element * formula + levelTotal))
	return normal/2, normal, -math.random(elemental/2, elemental)
end

setCombatCallback(combat, CALLBACK_PARAM_SKILLVALUE, "onGetFormulaValues")
function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end
