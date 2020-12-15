local config = {
	bossName = 'Jaul',
	centerRoom = Position(1143, 558, 15),
	storage = Storage.HidenChest.deaplingOutfit,
	value = 1,
	range = 15, 
	timer = Storage.MisidiaQuest.JaulTimer,
	BossPosition = Position(1143, 559, 15),
	newPosition = Position(1149, 558, 15), -- Send Player to new Positions
}

local function clearJaul()
	local spectators = Game.getSpectators(config.centerRoom, false, false, 15, 15, 15, 15)
	for i = 1, #spectators do
		local spectator = spectators[i]
		if spectator:isPlayer() and spectator.uid == playerId then
			spectator:teleportTo(Position(1138, 472, 14)) -- Kick Potition
			spectator:getPosition():sendMagicEffect(CONST_ME_TELEPORT)
			spectator:say('Time out! You were teleported out by strange forces.', TALKTYPE_MONSTER_SAY)
		end

		if spectator:isMonster() then
			spectator:remove()
		end
	end
end

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if item.itemid == 9825 then
		if player:getPosition() ~= Position(1143, 456, 14) then -- Lever Main Position
			return true
		end
	end

	if item.itemid == 9825 then
		if player:getStorageValue(config.storage) < config.value then
			player:say('You don\'t have permission to use this lever', TALKTYPE_MONSTER_SAY)
			return true
		end

		if player:getExhaustion(config.timer) > 0 then
			player:say('You have to wait to challange this enemy again!', TALKTYPE_MONSTER_SAY)
			return true
		end

		if roomIsOccupied(config.BossPosition, config.range, config.range) then
			player:say('Someone is fighting against the boss! You need wait awhile.', TALKTYPE_MONSTER_SAY)
			return true
		end
		clearRoom(config.BossPosition, config.range, config.range, fromPosition)
		local monster = Game.createMonster(config.bossName, config.BossPosition, true, true)
		if not monster then
			return true
		end

		for x = 1141, 1145 do
			local playerTile = Tile(Position(x, 456, 14)):getTopCreature()
			if playerTile and playerTile:isPlayer() then
				playerTile:getPosition():sendMagicEffect(CONST_ME_POFF)
				playerTile:teleportTo(config.newPosition)
				playerTile:getPosition():sendMagicEffect(CONST_ME_TELEPORT)
				playerTile:say('You have twenty minutes to kill and loot this boss. Otherwise you will lose that chance and will be kicked out.', TALKTYPE_MONSTER_SAY)
				addEvent(clearJaul, 60 * 20 * 1000, player.uid, monster.uid, config.BossPosition, config.range, config.range, fromPosition)
				playerTile:setExhaustion(config.timer, 20 * 60 * 60)
			end
		end
		return true
	end
end
