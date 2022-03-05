local setting = {
	-- {fromPos = Position(32718, 31628, 7), toPos = Position(32736, 31639, 7), townId = TOWNS_LIST.AB_DENDRIEL},
	{fromPos = Position(937, 993, 7), toPos = Position(948, 1002, 7), townId = 1}, -- Fynn
	{fromPos = Position(694, 443, 5), toPos = Position(708, 454, 5), townId = 2}, -- Anshara
	{fromPos = Position(336, 1093, 8), toPos = Position(340, 1098, 8), townId = 3}, -- Sohan
	{fromPos = Position(308, 1286, 6), toPos = Position(316, 1296, 6), townId = 4}, -- Samaransa
	{fromPos = Position(373, 1400, 7), toPos = Position(381, 1412, 7), townId = 5}, -- Forgos
	{fromPos = Position(820, 1279, 7), toPos = Position(825, 1286, 7), townId = 6}, -- Agard
	{fromPos = Position(1064, 1216, 8), toPos = Position(1085, 1239, 8), townId = 7}, -- Mer Jungle
	{fromPos = Position(414, 856, 6), toPos = Position(426, 865, 6), townId = 8}, -- Vinor
	{fromPos = Position(1043, 680, 7), toPos = Position(1055, 691, 7), townId = 9}, -- Jorvik
	{fromPos = Position(1104, 312, 6), toPos = Position(1111, 323, 6), townId = 10}, -- Misidia
	{fromPos = Position(691, 1158, 6), toPos = Position(702, 1169, 6), townId = 11}, -- Elfic
	{fromPos = Position(1541, 666, 12), toPos = Position(1560, 678, 12), townId = 12}, -- Vaargdon
	{fromPos = Position(944, 1214, 8), toPos = Position(977, 1224, 8), townId = 13}, -- Hive Island
	{fromPos = Position(1270, 469, 14), toPos = Position(1297, 486, 14), townId = 14}, -- Sunken Sanctuary
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

	local destination = Position(926, 982, 3)
		player:teleportTo(destination)
		destination:sendMagicEffect(CONST_ME_TELEPORT)
	return true
end

adventurersStone:id(16277)
adventurersStone:register()
