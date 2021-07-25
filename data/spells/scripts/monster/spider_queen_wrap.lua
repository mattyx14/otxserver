local combat = Combat()
combat:setParameter(COMBAT_PARAM_EFFECT, CONST_ME_HITBYPOISON)
combat:setParameter(COMBAT_PARAM_DISTANCEEFFECT, CONST_ANI_POISON)

local conditionParalize = Condition(CONDITION_PARALYZE)
conditionParalize:setParameter(CONDITION_PARAM_TICKS, 30000)
conditionParalize:setFormula(-0.3, 0, -0.45, 0)
combat:addCondition(conditionParalize)

local conditionOutfit = Condition(CONDITION_OUTFIT)
conditionOutfit:setTicks(30000)
conditionOutfit:setOutfit({lookType = 422})


function onCastSpell(creature, var)
	local target = creature:getTarget()
	if target and target:isPlayer() then
		if combat:execute(creature, var) then
			target:addCondition(conditionOutfit)
			return true
		end
	end
	return false
end
