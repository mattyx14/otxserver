local upFloorIds = {1386, 3678}
local draw_well = 1369

function onUse(player, item, fromPosition, target, toPosition)
	if item.itemid == draw_well and item.actionid ~= 100 then
		return false
	end

	if table.contains(upFloorIds, item.itemid) then
		fromPosition:moveUpstairs()
	else
		fromPosition.z = fromPosition.z + 1
	end

	if player:isPzLocked() and Tile(fromPosition):hasFlag(TILESTATE_PROTECTIONZONE) then
		player:sendCancelMessage(RETURNVALUE_PLAYERISPZLOCKED)
		return true
	end

	player:teleportTo(fromPosition, false)
	return true
end
