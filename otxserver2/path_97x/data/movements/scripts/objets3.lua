function onStepIn(cid, item, pos)
	doCreatureSay(cid, "Grrrr!", TALKTYPE_MONSTER_SAY)
	doSendMagicEffect(pos, CONST_ME_POFF)
	return true
end