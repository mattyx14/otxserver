function onSay(cid, words, param, channel)
	if(param == '') then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Command param required.")
		return true
	end

	local t = string.explode(param, ",")
	if(not t[2]) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Not enough params.")
		return true
	end

	local pid = getPlayerByNameWildcard(t[1])
	if(not pid or (isPlayerGhost(pid) and getPlayerGhostAccess(pid) > getPlayerGhostAccess(cid))) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Player " .. t[1] .. " not found.")
		return true
	end

	t[2] = t[2]:lower()
	local blessing = BLESS_IDS[t[2]]
	if(not blessing) then
		blessing = tonumber(t[2])
		if(not blessing or blessing < BLESS_FIRST or blessing > BLESS_LAST) then
			doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "This bless not found.")
			return true
		end
	end

	doPlayerAddBlessing(pid, blessing)
	doSendMagicEffect(getCreaturePosition(cid), CONST_ME_MAGIC_BLUE)
	return true
end

