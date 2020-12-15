function onUse(player, item, fromPosition, itemEx, toPosition, isHotkey)
	if player:getStamina() >= 2400 then
		player:sendCancelMessage("Your stamina is already full.")
	elseif player:getPremiumDays() < 5 then
		player:sendCancelMessage("You must have a 5 premium days on account for to use this charm.")
	else
		player:removePremiumDays(5)
		player:setStamina(2400)
		player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, "Your stamina has been refilled.")
		item:remove(1)
		player:save()
	end

	return true
end
