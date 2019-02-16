local SPECIAL_QUESTS = {2001}

function onStepIn(cid, item, position, lastPosition, fromPosition, toPosition, actor)
	if(not isPlayer(cid)) then
		return true
	end

	local container = isContainer(item.uid)
	if((container and not isInArray(SPECIAL_QUESTS, item.actionid) and
		item.uid > 65535) or (not container and getTileInfo(position).creatures <= 1)
		or (fromPosition.x ~= 0 and getTileInfo(fromPosition).floorChange[9])) then
		return true
	end

	if(fromPosition.x == 0) then -- player just logged in
		lastPosition = getTownTemplePosition(getPlayerTown(cid))
		doSendMagicEffect(lastPosition, CONST_ME_TELEPORT)
	end

	doTeleportThing(cid, lastPosition, true)
	return true
end
