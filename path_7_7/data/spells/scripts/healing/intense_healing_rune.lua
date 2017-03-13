local combat = Combat()
combat:setParameter(COMBAT_PARAM_TYPE, COMBAT_HEALING)
combat:setParameter(COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_BLUE)
combat:setParameter(COMBAT_PARAM_AGGRESSIVE, 0)
combat:setParameter(COMBAT_PARAM_TARGETCASTERORTOPMOST, 1)
combat:setParameter(COMBAT_PARAM_DISPEL, CONDITION_PARALYZE)

function onGetFormulaValues(player, level, maglevel)
	if (((level * 2) + (maglevel * 3)) * 0.335) < 35 then
		min = 35
	else
		min = ((level * 2) + (maglevel * 3)) * 0.335
	end
	if (((level * 2) + (maglevel * 3)) * 0.58) < 45 then
		max = 45
	else
		max = ((level * 2) + (maglevel * 3)) * 0.58
	end
	return min, max
end

combat:setCallback(CALLBACK_PARAM_LEVELMAGICVALUE, "onGetFormulaValues")

function onCastSpell(creature, var)
	if Tile(var:getPosition()):getTopCreature() then
		return combat:execute(creature, var)
	else
		creature:sendCancelMessage("You can only use this rune on creatures.")
		creature:getPosition():sendMagicEffect(CONST_ME_POFF)
	end
end
