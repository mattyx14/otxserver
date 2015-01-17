TILE_SNOW = 670
TILE_FOOTPRINT_I = 6594
TILE_FOOTPRINT_II = 6598

function onStepIn(cid, item, position, lastPosition, fromPosition, toPosition, actor)
	if(isPlayerGhost(cid)) then
		return true
	end

	if(item.itemid == TILE_SNOW) then
		doTransformItem(item.uid, TILE_FOOTPRINT_I)
		doDecayItem(item.uid)
	elseif(item.itemid == TILE_FOOTPRINT_I) then
		doTransformItem(item.uid, TILE_FOOTPRINT_II)
		doDecayItem(item.uid)
	else
		doTransformItem(item.uid, TILE_FOOTPRINT_I)
	end

	return true
end
