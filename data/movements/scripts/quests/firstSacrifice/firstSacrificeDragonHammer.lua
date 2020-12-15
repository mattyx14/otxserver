local config = {
    [9008] = {
		sacrificePosition = Position(569, 1254, 6),
		pushPosition = Position(562, 1266, 6),
		destination = Position(631, 1248, 6)
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
    if not isInArray({2434}, sacrificeId) then
        sacrifice = false
    end

    if not sacrifice then
        player:teleportTo(flame.pushPosition)
        position:sendMagicEffect(CONST_ME_ENERGYHIT)
        flame.pushPosition:sendMagicEffect(CONST_ME_ENERGYHIT)
        return true
    end

    local dragonHammer = Tile(flame.sacrificePosition):getItemById(sacrificeId)
    if dragonHammer then
        dragonHammer:remove()
		player:setStorageValue(Storage.FirstSacrifice.noSacrifice.hammerDragon, 1)
    end

    player:teleportTo(flame.destination)
    position:sendMagicEffect(CONST_ME_HITBYFIRE)
    flame.sacrificePosition:sendMagicEffect(CONST_ME_HITBYFIRE)
    flame.destination:sendMagicEffect(CONST_ME_HITBYFIRE)
    return true
end
