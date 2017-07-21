function onUse(player, item, fromPosition, itemEx, toPosition)
	if player:getStorageValue(50730) < 1 then
		player:sendTextMessage(MESSAGE_STATUS_SMALL, 'You not have permission.')
		return true
	end

	player:openImbuementWindow(itemEx)
	return true
end
