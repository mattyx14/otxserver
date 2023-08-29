local setting = {
	{ fromPos = Position(92, 111, 7), toPos = Position(99, 121, 7), townId = TOWNS_LIST.TREKOLT },
	{ fromPos = Position(157, 385, 6), toPos = Position(166, 390, 6), townId = TOWNS_LIST.RHYVES },
	{ fromPos = Position(237, 426, 12), toPos = Position(246, 436, 12), townId = TOWNS_LIST.VARAK },
	{ fromPos = Position(491, 166, 7), toPos = Position(503, 177, 7), townId = TOWNS_LIST.JORVIK },
	{ fromPos = Position(235, 562, 7), toPos = Position(245, 569, 7), townId = TOWNS_LIST.SAUND }
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

adventurersStone:id(16277)
adventurersStone:register()
