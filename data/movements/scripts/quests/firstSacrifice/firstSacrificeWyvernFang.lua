local config = {
    [9006] = {
		sacrificePosition = Position(969, 1035, 7),
		pushPosition = Position(973, 1033, 7),
		destination = Position(973, 1034, 6)
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
    if not isInArray({7408}, sacrificeId) then
        sacrifice = false
    end

    if not sacrifice then
        player:teleportTo(flame.pushPosition)
        position:sendMagicEffect(CONST_ME_ENERGYHIT)
        flame.pushPosition:sendMagicEffect(CONST_ME_ENERGYHIT)
        return true
    end

    local wyvernFang = Tile(flame.sacrificePosition):getItemById(sacrificeId)
    if wyvernFang then
        wyvernFang:remove()
		player:setStorageValue(Storage.FirstSacrifice.noSacrifice.fangWyvern, 1)
    end

    player:teleportTo(flame.destination)
    position:sendMagicEffect(CONST_ME_HITBYFIRE)
    flame.sacrificePosition:sendMagicEffect(CONST_ME_HITBYFIRE)
    flame.destination:sendMagicEffect(CONST_ME_HITBYFIRE)
    return true
end
