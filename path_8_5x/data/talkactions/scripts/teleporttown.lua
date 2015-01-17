function onSay(cid, words, param, channel)
	local master = false
	if(words == '/t') then
		master = true
	elseif(param == '') then
		local str = ""
		for i, town in ipairs(getTownList()) do
			str = str .. town.name .. "\n"
		end

		doShowTextDialog(cid, ITEM_ACTION_BOOK, str)
		return true
	end

	local tid, t = cid, string.explode(param, ",")
	if(t[(master and 1 or 2)]) then
		tid = getPlayerByNameWildcard(t[(master and 1 or 2)])
		if(not tid or (isPlayerGhost(tid) and getPlayerAccess(tid) > getPlayerAccess(cid))) then
			doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Player " .. t[(master and 1 or 2)] .. " not found.")
			return true
		end
	end

	local tmp = getPlayerTown(tid)
	if(not master) then
		tmp = t[1]
		if(not tonumber(tmp)) then
			tmp = getTownId(tmp)
			if(not tmp) then
				doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Town " .. t[1] .. " does not exists.")
				return true
			end
		end
	end

	local pos = getTownTemplePosition(tmp)
	if(type(pos) ~= 'table' or isInArray({pos.x, pos.y}, 0)) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Town " .. t[1] .. " does not exists or has invalid temple position.")
		return true
	end

	pos = getClosestFreeTile(tid, pos)
	if(type(pos) ~= 'table' or isInArray({pos.x, pos.y}, 0)) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Destination not reachable.")
		return true
	end

	tmp = getCreaturePosition(tid)
	if(doTeleportThing(tid, pos) and not isPlayerGhost(tid)) then
		doSendMagicEffect(tmp, CONST_ME_POFF)
		doSendMagicEffect(pos, CONST_ME_TELEPORT)
	end

	return true
end
