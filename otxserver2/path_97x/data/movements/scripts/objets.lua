function onStepIn(cid, item, pos)
	doCreatureSay(cid, "Crashhss", TALKTYPE_MONSTER_SAY)
	doSendMagicEffect(pos, CONST_ME_POFF)
	return true
end