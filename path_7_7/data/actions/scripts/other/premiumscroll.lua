function onUse(player, item, fromPosition, target, toPosition)
	player:addPremiumDays(30)
	player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You have received 30 premium days.")
	item:remove(1)
	return true
end
