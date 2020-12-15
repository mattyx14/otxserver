local config = {
	BossPosition = Position(382, 356, 9),
	SoulPosition = Position(381, 357, 11),
	centerRoom = Position(382, 357, 9),
	storage = Storage.demonHelmetQuest,
	value = 1,
	range = 17, 
	timer = Storage.ForgottenKnowledge.DragonkingTimer,
	newPosition = Position(382, 357, 9), -- Send Player to new Positions
}

local monsters = {
	{position = Position(376, 351, 9)},
	{position = Position(386, 360, 9)},
	{position = Position(387, 349, 9)},
	{position = Position(377, 364, 9)},
}

local function clearDragon()
	local spectators = Game.getSpectators(config.centerRoom, false, false, 17, 17, 17, 17)
	for i = 1, #spectators do
		local spectator = spectators[i]
		if spectator:isPlayer() and spectator.uid == playerId then
			spectator:teleportTo(Position(622, 372, 8)) -- Kick Potition
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
		if player:getPosition() ~= Position(608, 387, 8) then -- Lever Main Position
			return true
		end
	end

	if item.itemid == 9825 then
		if player:getStorageValue(config.storage) < config.value then
			player:say('You don\'t have permission to use this lever require Demon Helmet Quest.', TALKTYPE_MONSTER_SAY)
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

		for d = 1, #monsters do
			Game.createMonster('soulcatcher', monsters[d].position, true, true)
		end
		Game.createMonster("dragonking zyrtarch", config.BossPosition, true, true)
		Game.createMonster("soul of dragonking zyrtarch", config.SoulPosition, true, true)

		for x = 606, 610 do
			local playerTile = Tile(Position(x, 387, 8)):getTopCreature()
			if playerTile and playerTile:isPlayer() then
				playerTile:teleportTo(config.newPosition)
				playerTile:getPosition():sendMagicEffect(CONST_ME_TELEPORT)
				playerTile:say('You have thirty minutes to kill and loot this boss. Otherwise you will lose that chance and will be kicked out.', TALKTYPE_MONSTER_SAY)
				addEvent(clearDragon, 60 * 30 * 1000, player.uid, config.BossPosition, config.SoulPosition, config.range, config.range, fromPosition)
				playerTile:setExhaustion(config.timer, 20 * 60 * 60)
			end
		end

		return true
	end
end
