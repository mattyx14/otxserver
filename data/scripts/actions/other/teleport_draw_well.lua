local teleportDrawWell = Action()

function teleportDrawWell.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if item:getActionId() ~= 100 then
		fromPosition.z = fromPosition.z + 1
		player:teleportTo(fromPosition, false)
	end
	return true
end

teleportDrawWell:id(1931)
teleportDrawWell:register()
