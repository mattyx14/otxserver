local JUNGLE_GRASS = { 2782, 3985, 19433 }
local WILD_GROWTH = { 1499, 11099 }

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if isInArray(JUNGLE_GRASS, target.itemid) then
		target:transform(target.itemid == 19433 and 19431 or target.itemid - 1)
		target:decay()
		return true
	end

	if isInArray(WILD_GROWTH, target.itemid) then
		toPosition:sendMagicEffect(CONST_ME_POFF)
		target:remove()
		return true
	end

	return destroyItem(player, target, toPosition)
end
