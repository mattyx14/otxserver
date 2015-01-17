function onSay(cid, words, param, channel)
	local func = doCreateMonster
	if(words:sub(2, 2) == "n") then
		func = doCreateNpc
	end

	local pid, t = cid, string.explode(param, ",")
	if(t[2]) then
		pid = getPlayerByNameWildcard(t[2])
		if(not pid) then
			doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Player " .. t[2] .. " not found.")
			return true
		end
	end

	local position, effect = getCreaturePosition(pid), CONST_ME_MAGIC_RED
	errors(false)
	local ret = func(t[1], position)
	errors(true)

	if(tonumber(ret) == nil) then
		effect = CONST_ME_POFF
		doPlayerSendDefaultCancel(cid, (not ret and RETURNVALUE_NOTPOSSIBLE or RETURNVALUE_NOTENOUGHROOM))
	end

	doSendMagicEffect(position, effect)
	return true
end
