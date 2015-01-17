function onStepIn(cid, item, position, lastPosition, fromPosition, toPosition, actor)
	doCreatureSay(cid, "Faaart!", TALKTYPE_MONSTER)
	doSendMagicEffect(getCreaturePosition(cid), CONST_ME_POFF)
	return true
end
