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
	local skill = SKILL_IDS[t[2]]
	if(not skill) then
		local tmp = t[2]:sub(1, 1)
		if(tmp == 'l' or tmp == 'e') then
			skill = SKILL__LEVEL
		elseif(tmp == 'm') then
			skill = SKILL__MAGLEVEL
		else
			skill = tonumber(t[2])
			if(not skill or skill < SKILL_FIRST or SKILL > SKILL__PRE_LAST) then
				doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Such skill does not exists.")
				return true
			end
		end
	end

	local amount = tonumber(t[3])
	if(not amount or amount == 0) then
		amount = 1
	end

	doPlayerAddSkill(pid, skill, amount, true)
	return true
end
