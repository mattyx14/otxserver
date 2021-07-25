local gems = Action()

function gems.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if player:getItemCount(2147) >= 1 and target.itemid == 2342 then
		target:transform(2343)
		target:decay()
		item:remove(1)
		player:getPosition():sendMagicEffect(CONST_ME_MAGIC_RED)
		toPosition:sendMagicEffect(CONST_ME_MAGIC_RED)
		return true
	end
	return false
end

gems:id(9970)
gems:register()
