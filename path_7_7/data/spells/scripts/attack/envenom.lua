local combat = Combat()
combat:setParameter(COMBAT_PARAM_TYPE, COMBAT_EARTHDAMAGE)
combat:setParameter(COMBAT_PARAM_EFFECT, CONST_ME_GREEN_RINGS)
combat:setParameter(COMBAT_PARAM_DISTANCEEFFECT, CONST_ANI_POISON)

local condition = Condition(CONDITION_POISON)
condition:setParameter(CONDITION_PARAM_DELAYED, true)
--[[
condition:addDamage(3, 2000, -25)
condition:addDamage(3, 3000, -5)
condition:addDamage(4, 4000, -4)
condition:addDamage(6, 6000, -3)
condition:addDamage(9, 8000, -2)
condition:addDamage(12, 10000, -1)
]]--
condition:setParameter(CONDITION_PARAM_MINVALUE, 20)
condition:setParameter(CONDITION_PARAM_MAXVALUE, 70)
condition:setParameter(CONDITION_PARAM_STARTVALUE, 5)
condition:setParameter(CONDITION_PARAM_TICKINTERVAL, 6000)
condition:setParameter(CONDITION_PARAM_FORCEUPDATE, true)

combat:setCondition(condition)

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