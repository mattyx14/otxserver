function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if item.itemid == ITEM_GOLD_COIN and item.type == ITEMCOUNT_MAX then
		item:remove()
		player:addItem(ITEM_PLATINUM_COIN, 1)
	elseif item.itemid == ITEM_PLATINUM_COIN and item.type == ITEMCOUNT_MAX then
		item:remove()
		player:addItem(ITEM_CRYSTAL_COIN, 1)
	elseif item.itemid == ITEM_PLATINUM_COIN and item.type < ITEMCOUNT_MAX then
		item:transform(item.itemid, item.type - 1)
		player:addItem(ITEM_GOLD_COIN, ITEMCOUNT_MAX)
	elseif item.itemid == ITEM_CRYSTAL_COIN then
		item:transform(item.itemid, item.type - 1)
		player:addItem(ITEM_PLATINUM_COIN, ITEMCOUNT_MAX)
	else
		return false
	end

	return true
end
