local config = {
	{from = 1, to = 1644, itemId = 18394},
	{from = 1645, to = 3189, itemId = 2158},
	{from = 3190, to = 4725, itemId = 18391},
	{from = 4726, to = 6225, itemId = 18414, count = 5},
	{from = 6226, to = 7672, itemId = 18418, count = 10},
	{from = 7673, to = 9083, itemId = 18413, count = 10},
	{from = 9084, to = 9577, itemId = 2445},
	{from = 9578, to = 9873, itemId = 8878},
	{from = 9874, to = 9999, itemId = 18450}
}

function onUse(player, item, fromPosition, itemEx, toPosition, isHotkey)
	if itemEx.itemid ~= 18396 then
		return false
	end

	local chance = math.random(9999)
	for i = 1, #config do
		local randomItem = config[i]
		if chance >= randomItem.from and chance <= randomItem.to then
			local targetItem = Item(itemEx.uid)
			targetItem:getPosition():sendMagicEffect(CONST_ME_GREEN_RINGS)
			targetItem:remove(1)
			Item(item.uid):remove(1)
			player:addItem(randomItem.itemId, randomItem.count or 1)
			break
		end
	end
	return true
end
