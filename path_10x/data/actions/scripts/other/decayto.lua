local ITEM_IDS = {
	[2041] = 2042,
	[2042] = 2041,
	[2044] = 2045,
	[2045] = 2044,
	[2047] = 2048,
	[2048] = 2047,
	[2050] = 2051,
	[2051] = 2050,
	[2052] = 2053,
	[2053] = 2051,
	[2054] = 2055,
	[2055] = 2054,
	[5812] = 5813,
	[5813] = 5812,
	[7183] = 7184,
	[7184] = 7183,
	[9006] = 9007,
	[9007] = 9006,

	-- crystal pedestals
	[9976] = 9977,
	[9977] = 9978,
	[9978] = 9979,
	[9979] = 9976
}

function onUse(cid, item, fromPosition, itemEx, toPosition)
	if(not ITEM_IDS[item.itemid]) then
		return false
	end

	doTransformItem(item.uid, ITEM_IDS[item.itemid])
	doDecayItem(item.uid)
	return true
end
