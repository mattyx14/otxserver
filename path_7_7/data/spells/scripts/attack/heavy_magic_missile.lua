local combat = Combat()
combat:setParameter(COMBAT_PARAM_TYPE, COMBAT_ENERGYDAMAGE)
combat:setParameter(COMBAT_PARAM_EFFECT, CONST_ME_EXPLOSIONHIT)
combat:setParameter(COMBAT_PARAM_DISTANCEEFFECT, CONST_ANI_FIRE)

function onGetFormulaValues(player, level, maglevel)
	min = -((level * 2) + (maglevel * 3)) * 0.2
	max = -((level * 2) + (maglevel * 3)) * 0.4
	return min, max
end

combat:setCallback(CALLBACK_PARAM_LEVELMAGICVALUE, "onGetFormulaValues")

function onCastSpell(creature, var)
	-- check for stairHop delay
	if not getCreatureCondition(creature, CONDITION_PACIFIED) then
		-- check making it able to shot invisible creatures
		if Tile(var:getPosition()):getTopCreature() then
			return combat:execute(creature, var)
		else
			creature:sendCancelMessage("You can only use this rune on creatures.")
			creature:getPosition():sendMagicEffect(CONST_ME_POFF)
			return false
		end
	else
		-- attack players even with stairhop delay
		if Tile(var:getPosition()):getTopCreature() then
			if Tile(var:getPosition()):getTopCreature():isPlayer() then
				return combat:execute(creature, var)
			else
				creature:sendCancelMessage(RETURNVALUE_YOUAREEXHAUSTED)
				creature:getPosition():sendMagicEffect(CONST_ME_POFF)
				return false
			end
		else
			creature:sendCancelMessage(RETURNVALUE_YOUAREEXHAUSTED)
			creature:getPosition():sendMagicEffect(CONST_ME_POFF)
			return false
		end
	end
end
