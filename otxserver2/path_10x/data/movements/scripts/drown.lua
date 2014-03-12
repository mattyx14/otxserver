local conditionDrown = createConditionObject(CONDITION_DROWN)
setConditionParam(conditionDrown, CONDITION_PARAM_PERIODICDAMAGE, -20)
setConditionParam(conditionDrown, CONDITION_PARAM_TICKS, -1)
setConditionParam(conditionDrown, CONDITION_PARAM_TICKINTERVAL, 2000)

function onStepIn(cid, item, position, fromPosition)
	if isPlayerGhost(cid) then
		return true
	end

	if(math.random(1, 10) == 1) then
		doSendMagicEffect(position, CONST_ME_BUBBLES)
	end

	if(not isPlayer(cid)) then
		return false
	end

	doAddCondition(cid, conditionDrown)
	return true
end

function onStepOut(cid, item, position, lastPosition, fromPosition, toPosition, actor)
	if(not isPlayer(cid)) then
		return false
	end

	local slotItem = getPlayerSlotItem(cid, CONST_SLOT_HEAD)
	if(slotItem.uid ~= 0 and slotItem.itemid == 12541) then
		toPosition.stackpos = 0
		local ground = getTileThingByPos(toPosition)
		if(ground.uid ~= 0 and not isInArray(underWater, ground.itemid)) then
			local itemInfo = getItemInfo(slotItem.itemid)
			doTransformItem(slotItem.uid, 5461)
		end
	end

	doRemoveCondition(cid, CONDITION_DROWN)
	return true
end
