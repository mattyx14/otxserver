function onStepIn(cid, item, pos)
	doCreatureSay(cid, "Chosfffs", TALKTYPE_MONSTER_SAY)
	doSendMagicEffect(pos, CONST_ME_POFF)
	return true
end