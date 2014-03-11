local config = {
	access = 3 -- to see other players position
}

function onSay(cid, words, param, channel)
	local pid = cid
	if(param ~= '' and getPlayerAccess(cid) >= config.access) then
		pid = getPlayerByNameWildcard(param)
		if(not pid or (isPlayerGhost(pid) and getPlayerGhostAccess(pid) > getPlayerGhostAccess(cid))) then
			doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Player " .. param .. " is not currently online.")
			return true
		end
	end

	local position = getCreaturePosition(pid)
	doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, (pid == cid and "Your" or getCreatureName(pid)) .. " current position is [X: " .. position.x .. " | Y: " .. position.y .. " | Z: " .. position.z .. "]")
	return true
end
