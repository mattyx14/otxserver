function onUse(player, item, fromPosition, target, toPosition)
	-- TODO: Different music effect for different instruments.
	item:getPosition():sendMagicEffect(CONST_ME_SOUND_BLUE)
	return true
end
