local function doRemoveObject(cid, pos)
	pos.stackpos = 255
	local object = getThingFromPos(pos)
	if(object.uid > 65535 and not isCreature(object.uid) and isMovable(object.uid) and object.actionid == 0 and not getTileInfo(pos).protection) then
		doRemoveItem(object.uid)
		doSendMagicEffect(pos, CONST_ME_BLOCKHIT)
		return true
	end

	doPlayerSendDefaultCancel(cid, RETURNVALUE_NOTPOSSIBLE)
	doSendMagicEffect(getPlayerPosition(cid), CONST_ME_POFF)
	return false
end

function onCastSpell(cid, var)
	local pos = variantToPosition(var)
	if(pos.x == CONTAINER_POSITION) then
		pos = getThingPos(cid)
	end

	if(pos.x ~= 0 and pos.y ~= 0) then
		return doRemoveObject(cid, pos)
	end

	doPlayerSendDefaultCancel(cid, RETURNVALUE_NOTPOSSIBLE)
	doSendMagicEffect(getPlayerPosition(cid), CONST_ME_POFF)
	return false
end
