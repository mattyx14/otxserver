function onStepIn(cid, item)
	doTargetCombatHealth(0, cid, COMBAT_POISONDAMAGE, -30, -30, CONST_ME_POFF)
	doTransformItem(item.uid, item.itemid + 1)
	doDecayItem(item.uid)
	return true
end

function onAddItem(moveItem, tileItem, position)
	doTransformItem(tileItem.uid, tileItem.itemid + 1)
	doSendMagicEffect(position, CONST_ME_POFF)
	return true
end
