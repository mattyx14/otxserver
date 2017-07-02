local function isQuestChest(item)
	local itemid = item:getId()
	if itemid == 1738 or itemid == 1740 or (itemid >= 1746 and itemid <= 1749) or (itemid >= 12664 and itemid <= 12665) or (itemid >= 12796 and itemid <= 12797) then
		return true
	end
end

local function isWalkable(item)
	if item.uid > 0 and item.uid <= 65535 then
		return false
	end
	return true
end

local function isPositionSafe(position)
	local tile = Tile(position)
	for _, item in ipairs(tile:getItems()) do
		if isQuestChest(item) and not isWalkable(item) then
			return false
		end
	end
	return true
end

function onStepIn(creature, item, position, fromPosition)
	if not isWalkable(item) then
		if creature:isPlayer() then
			local safePosition = creature:getTown():getTemplePosition()

			if position.x == fromPosition.x and position.y == fromPosition.y and position.z == fromPosition.z then
				creature:teleportTo(safePosition, false)
				return true
			elseif not isPositionSafe(fromPosition) then
				creature:teleportTo(safePosition, false)
				return true
			end
		end

		creature:teleportTo(fromPosition, false)
	end
	return true
end
