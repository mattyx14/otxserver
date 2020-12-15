local destination = {
	[24878] = { -- Lady Tenebris
		newPos = Position(1127, 1080, 15),
		backPos = Position(1095, 1080, 15),
		timer = Storage.ForgottenKnowledge.LadyTenebrisTimer,
	},
	[24879] = { -- Jaul
		newPos = Position(1133, 461, 14),
		backPos = Position(1138, 472, 14),
		timer = Storage.MisidiaQuest.JaulTimer
	},
	-- [24880] = {}, -- Reserved to lavaPortal
}

function onStepIn(creature, item, position, fromPosition)
	local player = creature:getPlayer()
	if not player then
		return
	end

	local teleport = destination[item.uid]
	if not teleport then
		return
	end

	if player:getExhaustion(teleport.timer) > 0 then
		player:getPosition():sendMagicEffect(CONST_ME_TELEPORT)
		player:teleportTo(teleport.backPos)
		player:getPosition():sendMagicEffect(CONST_ME_FIREAREA)
		player:say("You haven't permission to use this teleport.", TALKTYPE_MONSTER_SAY)
		return true
	end

	position:sendMagicEffect(CONST_ME_TELEPORT)
	player:teleportTo(teleport.newPos)
	player:getPosition():sendMagicEffect(CONST_ME_FIREAREA)
	return true
end
