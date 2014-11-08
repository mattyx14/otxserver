function onUse(cid, item, fromPosition, itemEx, toPosition)
	local random = math.random(1, 2)
	if(itemEx.itemid == 7528) then
		doTransformItem(itemEx.uid, 7529)
	elseif(itemEx.itemid == 7529) then
		if(random == 1) then
			doTransformItem(itemEx.uid, 7530)
		elseif(random == 2) then
			doTransformItem(itemEx.uid, 7531)
		end
	elseif(itemEx.itemid == 7530) then
		if(random == 1) then
			doTransformItem(itemEx.uid, 7532)
		elseif(random == 2) then
			doTransformItem(itemEx.uid, 7533)
		end
	elseif(itemEx.itemid == 7532) then
		if(random == 1) then
			doTransformItem(itemEx.uid, 7534)
		elseif(random == 2) then
			doTransformItem(itemEx.uid, 7535)
		end
	end

	return true
end
