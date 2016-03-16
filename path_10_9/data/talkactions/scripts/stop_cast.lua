function onSay(player, words, param)
	if player:getExhaustion(1000) <= 0 then
		player:setExhaustion(1000, 10)
	else
		print('You\'re exhausted for: '..player:getExhaustion(1000)..' seconds.')
	end

	if player:stopLiveCast(param) then
		player:sendTextMessage(MESSAGE_INFO_DESCR, "You have stopped casting your gameplay.")
	else
		player:sendCancelMessage("You're not casting your gameplay.")
	end
	return false
end
