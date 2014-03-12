local outfit = {lookType = 267, lookHead = 0, lookBody = 0, lookLegs = 0, lookFeet = 0, lookTypeEx = 0, lookAddons = 0}

function onStepIn(cid, item, position, lastPosition, fromPosition, toPosition, actor)
	if(hasCondition(cid, CONDITION_OUTFIT, 0, CONDITIONID_COMBAT) and getCreatureOutfit(cid).lookType == outfit.lookType) then
		doRemoveCondition(cid, CONDITION_OUTFIT)
		if(not isPlayerGhost(cid)) then
			doSendMagicEffect(position, CONST_ME_POFF)
		end
	end

	return true
end

function onStepOut(cid, item, position, lastPosition, fromPosition, toPosition, actor)
	if(not isPlayer(cid)) then
		return true
	end

	local tmp = getTileInfo(toPosition)
	if(tmp.trashHolder) then
		if(doTileQueryAdd(cid, toPosition, 4) ~= RETURNVALUE_NOERROR) then
			return false
		end

		if(not isPlayerGhost(cid)) then
			doSendMagicEffect(fromPosition, CONST_ME_POFF)
			doSendMagicEffect(toPosition, CONST_ME_WATERSPLASH)
		end

		doRemoveConditions(cid, true)
		doSetCreatureOutfit(cid, outfit, -1)
	end

	return true
end
