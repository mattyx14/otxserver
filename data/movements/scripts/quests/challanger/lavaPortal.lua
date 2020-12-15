local destination = {
	[24880] = {position = Position(495, 172, 9), storage = Storage.AnsharaPOI.ritualInfernus}
}

function onStepIn(creature, item, position, fromPosition)
	if not creature or not creature:isPlayer() then
		return true
	end

	local teleport = destination[item.actionid]
	if not teleport then
		return
	end

	if creature:getStorageValue(teleport.storage) >= 1 then
		creature:teleportTo(teleport.position)
		creature:getPosition():sendMagicEffect(CONST_ME_FIREAREA)
	else
		creature:teleportTo(Position(495, 176, 9))
		creature:say("You haven't permission to use this teleport.", TALKTYPE_MONSTER_SAY)
		creature:getPosition():sendMagicEffect(CONST_ME_FIREAREA)
	end
	return true
end
