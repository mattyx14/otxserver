local cleanItemsInMap = getBooleanFromString(getConfigValue('cleanItemsInMap'))

function onAddItem(moveitem, tileitem, position, cid)
	if(cleanItemsInMap) then
		if not(tileitem) or not(moveitem) then
			return true
		end

		if(getTilePzInfo(position))then 
			return true
		end

		doRemoveItem(moveitem.uid, moveitem.type)
		return true
	end
end
