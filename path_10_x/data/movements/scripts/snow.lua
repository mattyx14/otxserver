TILE_SNOW = 670
TILE_FOOTPRINT_I = 6594

function onStepOut(cid, item, position, fromPosition)
	if(isPlayerGhost(cid)) == false then
		if(item.itemid == TILE_SNOW) then
			doDecayItem(doCreateItem(TILE_FOOTPRINT_I, fromPosition))
		else
			doDecayItem(doCreateItem(item.itemid + 15, fromPosition))
		end
	end

	return true
end
