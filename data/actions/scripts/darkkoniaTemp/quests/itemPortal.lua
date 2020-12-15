local config = {
	[39512] = Position(853, 1050, 10), -- Mirror Conjurer Outfit,
	[39513] = Position(1529, 910, 1), -- Zevelon Duskbringer
	[39514] = Position(1529, 892, 1), -- Sir Valorcrest
	[39515] = Position(1556, 910, 1), -- Diblis the Fair
	[39516] = Position(1556, 892, 1), -- Arachir the Ancient One
--	[39517] = Position(1542, 875, 1), -- Reserved to Dracula /actions/quest/outfits/portalDracula.lua
	[39518] = Position(618, 376, 8), -- Enter Portal Volcano
	[39519] = Position(627, 376, 7), -- Exit Portal Volcano
}

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local targetPosition = config[item.actionid]
	if not targetPosition then
		return true
	end

	player:getPosition():sendMagicEffect(CONST_ME_TELEPORT)
	player:teleportTo(targetPosition)
	return true
end
