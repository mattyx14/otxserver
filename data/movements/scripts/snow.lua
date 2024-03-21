TILE_SNOW = 670
TILE_FOOTPRINT_I = 6594
TILE_FOOTPRINT_II = 6598

function onStepIn(cid, item, position, lastPosition, fromPosition, toPosition, actor)
	if(isPlayerGhost(cid)) then
		return true
	end

	if(item.itemid == TILE_SNOW) then
		doDecayItem(doCreateItem(TILE_FOOTPRINT_I, position))
	elseif(item.itemid == TILE_FOOTPRINT_I) then
		doTransformItem(item.uid, TILE_FOOTPRINT_II)
		doDecayItem(item.uid)
	else
		doDecayItem(doCreateItem(TILE_FOOTPRINT_I, position))
	end

	return true
end
