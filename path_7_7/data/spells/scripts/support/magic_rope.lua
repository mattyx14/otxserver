local ROPE_SPOT = {384, 418}

function onCastSpell(cid, var)
	local pos = getCreaturePosition(cid)
	pos.stackpos = 0
	local grounditem = getThingfromPos(pos)
	if isInArray(ROPE_SPOT, grounditem.itemid) == true then
		local newpos = pos
        local oldpos = getCreaturePosition(cid)
		newpos.y = newpos.y + 1
		newpos.z = newpos.z - 1
		doTeleportThing(cid, newpos)
		doSendMagicEffect(newpos, CONST_ME_TELEPORT)
		return LUA_NO_ERROR
	end
	doPlayerSendDefaultCancel(cid, RETURNVALUE_NOTPOSSIBLE)
	doSendMagicEffect(pos, CONST_ME_POFF)
	return LUA_ERROR
end