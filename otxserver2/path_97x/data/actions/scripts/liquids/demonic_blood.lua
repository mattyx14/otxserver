local POTIONS = {7588, 7589}

function onUse(cid, item, fromPosition, itemEx, toPosition)
	doTransformItem(item.uid, POTIONS[math.random(1, table.maxn(POTIONS))])
	doSendMagicEffect(fromPosition, CONST_ME_MAGIC_RED)
	return true
end
