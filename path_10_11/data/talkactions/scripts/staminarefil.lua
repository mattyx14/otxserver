function onSay(player, words, param)
	if player:getStamina() >= 2520 then
		player:sendCancelMessage("Your stamina is already full.")
	elseif player:getPremiumDays() < 5 then
		player:sendCancelMessage("You must have a 5 premium days on account for to use this command.")
	else
		player:removePremiumDays(5)
		player:setStamina(2520)
		player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, "Your stamina has been refilled.")
	end
end
