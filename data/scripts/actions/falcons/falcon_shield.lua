local falconShield = Action()

function falconShield.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if not target or type(target) ~= 'userdata' or not target:isItem() then
		return false
	end

	if player:getItemCount(32521) < 1 or player:getItemCount(32518) < 1 then
		return false
	end

	if target:getId() ~= 8671 then
		return false
	end

	player:removeItem(32518, 1)
	player:addItem(32422, 1)
	item:remove(1)
	return true
end

falconShield:id(32421)
falconShield:register()
