local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_ENERGYDAMAGE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_EXPLOSIONHIT)
setCombatFormula(combat, COMBAT_FORMULA_LEVELMAGIC, -1.3, -0, -1.7, 0)

local area = createCombatArea(AREA_BEAM6, AREADIAGONAL_BEAM6)
setCombatArea(combat, area)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end
