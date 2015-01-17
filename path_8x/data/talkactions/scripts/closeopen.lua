function onSay(cid, words, param, channel)
	local state = GAMESTATE_CLOSED
	if(words:sub(2, 2) == "o") then
		state = GAMESTATE_NORMAL
	end

	local str = "Failed to " .. (state == GAMESTATE_CLOSED and "close" or "open") .. " server."
	if(doSetGameState(state)) then
		str = "Server has been " .. (state == GAMESTATE_CLOSED and "closed" or "opened") .. " successfully."
	end

	doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, str)
	return true
end
