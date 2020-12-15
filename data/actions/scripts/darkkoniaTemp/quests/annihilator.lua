local config = {
	requiredLevel = 270,
	centerRoomPosition = Position(1231, 1048, 10),
	playerPositions = {
		Position(973, 1068, 9),
		Position(974, 1068, 9),
		Position(975, 1068, 9),
		Position(976, 1068, 9)
	},
	newPositions = {
		Position(1229, 1048, 10),
		Position(1230, 1048, 10),
		Position(1231, 1048, 10),
		Position(1232, 1048, 10)
	},
	monsterPositions = {
		Position(1229, 1046, 10),
		Position(1231, 1046, 10),
		Position(1230, 1050, 10),
		Position(1232, 1050, 10),
		Position(1233, 1048, 10),
		Position(1234, 1048, 10)
	}
}

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if player:getStorageValue(Storage.VampireQuest.draculaDone) == 1 and
		player:getStorageValue(Storage.Missions.DjinnMission.Done) == 1 and
		player:getStorageValue(Storage.AnniQuest.justiceSeekerDone) == 1 and
		player:getStorageValue(Storage.AnniQuest.blessedSceptreDone) == 1 and
		player:getStorageValue(Storage.AnniQuest.royalAxeDone) == 1 and
		player:getStorageValue(Storage.AnniQuest.pirateDone) == 1 and
		player:getStorageValue(Storage.AnniQuest.conjurerDone) == 1 and
		player:getStorageValue(Storage.AnniQuest.deathHeraldDone) == 1 and
		player:getStorageValue(Storage.AnniQuest.assassinDone) == 1 then
		player:sendTextMessage(MESSAGE_STATUS_SMALL, Game.getReturnMessage(RETURNVALUE_NOTPOSSIBLE))
		return false
	end

	if item.itemid == 1946 then
		local storePlayers, playerTile = {}

		for i = 1, #config.playerPositions do
			playerTile = Tile(config.playerPositions[i]):getTopCreature()
			if not playerTile or not playerTile:isPlayer() then
				player:sendTextMessage(MESSAGE_STATUS_SMALL, "You need 4 players.")
				return true
			end

			if playerTile:getLevel() < config.requiredLevel then
				player:sendTextMessage(MESSAGE_STATUS_SMALL, "All the players need to be level ".. config.requiredLevel .." or higher.")
				return true
			end

			storePlayers[#storePlayers + 1] = playerTile
		end

		local specs, spec = Game.getSpectators(config.centerRoomPosition, false, false, 3, 3, 2, 2)
		for i = 1, #specs do
			spec = specs[i]
			if spec:isPlayer() then
				player:sendTextMessage(MESSAGE_STATUS_SMALL, "A team is already inside the quest room.")
				return true
			end

			spec:remove()
		end

		for i = 1, #config.monsterPositions do
			Game.createMonster("Sokay Guardian", config.monsterPositions[i])
		end

		local players
		for i = 1, #storePlayers do
			players = storePlayers[i]
			config.playerPositions[i]:sendMagicEffect(CONST_ME_POFF)
			players:teleportTo(config.newPositions[i])
			config.newPositions[i]:sendMagicEffect(CONST_ME_ENERGYAREA)
			players:setDirection(DIRECTION_EAST)
		end
	end

	item:transform(item.itemid == 1946 and 1945 or 1946)
	return true
end
