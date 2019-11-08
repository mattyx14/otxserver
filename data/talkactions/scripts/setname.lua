function onSay(player, words, param)
	if not player:isLiveCaster() then
		player:sendTextMessage(MESSAGE_INFO_DESCR, "You don't have live cast.")
		return false
	end

	local split = param:split(",")
	if player:setSpectatorName(split[1], split[2]) then
		-- Cast Channel #40 defined on const.h
		player:sendChannelMessage("[Cast]", "Spectator[" ..split[1] .. "]'s name has been changed to"..split[2]..".", TALKTYPE_CHANNEL_O, 40)
	else
		player:sendTextMessage(MESSAGE_INFO_DESCR, "We couldn't find that spectator, please try again.")
	end
	return false
end
