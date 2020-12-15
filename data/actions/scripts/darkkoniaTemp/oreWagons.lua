local config = {
	[50140] = Position(1545, 672, 12), -- Temple
	[50141] = Position(1503, 704, 7), -- Enter City
	[50142] = Position(1475, 688, 11), -- Depot
	[50143] = Position(574, 1175, 9), -- Elfic Ville Cave
	[50144] = Position(1580, 743, 7), -- Forgotten Town
	[50145] = Position(1513, 758, 10), -- Varkhal Route
}

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local targetPosition = config[item.actionid]
	if not targetPosition then
		return true
	end

	if player:getStorageValue(Storage.HidenChest.hiddenCity.OreWagon) == 1 then
		player:teleportTo(targetPosition)
		player:getPosition():sendMagicEffect(CONST_ME_TELEPORT)
	elseif player:getStorageValue(Storage.HidenChest.hiddenCity.OreWagon) < 1 then
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You don't know how to use this yet. Need find to secret letter.")
	end
	return
end
