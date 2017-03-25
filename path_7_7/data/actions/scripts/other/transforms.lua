local transformItems = {
	[3743] = 4404, [4404] = 3743
}

function onUse(player, item, fromPosition, target, toPosition)
	local transformIds = transformItems[item:getId()]
	if not transformIds then
		return false
	end

	item:transform(transformIds)
	return true
end
