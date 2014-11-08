function onStepIn(cid, item, pos)
	if(item.itemid == 2579) then
		if(not isPlayer(cid)) then
			doTargetCombatHealth(0, cid, COMBAT_PHYSICALDAMAGE, -15, -30, CONST_ME_NONE)
			doTransformItem(item.uid, item.itemid - 1)
		end
	else
		if(isPlayer(cid)) then
			doTargetCombatHealth(0, cid, COMBAT_PHYSICALDAMAGE, -50, -100, CONST_ME_NONE)
			doTransformItem(item.uid, item.itemid + 1)
		end
	end
	return true
end

function onStepOut(cid, item, pos)
	doTransformItem(item.uid, item.itemid - 1)
	return true
end

function onRemoveItem(item, tile, pos)
	local thingPos = getThingPos(item.uid)
	if(getDistanceBetween(thingPos, pos) > 0) then
		doTransformItem(item.uid, item.itemid - 1)
		doSendMagicEffect(thingPos, CONST_ME_POFF)
	end
	return true
end
