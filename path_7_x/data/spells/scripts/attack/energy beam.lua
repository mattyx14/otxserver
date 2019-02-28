local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_ENERGYDAMAGE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_EXPLOSIONHIT)
setCombatFormula(combat, COMBAT_FORMULA_LEVELMAGIC, -0.52, -0, -0.68, 0)

local area = createCombatArea(AREA_BEAM4, AREADIAGONAL_BEAM4)
setCombatArea(combat, area)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end
