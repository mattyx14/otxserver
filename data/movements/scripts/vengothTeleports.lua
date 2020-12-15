local config = {
	[50220] = Position(1548, 977, 4),
	[50221] = Position(1540, 987, 3)
}

function onStepIn(creature, item, position, fromPosition)
	local player = creature:getPlayer()
	if not player then
		return true
	end

	local targetPosition = config[item.actionid]
	if not targetPosition then
		return true
	end

	player:teleportTo(targetPosition)
	targetPosition:sendMagicEffect(CONST_ME_ICETORNADO)
	return true
end