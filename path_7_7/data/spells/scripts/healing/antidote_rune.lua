local combat = Combat()
combat:setParameter(COMBAT_PARAM_EFFECT, CONST_ME_MAGIC_BLUE)
combat:setParameter(COMBAT_PARAM_AGGRESSIVE, 0)
combat:setParameter(COMBAT_PARAM_DISPEL, CONDITION_POISON)

function onCastSpell(creature, var)
	if Tile(var:getPosition()):getTopCreature() then
		return combat:execute(creature, var)
	else
		creature:sendCancelMessage("You can only use this rune on creatures.")
		creature:getPosition():sendMagicEffect(CONST_ME_POFF)
	end
end
