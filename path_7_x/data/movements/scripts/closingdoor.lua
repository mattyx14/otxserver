function onStepOut(cid, item, position, lastPosition)
	if(getTileInfo(position).creatures > 0) then
		return true
	end

	local newPosition = {x = position.x + 1, y = position.y, z = position.z}
	local query = doTileQueryAdd(cid, newPosition, 6)
	if query ~= RETURNVALUE_NOERROR or query == RETURNVALUE_NOTENOUGHROOM then
		newPosition.x = newPosition.x - 1
		newPosition.y = newPosition.y + 1
		query = doTileQueryAdd(cid, newPosition, 6) -- repeat until found
	end
	if query == RETURNVALUE_NOERROR or ((not query == RETURNVALUE_NOTENOUGHROOM) and (not query == RETURNVALUE_NOTPOSSIBLE)) then
		doRelocate(position, newPosition)
	end

	position.stackpos = -1
	local i, tileItem, tileCount = 1, {uid = 1}, getTileThingByPos(position)
	while(tileItem.uid ~= 0 and i < tileCount) do
		position.stackpos = i
		tileItem = getTileThingByPos(position)
		if(tileItem.uid ~= 0 and tileItem.uid ~= item.uid and not isMovable(tileItem.uid) and not isCorpse(tileItem.uid)) then
			doRemoveItem(tileItem.uid)
		else
			i = i + 1
		end
	end

	local itemInfo = getItemInfo(item.itemid)
	doTransformItem(item.uid, itemInfo.transformUseTo)
	return true
end