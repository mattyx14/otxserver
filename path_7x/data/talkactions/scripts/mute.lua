function onSay(cid, words, param, channel)
	if(param == '') then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Command param required.")
		return true
	end

	local t = string.explode(param, ",")
	local pid = getPlayerByNameWildcard(t[1])
	if(not pid or (isPlayerGhost(pid) and getPlayerGhostAccess(pid) > getPlayerGhostAccess(cid))) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Player " .. t[1] .. " is not currently online.")
		return true
	end

	if(getPlayerAccess(cid) <= getPlayerAccess(pid) or getPlayerFlagValue(pid, PLAYERFLAG_CANNOTBEMUTED)) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Cannot perform action.")
		return true
	end

	if(words:sub(2, 2) == "u") then
		doRemoveCondition(pid, CONDITION_MUTED, 0)
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, getCreatureName(pid) .. " has been unmuted.")
		return true
	end

	local time = tonumber(t[2])
	if(not time or time < 1) then
		time = -1
	end

	doMutePlayer(pid, time)
	doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, getCreatureName(pid) .. " has been muted for" .. (time < 1 and "ever" or " " .. time .. " seconds") .. ".")
	return true
end
