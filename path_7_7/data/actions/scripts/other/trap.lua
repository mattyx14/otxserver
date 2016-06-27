function onUse(player, item, fromPosition, target, toPosition)
	item:transform(item:getId() - 1)
	fromPosition:sendMagicEffect(CONST_ME_POFF)
	return true
end
