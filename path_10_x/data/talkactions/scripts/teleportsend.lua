function onSay(cid, words, param, channel)
	if(param == '') then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Command param required.")
		return true
	end

	local t = string.explode(param, ";")
	if(not t[2]) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "No destination specified.")
		return true
	end

	local pid = getPlayerByNameWildcard(t[1])
	if(not pid or (isPlayerGhost(pid) and getPlayerAccess(pid) > getPlayerAccess(cid))) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Player " .. t[1] .. " not found.")
		return true
	end

	local creature = getCreatureByName(t[2])
	local player = getPlayerByNameWildcard(t[2])
	local waypoint = getWaypointPosition(t[2])
	local tile = string.explode(t[2], ",")
	local pos = {x = 0, y = 0, z = 0}

	if(player ~= nil and (not isPlayerGhost(player) or getPlayerAccess(player) <= getPlayerAccess(cid))) then
		pos = getCreaturePosition(player)
	elseif(creature ~= nil and (not isPlayer(creature) or (not isPlayerGhost(creature) or getPlayerAccess(creature) <= getPlayerAccess(cid)))) then
		pos = getCreaturePosition(creature)
	elseif(type(waypoint) == 'table' and waypoint.x ~= 0 and waypoint.y ~= 0) then
		pos = waypoint
	elseif(tile[2] and tile[3]) then
		pos = {x = tile[1], y = tile[2], z = tile[3]}
	else
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Invalid destination specified.")
		return true
	end

	if(not pos or isInArray({pos.x, pos.y}, 0)) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Destination not reachable.")
		return true
	end

	pos = getClosestFreeTile(cid, pos, true)
	if(not pos or isInArray({pos.x, pos.y}, 0)) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Cannot perform action.")
		return true
	end

	local tmp = getCreaturePosition(pid)
	if(doTeleportThing(pid, pos, true) and not isPlayerGhost(pid)) then
		doSendMagicEffect(tmp, CONST_ME_POFF)
		doSendMagicEffect(pos, CONST_ME_TELEPORT)
	end

	return true
end
