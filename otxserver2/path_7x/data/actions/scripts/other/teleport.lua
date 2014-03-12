local UP_FLOORS = {1386, 3678}
local FIELDS = {1497, 1499}
local DRAW_WELL = 1369

function onUse(cid, item, fromPosition, itemEx, toPosition)
	if(item.itemid == DRAW_WELL and item.actionid ~= 100) then
		return false
	end

	local check = false
	fromPosition.stackpos = STACKPOS_GROUND
	if(isInArray(UP_FLOORS, item.itemid)) then
		fromPosition.z = fromPosition.z - 1
		fromPosition.y = fromPosition.y + 1
		if(doTileQueryAdd(cid, fromPosition, 38) ~= RETURNVALUE_NOERROR) then
			local field = getTileItemByType(fromPosition, ITEM_TYPE_MAGICFIELD)
			if(field.uid == 0 or not isInArray(FIELDS, field.itemid)) then
				fromPosition.y = fromPosition.y - 2
			else
				check = true
			end
		end
	else
		fromPosition.z = fromPosition.z + 1
	end

	if(not check and doTileQueryAdd(cid, fromPosition, 38) ~= RETURNVALUE_NOERROR) then
		local field = getTileItemByType(fromPosition, ITEM_TYPE_MAGICFIELD)
		if(field.uid == 0 or not isInArray(FIELDS, field.itemid)) then
			return false
		end
	end

	local pos, dir = getCreaturePosition(cid), SOUTH
	if(pos.x < fromPosition.x) then
		dir = EAST
	elseif(pos.x == fromPosition.x) then
		if(pos.y == fromPosition.y) then
			dir = getCreatureLookDirection(cid)
		elseif(pos.y > fromPosition.y) then
			dir = NORTH
		end
	elseif(pos.x > fromPosition.x) then
		dir = WEST
	end

	doTeleportThing(cid, fromPosition, false)
	doCreatureSetLookDirection(cid, dir)
	return true
end
