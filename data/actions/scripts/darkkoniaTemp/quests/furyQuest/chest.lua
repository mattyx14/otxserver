function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if (player:getStorageValue(30492) == 1) then
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "The chest is empty.")
		return true
	end

	player:addItem(7899, 1)
	player:addItem(7894, 1)
	player:addItem(9933, 1)
	player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You found a magma set.")
	player:setStorageValue(30492, 1)
	return true
end
