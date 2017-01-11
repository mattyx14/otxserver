local upFloorIds = {1386, 3678, 5543, 8599, 10035, 13010, 22845, 22846}
local draw_well = 1369

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local t = Tile(fromPosition)
	if player:isPzLocked() and Tile(fromPosition):hasFlag(TILESTATE_PROTECTIONZONE) then
		player:sendTextMessage(MESSAGE_STATUS_SMALL, Game.getReturnMessage(RETURNVALUE_PLAYERISPZLOCKED))
		return true
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
