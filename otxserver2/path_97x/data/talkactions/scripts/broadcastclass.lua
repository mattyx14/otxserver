function onSay(cid, words, param, channel)
	if(param == '') then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Command param required.")
		return true
	end

	local t = string.explode(param, " ", 1)
	if(MESSAGE_TYPES[t[1]] == nil) then
		t = t[1] .. " " .. t[2]
	end

	if(type(t) == 'table') then
		doBroadcastMessage(t[2], MESSAGE_TYPES[t[1]])
	else
		doBroadcastMessage(t)
	end

	return true
end
