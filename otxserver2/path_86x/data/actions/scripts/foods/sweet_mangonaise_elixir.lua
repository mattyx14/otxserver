local config = {
	amount = 10
}
function onUse(cid, item, fromPosition, itemEx, toPosition)
	local food = SPECIAL_FOODS[item.itemid]
	local ring = getPlayerSlotItem(cid, CONST_SLOT_RING)
	if(food == nil) then
		return false
	end

	if(ring.itemid == 0) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_SMALL, "You may want to equip a ring before eating this.")
		doSendMagicEffect(fromPosition, CONST_ME_POFF)
		return true
	end

	if(getItemInfo(ring.itemid).showDuration) then
		local capRequired, pFreeCap = (getItemInfo(ring.itemid).weight * config.amount), getPlayerFreeCap(cid)

		if(pFreeCap < capRequired) then
			doPlayerSendTextMessage(cid, MESSAGE_STATUS_SMALL, "You may want to free some capacity before doing this.")
			doSendMagicEffect(fromPosition, CONST_ME_POFF)
			return true
		end

		for i=1,config.amount do
			doPlayerAddItemEx(cid, doCopyItem(ring).uid, true)
		end
	else
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_SMALL, "You may want to equip the correct type of ring before eating this.")
		doSendMagicEffect(fromPosition, CONST_ME_POFF)
		return true
	end

	doRemoveItem(item.uid, 1)
	doCreatureSay(cid, food, TALKTYPE_MONSTER)
	return true
end
