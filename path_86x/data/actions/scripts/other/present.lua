function onUse(cid, item, fromPosition, itemEx, toPosition)
	doSendMagicEffect(fromPosition, CONST_ME_EXPLOSIONAREA)
	doCreatureSay(cid, "KABOOOOOOOOOOM!", TALKTYPE_MONSTER)

	doRemoveItem(item.uid, 1)
	return true
end
