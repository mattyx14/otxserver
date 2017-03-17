local function isInvited(houseId, playerName)
	if(string.find(string.lower(getHouseAccessList(houseId, 0x100)), playerName) or string.find(string.lower(getHouseAccessList(houseId, 0x101)), playerName)) then
		return true
	end

	return false
end

function onMoveItem(moveItem, frompos, position, cid)
	if((getPlayerGroupId(cid) < 6) and (getTileInfo(position).house) and (getHouseOwner(getHouseFromPos(position)) ~= getPlayerGUID(cid)) and (not isInvited(getHouseFromPos(position), string.lower(getCreatureName(cid))))) then
		doPlayerSendCancel(cid, "You cannot throw there.")
	else
		return true
	end
end
