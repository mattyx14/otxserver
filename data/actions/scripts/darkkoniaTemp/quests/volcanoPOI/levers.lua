local config = {
	[9011] = {
		rockID = 1354,
		portalID = 27726,
		stonePosition = Position(402, 233, 9), -- stone position
		teleportPosition = Position(402, 236, 9), -- teleport creation position
		destinationTeleport = Position(401, 247, 9) -- where the teleport takes you
	},
}

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local p = config[item.actionid]
	if not p then
		return true
	end

	if item.itemid == 1945 then
		local tile = p.stonePosition:getTile()
		if tile then
			local stone = tile:getItemById(p.rockID)
			if stone then
				stone:remove()
			end
		end

		local teleport = Game.createItem(p.portalID, 1, p.teleportPosition)
		if teleport then
			teleport:setDestination(p.destinationTeleport)
			p.teleportPosition:sendMagicEffect(CONST_ME_TELEPORT)
		end
	elseif item.itemid == 1946 then
		local tile = p.teleportPosition:getTile()
		if tile then
			local teleport = tile:getItemById(p.portalID)
			if teleport and teleport:isTeleport() then
				teleport:remove()
			end
		end
		p.teleportPosition:sendMagicEffect(CONST_ME_POFF)
		Game.createItem(p.rockID, 1, p.stonePosition)
	end
	return item:transform(item.itemid == 1945 and 1946 or 1945)
end
