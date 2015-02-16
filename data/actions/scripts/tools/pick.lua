function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if (target.uid <= 65535 or target.actionid > 0) then
		if (target.itemid == 354 or target.itemid == 355) then
			target:transform(392)
			target:decay()
			toPosition:sendMagicEffect(CONST_ME_POFF)
		end
	end

	return true
end
