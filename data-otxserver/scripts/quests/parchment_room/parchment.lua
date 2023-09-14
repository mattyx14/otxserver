local demonPositions = {
	{ x = 254, y = 676, z = 14 },
	{ x = 262, y = 676, z = 14 },
	{ x = 255, y = 682, z = 14 },
	{ x = 263, y = 681, z = 14 }
}

local function recreateParchment(position)
	local item = Tile(position):getItemById(2819)
	if item then
		item:setActionId(104)
	else
		local parchment = Game.createItem(2819, 1, position)
		if parchment then
			parchment:setText("Buried forever that he never shall return. Don't remove this seal or bad things may happen.")
			parchment:setActionId(104)
		end
	end
end

local parchment = MoveEvent()

function parchment.onRemoveItem(item, position)
	item:removeAttribute(ITEM_ATTRIBUTE_ACTIONID)
	addEvent(recreateParchment, 2 * 60 * 60 * 1000, position) -- 2 hours

	for i = 1, #demonPositions do
		Game.createMonster("Demon", demonPositions[i])
	end
	return true
end

parchment:aid(104)
parchment:register()
