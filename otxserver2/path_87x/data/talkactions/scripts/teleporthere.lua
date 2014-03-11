function onSay(cid, words, param, channel)
	if(param == '') then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Command param required.")
		return true
	end

	local target = getPlayerByNameWildcard(param)
	if(not target) then
		target = getCreatureByName(param)
		if(not target) then
			doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Creature not found.")
			return true
		end
	end

	if(isPlayerGhost(target) and getPlayerGhostAccess(target) > getPlayerGhostAccess(cid)) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Creature not found.")
		return true
	end

	local pos = getClosestFreeTile(target, getCreaturePosition(cid), false, false)
	if(not pos or isInArray({pos.x, pos.y}, 0)) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Cannot perform action.")
		return true
	end

	local tmp = getCreaturePosition(target)
	if(doTeleportThing(target, pos, true) and not isPlayerGhost(target)) then
		doSendMagicEffect(tmp, CONST_ME_POFF)
		doSendMagicEffect(pos, CONST_ME_TELEPORT)
	end

	return true
end
