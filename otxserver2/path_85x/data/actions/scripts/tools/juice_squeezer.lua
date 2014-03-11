local FRUIT = {2673, 2674, 2675, 2676, 2677, 2678, 2679, 2680, 2681, 2682, 2684, 2685, 5097, 8839, 8840, 8841}

function onUse(cid, item, fromPosition, itemEx, toPosition)
	if isInArray(FRUIT, itemEx.itemid) and doPlayerRemoveItem(cid, 2006, 1) then
		doPlayerRemoveItem(itemEx.uid, 1)
		doPlayerAddItem(cid, 2006, itemEx.itemid == 2678 and 14 or 21)
		return true
	end

	return true
end
