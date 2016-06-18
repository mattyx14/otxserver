local config = {
	days = 90,
	maxDays = 365,
	price = 10000
}

function onSay(player, words, param)
	if player:getExhaustion(1000) <= 0 then
		player:setExhaustion(1000, 2)
		if configManager.getBoolean(configKeys.FREE_PREMIUM) then
			return true
		end

		if player:getPremiumDays() <= config.maxDays then
			if player:removeMoney(config.price) then
				player:addPremiumDays(config.days)
				player:sendTextMessage(MESSAGE_INFO_DESCR, "You have bought " .. config.days .." days of premium account.")
			else
				player:sendCancelMessage("You don't have enough money, " .. config.maxDays .. " days premium account costs " .. config.price .. " gold coins.")
				player:getPosition():sendMagicEffect(CONST_ME_POFF)
			end
		else
			player:sendCancelMessage("You can not buy more than " .. config.maxDays .. " days of premium account.")
			player:getPosition():sendMagicEffect(CONST_ME_POFF)
		end
		return false
	else
		player:sendTextMessage(MESSAGE_STATUS_SMALL, 'You\'re exhausted for: '..player:getExhaustion(1000)..' seconds.')
	end
end
