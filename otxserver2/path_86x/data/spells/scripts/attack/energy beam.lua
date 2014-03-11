local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_ENERGYDAMAGE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_ENERGYHIT)
setAttackFormula(combat, COMBAT_FORMULA_LEVELMAGIC, 5, 5, 2.5, 4)

local area = createCombatArea(AREA_BEAM4, AREADIAGONAL_BEAM4)
setCombatArea(combat, area)

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end
