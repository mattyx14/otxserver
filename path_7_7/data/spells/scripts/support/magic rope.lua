local SPOTS = {384, 418, 8278, 8592}

function onCastSpell(cid, var)
	local position = getThingPosition(cid)
	position.stackpos = 0

	local ground = getThingFromPos(position)
	if(isInArray(SPOTS, ground.itemid)) then
		local newPosition = position
		newPosition.y = newPosition.y + 1
		newPosition.z = newPosition.z - 1

		doTeleportThing(cid, newPosition, false)
		doSendMagicEffect(position, CONST_ME_TELEPORT)
		return true
	else
		doPlayerSendDefaultCancel(cid, RETURNVALUE_NOTPOSSIBLE)
		doSendMagicEffect(position, CONST_ME_POFF)
		return false
	end
end
