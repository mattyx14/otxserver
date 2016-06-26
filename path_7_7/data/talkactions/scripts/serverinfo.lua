function onSay(player, words, param)
	if player:getExhaustion(1000) <= 0 then
		player:setExhaustion(1000, 2)
		player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, "Server Info:"
						.. "\nExp rate: " .. Game.getExperienceStage(player:getLevel())
						.. "\nSkill rate: " .. configManager.getNumber(configKeys.RATE_SKILL)
						.. "\nMagic rate: " .. configManager.getNumber(configKeys.RATE_MAGIC)
						.. "\nLoot rate: " .. configManager.getNumber(configKeys.RATE_LOOT))
		return false
	else
		player:sendTextMessage(MESSAGE_STATUS_SMALL, 'You\'re exhausted for: '..player:getExhaustion(1000)..' seconds.')
	end

end
