local config = {
	maxLevel = getConfigInfo('maximumDoorLevel')
}

function onUse(cid, item, fromPosition, itemEx, toPosition)
	if(fromPosition.x ~= CONTAINER_POSITION and isPlayerPzLocked(cid) and getTileInfo(fromPosition).protection) then
		doPlayerSendDefaultCancel(cid, RETURNVALUE_NOTPOSSIBLE)
		return true
	end

	local locked = DOORS[item.itemid]
	if(locked) then
		doPlayerSendTextMessage(cid, MESSAGE_INFO_DESCR, "It is locked.")
		return true
	end

	local door = getItemInfo(item.itemid)
	if(door.levelDoor > 0) then
		if(item.aid == 189) then
			if(not isPremium(cid)) then
				doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "Only the worthy may pass.")
				return true
			end

			doTeleportThing(cid, toPosition)
			return false
		end

		local gender = item.aid - 186
		if(isInArray({PLAYERSEX_FEMALE,  PLAYERSEX_MALE}, gender)) then
			if(gender ~= getPlayerSex(cid)) then
				doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "Only the worthy may pass.")
				return true
			end

			doTeleportThing(cid, toPosition)
			return false
		end

		local skull = item.aid - 180
		if(skull >= SKULL_NONE and skull <= SKULL_BLACK) then
			if(skull ~= getCreatureSkullType(cid)) then
				doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "Only the worthy may pass.")
				return true
			end

			doTeleportThing(cid, toPosition)
			return false
		end

		local group = item.aid - 150
		if(group >= 0 and group < 30) then
			if(group > getPlayerGroupId(cid)) then
				doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "Only the worthy may pass.")
				return true
			end

			doTeleportThing(cid, toPosition)
			return false
		end

		local vocation = item.aid - 100
		if(vocation >= 0 and vocation < 50) then
			local vocationEx = getVocationInfo(getPlayerVocation(cid))
			if(vocationEx.id ~= vocation and vocationEx.fromVocation ~= vocation) then
				doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "Only the worthy may pass.")
				return true
			end

			doTeleportThing(cid, toPosition)
			return false
		end

		if(item.aid == 190 or (item.aid >= 1000 and (item.aid - door.levelDoor) <= config.maxLevel and getPlayerLevel(cid) >= (item.aid - door.levelDoor))) then
			doTeleportThing(cid, toPosition)
			return false
		end

		doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "Only the worthy may pass.")
		return true
	end

	if(door.specialDoor) then
		if(item.aid == 100 or (item.aid ~= 0 and getCreatureStorage(cid, item.aid) ~= EMPTY_STORAGE)) then
			doTeleportThing(cid, toPosition)
			return false
		end

		doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "The door seems to be sealed against unwanted intruders.")
		return true
	end

	if(getTileInfo(toPosition).creatures > 0) then -- check only if there are any creatures
		local position = {x = toPosition.x, y = toPosition.y, z = toPosition.z, stackpos = STACKPOS_TOP_CREATURE}
		position.x = position.x + 1

		local query = doTileQueryAdd(cid, position, 20)
		if(query == RETURNVALUE_NOTPOSSIBLE) then
			position.x = position.x - 1
			position.y = position.y + 1
			query = doTileQueryAdd(cid, position, 20)
		end

		if(query ~= RETURNVALUE_NOERROR) then
			doPlayerSendDefaultCancel(cid, query)
			return true
		end

		toPosition.stackpos = STACKPOS_TOP_CREATURE
		while(true) do
			local thing = getThingFromPosition(toPosition)
			if(thing.uid == 0) then
				break
			end

			doTeleportThing(thing.uid, position)
		end
	end

	local field = getTileItemByType(toPosition, ITEM_TYPE_MAGICFIELD)
	if(field.uid ~= 0) then
		doRemoveItem(field.uid)
	end

	return door.closingDoor
end
