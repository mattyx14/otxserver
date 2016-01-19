local combat = Combat()
combat:setParameter(COMBAT_PARAM_TYPE, COMBAT_ENERGYDAMAGE)
combat:setParameter(COMBAT_PARAM_EFFECT, CONST_ME_HITAREA)
combat:setParameter(COMBAT_PARAM_DISTANCEEFFECT, CONST_ANI_ENERGY)

local condition = Condition(CONDITION_ENERGY)
condition:setParameter(CONDITION_PARAM_DELAYED, true)
condition:addDamage(17, 10000, -25)
combat:setCondition(condition)

local area = createCombatArea(AREA_SQUARE1X1)
combat:setArea(area)

function onCastSpell(creature, variant)
	return combat:execute(creature, variant)
end
