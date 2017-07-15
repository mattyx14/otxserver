function onUse(cid, item, fromPosition, itemEx, toPosition)
	local player = Player(cid)
	if not player then
		return true
	end

	if player:getStorageValue(2526) >= 1 then
		player:openImbuementWindow(itemEx)
	else
		player:sendTextMessage(MESSAGE_STATUS_SMALL, 'You not have permission.')
	end
	return true
end
