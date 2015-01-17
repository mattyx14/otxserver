local PUMPKIN_HEAD_LIGHT = 2097
local CANDLE = 2048
local KNIFE = 2566

function onUse(cid, item, fromPosition, itemEx, toPosition)
	if(item.itemid == PUMPKIN_HEAD and itemEx.itemid == CANDLE) then
		doTransformItem(item.uid, PUMPKIN_HEAD_LIGHT)
		doRemoveItem(itemEx.uid)

		doDecayItem(item.uid)
		return true
	end

	return item.itemid == KNIFE and TOOLS.KNIFE(cid, item, fromPosition, itemEx, toPosition)
end
