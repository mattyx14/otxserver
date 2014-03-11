function onSay(cid, words, param, channel)
	local toPos = getCreatureLookPosition(cid)
	if(isInArray({"full", "all"}, param:lower())) then
		doCleanTile(toPos, false)
		doSendMagicEffect(toPos, CONST_ME_MAGIC_RED)
		return true
	end

	local amount = 1
	param = tonumber(param)
	if(param) then
		amount = param
	end

	toPos.stackpos = STACKPOS_TOP_MOVEABLE_ITEM_OR_CREATURE
	local tmp = getThingFromPos(toPos)
	if(tmp.uid ~= 0) then
		if(isCreature(tmp.uid) and (not isPlayer(tmp.uid) or (not isPlayerGhost(tmp.uid) or getPlayerGhostAccess(cid) >= getPlayerGhostAccess(tmp.uid)))) then
			doRemoveCreature(tmp.uid)
		else
			doRemoveItem(tmp.uid, math.min(math.max(1, tmp.type), amount))
		end

		doSendMagicEffect(toPos, CONST_ME_MAGIC_RED)
		return true
	end

	toPos.stackpos = STACKPOS_TOP_FIELD
	tmp = getThingFromPos(toPos)
	if(tmp.uid ~= 0) then
		doRemoveItem(tmp.uid, math.min(math.max(1, tmp.type), amount))
		doSendMagicEffect(toPos, CONST_ME_MAGIC_RED)
		return true
	end

	toPos.stackpos = STACKPOS_TOP_CREATURE
	tmp = getThingFromPos(toPos)
	if(tmp.uid ~= 0) then
		doRemoveCreature(tmp.uid)
		doSendMagicEffect(toPos, CONST_ME_MAGIC_RED)
		return true
	end

	for i = 5, 1, -1 do
		toPos.stackpos = i
		tmp = getThingFromPos(toPos)
		if(tmp.uid ~= 0) then
			if(isCreature(tmp.uid)) then
				doRemoveCreature(tmp.uid)
			else
				doRemoveItem(tmp.uid, math.min(math.max(1, tmp.type), amount))
			end

			doSendMagicEffect(toPos, CONST_ME_MAGIC_RED)
			return true
		end
	end

	doSendMagicEffect(getCreaturePosition(cid), CONST_ME_POFF)
	return true
end
