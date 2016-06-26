function onSay(player, words, param)
	if player:getExhaustion(1000) <= 0 then
		player:setExhaustion(1000, 2)
	else
		print('You\'re exhausted for: '..player:getExhaustion(1000)..' seconds.')
	end

	local uptime = getWorldUpTime()

	local hours = math.floor(uptime / 3600)
	local minutes = math.floor((uptime - (3600 * hours)) / 60)
	player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, "Uptime: " .. hours .. " hours and " .. minutes .. " minutes.")
	return false
end
