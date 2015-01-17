function onSay(cid, words, param, channel)
	if(param == '') then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Command param required.")
		return true
	end

	local players = {}
	if(param:sub(1, 1) ~= '*') then
		local t = string.explode(param, ",")
		if(not t[2]) then
			t[2] = t[1]
		end

		local multifloor = false
		if(t[3]) then
			multifloor = getBooleanFromString(t[3])
		end

		players = getSpectators(getCreaturePosition(cid), t[1], t[2], multifloor)
	else
		players = getPlayersOnline()
	end

	local tmp = 0
	for i, tid in ipairs(players) do
		if(isPlayer(tid) and tid ~= cid and getPlayerAccess(tid) < getPlayerAccess(cid)) then
			doRemoveCreature(tid)
			tmp = tmp + 1
		end
	end

	if(tmp > 0) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Kicked " .. tmp .. " players.")
	else
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Could not kick any player.")
	end

	return true
end
