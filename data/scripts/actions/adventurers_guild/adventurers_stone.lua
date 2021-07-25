local setting = {
	-- 
	-- {fromPos = Position(32718, 31628, 7), toPos = Position(32736, 31639, 7), townId = TOWNS_LIST.AB_DENDRIEL},
}

local adventurersStone = Action()

function adventurersStone.onUse(player, item, fromPosition, target, toPosition, isHotkey)

	local playerPos, isInTemple, temple, townId = player:getPosition(), false
	for i = 1, #setting do
		temple = setting[i]
		if isInRange(playerPos, temple.fromPos, temple.toPos) then
			if Tile(playerPos):hasFlag(TILESTATE_PROTECTIONZONE) then
				isInTemple, townId = true, temple.townId
				break
			end
		end
	end

	if not isInTemple then
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, 'Try to move more to the center of a temple to use the spiritual energy for a teleport.')
		return true
	end

	player:setStorageValue(Storage.AdventurersGuild.Stone, townId)
	playerPos:sendMagicEffect(CONST_ME_TELEPORT)

	local destination = Position(32210, 32300, 6)
		player:teleportTo(destination)
		destination:sendMagicEffect(CONST_ME_TELEPORT)
	return true
end

adventurersStone:id(18559)
adventurersStone:register()
