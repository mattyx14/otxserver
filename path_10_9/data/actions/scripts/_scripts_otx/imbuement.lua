function onUse(cid, item, fromPosition, itemEx, toPosition)
	local player = Player(cid)
	if (not player) then
		return false
	end

	player:openImbuementWindow(itemEx)
	return true
end
