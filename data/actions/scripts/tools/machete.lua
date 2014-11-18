local JUNGLE_GRASS = { 2782, 3985, 19433 }
local WILD_GROWTH = { 1499, 11099 }

function onUse(player, item, fromPosition, itemEx, toPosition, isHotkey)
	local targetItem = Item(itemEx.uid)
	if isInArray(JUNGLE_GRASS, itemEx.itemid) then
		targetItem:transform(itemEx.itemid == 19433 and 19431 or itemEx.itemid - 1)
		targetItem:decay()
		return true
	end

	if isInArray(WILD_GROWTH, itemEx.itemid) then
		toPosition:sendMagicEffect(CONST_ME_POFF)
		targetItem:remove()
		return true
	end

	return destroyItem(player, itemEx, toPosition)
end
