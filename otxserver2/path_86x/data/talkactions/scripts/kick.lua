function onSay(cid, words, param, channel)
	local pid = 0
	if(param == '') then
		pid = getCreatureTarget(cid)
		if(pid == 0) then
			doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Command param required.")
			return true
		end
	else
		pid = getPlayerByNameWildcard(param)
	end

	if(not pid or (isPlayerGhost(pid) and getPlayerGhostAccess(pid) > getPlayerGhostAccess(cid))) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Player " .. param .. " is not currently online.")
		return true
	end

	if(isPlayer(pid) and getPlayerAccess(pid) >= getPlayerAccess(cid)) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "You cannot kick this player.")
		return true
	end

	doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, getCreatureName(pid) .. " has been kicked.")
	doRemoveCreature(pid)
	return true
end
