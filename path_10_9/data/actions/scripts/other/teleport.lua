local upFloorIds = {1386, 3678, 5543, 8599, 10035, 13010, 22845, 22846}
local draw_well = 1369

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local tile = item:getTile()
	if tile and tile:hasFlag(TILESTATE_PROTECTIONZONE) and player:isPzLocked() then
		player:sendCancelMessage("You can not enter a protection zone after attacking another player.")
		return false
	end

	if item.itemid == draw_well and item.actionid ~= 100 then
		return false
	end

	if isInArray(upFloorIds, item.itemid) then
		fromPosition:moveUpstairs()
	else
		fromPosition.z = fromPosition.z + 1
	end
	player:teleportTo(fromPosition, false)
	return true
end
