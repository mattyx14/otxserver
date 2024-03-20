local function doRemoveObject(pos)
	local object = getThingfromPos(pos)
	if(object.uid > 0 and (not isItemDoor(object.itemid)) and (not isCreature(object.uid)) and not isCorpse(object.uid)) then
		doRemoveItem(object.uid)
		doRemoveObject(pos, true)
		return LUA_NO_ERROR
	elseif(object.uid > 0) then
		pos.stackpos = pos.stackpos + 1
		return doRemoveObject(pos)
	end
end

function onStepOut(cid, item, position, fromPosition)
	local uid = item.uid
	local newPosition = {x = position.x, y = position.y, z = position.z}
	if(isInArray(verticalOpenDoors, item.itemid)) then
		newPosition.x = newPosition.x + 1
	else
		newPosition.y = newPosition.y + 1
	end
	doTransformItem(uid, item.itemid - 1)
	doRelocate(position, newPosition, true, true)
	local tmpPos = {x = position.x, y = position.y, z = position.z, stackpos = -1}
	local tileCount = getTileThingByPos(tmpPos)

	if tileCount > 0 then
		position.stackpos = 1
		doRemoveObject(position)
	end
	return true
end
