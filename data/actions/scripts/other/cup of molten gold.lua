function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if target.itemid ~= 2700 then
		return false
	end

	item:remove(1)
	player:addItem(13539, 1)
	toPosition:sendMagicEffect(CONST_ME_EXPLOSIONAREA)
	return true
end