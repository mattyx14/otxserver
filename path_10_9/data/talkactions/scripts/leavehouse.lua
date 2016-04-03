function onSay(player, words, param)
	if player:getExhaustion(1000) <= 0 then
		player:setExhaustion(1000, 2)
		local position = player:getPosition()
		local tile = Tile(position)
		local house = tile and tile:getHouse()
		if house == nil then
			player:sendCancelMessage("You are not inside a house.")
			position:sendMagicEffect(CONST_ME_POFF)
			return false
		end

		if house:getOwnerGuid() ~= player:getGuid() then
			player:sendCancelMessage("You are not the owner of this house.")
			position:sendMagicEffect(CONST_ME_POFF)
			return false
		end

		house:setOwnerGuid(0)
		player:sendTextMessage(MESSAGE_INFO_DESCR, "You have successfully left your house.")
		position:sendMagicEffect(CONST_ME_POFF)
		return false
	else
		player:sendTextMessage(MESSAGE_STATUS_SMALL, 'You\'re exhausted for: '..player:getExhaustion(1000)..' seconds.')
	end
end
