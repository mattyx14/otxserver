local bosses = {
	['demon'] = {
		message = "Escape through the teleport quickly before it closes!",
		teleportToPosition = Position(981, 1053, 10)
	},
	['zarabustor'] = {
		message = "Escape through the teleport quickly before it closes!",
		teleportToPosition = Position(1021, 189, 7)
	},
	['dracula'] = {
		message = "Escape through the teleport quickly before it closes!",
		teleportToPosition = Position(1543, 875, 5)
	},
	['fezarus'] = {
		message = "Escape through the teleport quickly before it closes!",
		teleportToPosition = Position(737, 809, 12)
	},
	['fyzarus'] = {
		message = "Escape through the teleport quickly before it closes!",
		teleportToPosition = Position(924, 1107, 12)
	},
	['toxirus'] = {
		message = "Escape through the teleport quickly before it closes!",
		teleportToPosition = Position(895, 987, 10)
	},
	['leaf golem grom'] = {
		message = "Escape through the teleport quickly before it closes!",
		teleportToPosition = Position(656, 1094, 12)
	},
	['black knight'] = {
		message = "Escape through the teleport quickly before it closes!",
		teleportToPosition = Position(1179, 1312, 8)
	},
	['massacre'] = {
		message = "Escape through the teleport quickly before it closes!",
		teleportToPosition = Position(1018, 1153, 10)
	},
	['black magician'] = {
		message = "Escape through the teleport quickly before it closes!",
		teleportToPosition = Position(891, 1175, 11)
	},
}

local function removeTeleport(position)
	local teleportItem = Tile(position):getItemById(1387)
	if teleportItem then
		teleportItem:remove()
		position:sendMagicEffect(CONST_ME_POFF)
	end
end

function onKill(creature, target)
	local targetMonster = target:getMonster()
	if not targetMonster then
		return true
	end

	local bossConfig = bosses[targetMonster:getName():lower()]
	if not bossConfig then
		return true
	end

	local position = targetMonster:getPosition()
	position:sendMagicEffect(CONST_ME_TELEPORT)
	local item = Game.createItem(1387, 1, position)
	if item:isTeleport() then
		item:setDestination(bossConfig.teleportToPosition)
	end
	targetMonster:say(bossConfig.message, TALKTYPE_MONSTER_SAY, 0, 0, position)

	--remove portal after 1 min
	addEvent(removeTeleport, 1 * 60 * 1000, position)
	return true
end