local DOLLS = {
	[5080] = {"Hug me."},
}

function onUse(cid, item, fromPosition, itemEx, toPosition)
	local doll = DOLLS[item.itemid]
	if(doll == nil) then
		return false
	end

	if(fromPosition.x == CONTAINER_POSITION) then
		fromPosition = getThingPosition(cid)
	end

	doCreatureSay(cid, doll[math.random(1, table.maxn(doll))], TALKTYPE_MONSTER, false, 0, fromPosition)
	return true
end
