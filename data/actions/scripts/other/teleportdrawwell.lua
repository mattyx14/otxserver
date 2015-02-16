function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if target.actionid == 1000 and target.itemid == 1369 then
		fromPosition.z = fromPosition.z + 1
		player:teleportTo(fromPosition)
	end
	return true
end