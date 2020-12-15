local config = {
    [9007] = {
		sacrificePosition = Position(976, 949, 8),
		pushPosition = Position(977, 945, 8),
		destination = Position(1034, 913, 8)
	}
}

function onStepIn(creature, item, position, fromPosition)
    local player = creature:getPlayer()
    if not player then
        return true
    end

    local flame = config[item.actionid]
    if not flame then
        return true
    end

    local sacrificeId, sacrifice = Tile(flame.sacrificePosition):getThing(1).itemid, true
    if not isInArray({2430}, sacrificeId) then
        sacrifice = false
    end

    if not sacrifice then
        player:teleportTo(flame.pushPosition)
        position:sendMagicEffect(CONST_ME_ENERGYHIT)
        flame.pushPosition:sendMagicEffect(CONST_ME_ENERGYHIT)
        return true
    end

    local knightAxe = Tile(flame.sacrificePosition):getItemById(sacrificeId)
    if knightAxe then
        knightAxe:remove()
		player:setStorageValue(Storage.FirstSacrifice.noSacrifice.axeKnight, 1)
    end

    player:teleportTo(flame.destination)
    position:sendMagicEffect(CONST_ME_HITBYFIRE)
    flame.sacrificePosition:sendMagicEffect(CONST_ME_HITBYFIRE)
    flame.destination:sendMagicEffect(CONST_ME_HITBYFIRE)
    return true
end
