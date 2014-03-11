local combat = createCombatObject()
setCombatParam(combat, COMBAT_PARAM_TYPE, COMBAT_PHYSICALDAMAGE)
setCombatParam(combat, COMBAT_PARAM_EFFECT, CONST_ME_GROUNDSHAKER)
setCombatParam(combat, COMBAT_PARAM_USECHARGES, true)

local area = createCombatArea(AREA_CIRCLE3X3)
setCombatArea(combat, area)

function onGetFormulaValues(cid, level, skill, attack, element, factor)
	local attackTotal, elementTotal, levelTotal = skill + attack, skill + element, level / 9
	return -(attackTotal * 0.53 + levelTotal), -(attackTotal * 1.0 + levelTotal),
		-math.random(math.ceil(elementTotal * 0.5), math.ceil(elementTotal * 0.9))
end

setCombatCallback(combat, CALLBACK_PARAM_SKILLVALUE, "onGetFormulaValues")
function onCastSpell(cid, var)
	return doCombat(cid, combat, var)
end
