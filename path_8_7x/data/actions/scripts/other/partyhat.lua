function onUse(cid, item, fromPosition, itemEx, toPosition)
	if(item.uid ~= getPlayerSlotItem(cid, CONST_SLOT_HEAD).uid) then
		return false
	end

	doSendMagicEffect(getCreaturePosition(cid), CONST_ME_GIFT_WRAPS)
	return true
end
