local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_PHYSICALDAMAGE)
setCombatParam(combat, COMBAT_PARAM_BLOCKARMOR, 1)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_EXPLOSIONAREA)
setCombatParam(combat, COMBAT_PARAM_DISTANCEEFFECT, CONST_ANI_FIRE)

local area = createCombatArea(AREA_CROSS1X1)
setCombatArea(combat, area)

function onGetFormulaValues(cid, level, maglevel)
	min = 0
	max = -((level * 2) + (maglevel * 3)) * 1
--	min = -((level * 2) + (maglevel * 3)) * 0.15
--	max = -((level * 2) + (maglevel * 3)) * 0.9
	return min, max
end

setCombatCallback(combat, CALLBACK_PARAM_LEVELMAGICVALUE, "onGetFormulaValues")

function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end
