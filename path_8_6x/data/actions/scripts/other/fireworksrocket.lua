function onUse(cid, item, fromPosition, itemEx, toPosition)
	if(fromPosition.x ~= CONTAINER_POSITION) then
		doSendMagicEffect(fromPosition, math.random(CONST_ME_FIREWORK_YELLOW, CONST_ME_FIREWORK_BLUE))
	else
		doCreatureSay(cid, "Ouch! Rather place it on the ground next time.", TALKTYPE_MONSTER)
		doTargetCombatHealth(0, cid, COMBAT_PHYSICALDAMAGE, -1, -1, CONST_ME_EXPLOSIONHIT)
	end

	doRemoveItem(cid, item.uid, 1)
	return true
end
