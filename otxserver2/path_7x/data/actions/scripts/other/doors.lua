local function doorEnter(cid, uid, id, position)
	doTransformItem(uid, id)
	doTeleportThing(cid, position)
	return true
end

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

			return doorEnter(cid, item.uid, door.transformUseTo, toPosition)
		end

		local gender = item.aid - 186
		if(isInArray({PLAYERSEX_FEMALE,  PLAYERSEX_MALE}, gender)) then
			if(gender ~= getPlayerSex(cid)) then
				doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "Only the worthy may pass.")
				return true
			end

			return doorEnter(cid, item.uid, door.transformUseTo, toPosition)
		end

		local skull = item.aid - 180
		if(skull >= SKULL_NONE and skull <= SKULL_BLACK) then
			if(skull ~= getCreatureSkullType(cid)) then
				doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "Only the worthy may pass.")
				return true
			end

			return doorEnter(cid, item.uid, door.transformUseTo, toPosition)
		end

		local group = item.aid - 150
		if(group >= 0 and group < 30) then
			if(group > getPlayerGroupId(cid)) then
				doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "Only the worthy may pass.")
				return true
			end

			return doorEnter(cid, item.uid, door.transformUseTo, toPosition)
		end

		local vocation = item.aid - 100
		if(vocation >= 0 and vocation < 50) then
			local vocationEx = getVocationInfo(getPlayerVocation(cid))
			if(vocationEx.id ~= vocation and vocationEx.fromVocation ~= vocation) then
				doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "Only the worthy may pass.")
				return true
			end

			return doorEnter(cid, item.uid, door.transformUseTo, toPosition)
		end

		if(item.aid == 190 or (item.aid ~= 0 and getPlayerLevel(cid) >= (item.aid - door.levelDoor))) then
			return doorEnter(cid, item.uid, door.transformUseTo, toPosition)
		end

		doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "Only the worthy may pass.")
		return true
	end

	if(door.specialDoor) then
		if(item.aid == 100 or (item.aid ~= 0 and getCreatureStorage(cid, item.aid) > 0)) then
			return doorEnter(cid, item.uid, door.transformUseTo, toPosition)
		end

		doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "The door seems to be sealed against unwanted intruders.")
		return true
	end

	toPosition.stackpos = STACKPOS_TOP_MOVEABLE_ITEM_OR_CREATURE
	local fields, thing = getTileItemsByType(fromPosition, ITEM_TYPE_MAGICFIELD), getThingFromPosition(toPosition)
	if(item.uid ~= thing.uid and thing.itemid >= 100 and table.maxn(fields) ~= 0) then
		return true
	end

	local doorCreature = getThingFromPosition(toPosition)
	if(doorCreature.itemid ~= 0) then
		toPosition.x = toPosition.x + 1
		local query = doTileQueryAdd(doorCreature.uid, toPosition, 20) -- allow to stack outside doors, but not on teleports or floor changing tiles
		if(query == RETURNVALUE_NOTPOSSIBLE) then
			toPosition.x = toPosition.x - 1
			toPosition.y = toPosition.y + 1
			query = doTileQueryAdd(doorCreature.uid, toPosition, 20) -- repeat until found
		end

		if(query ~= RETURNVALUE_NOERROR) then
			doPlayerSendDefaultCancel(cid, query)
			return true
		end

		doTeleportThing(doorCreature.uid, toPosition)
		if(not door.closingDoor) then
			doTransformItem(item.uid, door.transformUseTo)
		end

		return true
	end

	return false
end