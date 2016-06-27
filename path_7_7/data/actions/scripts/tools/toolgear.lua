function onUse(player, item, fromPosition, target, toPosition)
	return onUseRope(player, item, fromPosition, target, toPosition)
		or onUseShovel(player, item, fromPosition, target, toPosition)
		or onUsePick(player, item, fromPosition, target, toPosition)
		or onUseMachete(player, item, fromPosition, target, toPosition)
		or onUseScythe(player, item, fromPosition, target, toPosition)
end
