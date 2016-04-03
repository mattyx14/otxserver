function onSay(player, words, param)
	if player:getExhaustion(1000) <= 0 then
		player:setExhaustion(1000, 2)
		player:save()
		player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, "My player is saved ...")
	else
		player:sendTextMessage(MESSAGE_STATUS_SMALL, 'You\'re exhausted for: '..player:getExhaustion(1000)..' seconds.')
	end
end
