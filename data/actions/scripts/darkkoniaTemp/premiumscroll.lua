function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	player:addPremiumDays(5)
	player:sendTextMessage(MESSAGE_EVENT_ADVANCE, 'You gain 5 premium days time.')
	player:save()
	item:remove(1)
	return true
end
