function onUse(player, item, fromPosition, target, toPosition)
	player:sendTextMessage(MESSAGE_INFO_DESCR, "The time is " .. getFormattedWorldTime() .. ".")
	return true
end
