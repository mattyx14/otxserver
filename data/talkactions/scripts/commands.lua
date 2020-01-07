function onSay(player, words, param)
	if player:getExhaustion(1000) <= 0 then
		player:setExhaustion(1000, 2)
		player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, "Player commands:" .. "\n"
			.. "!buypremium" .. "\n"
			.. "!kills" .. "\n"
			.. "!saveme" .. "\n"
			.. "!serverinfo" .. "\n"
			.. "!shop" .. "\n"
			.. "!online" .. "\n"
			.. "!add" .. " -- for autoloot !add MONSTERNAME\n"
			.. "!autoloot" .. " -- for autoloot managelist loot\n"
			.. "!commands")
		return false
	else
		player:sendTextMessage(MESSAGE_STATUS_SMALL, 'You\'re exhausted for: '..player:getExhaustion(1000)..' seconds.')
	end
end
