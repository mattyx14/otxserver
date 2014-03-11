local FOODS = {
	2666,
	2671,
	2681,
	2674,
	2689,
	2690,
	2696
}

function onCastSpell(cid, var)
	local tmpPos =getThingPosition(cid)
	tmpPos.stackpos = -1
	local i, tmpItem, tileCount = 1, {uid = 1}, getTileThingByPos(tmpPos)
	while(tmpItem.uid ~= 0 and i < tileCount) do
		tmpPos.stackpos = i
		tmpItem = getTileThingByPos(tmpPos)
		if(tmpItem.uid ~= 0 and isMoveable(tmpItem.uid)) then
			return false
		else
			i = i + 1
		end
	end

	local size = table.maxn(FOODS)
	if(math.random(1, 100) > 50) then
		doPlayerAddItem(cid, FOODS[math.random(1, size)])
	end

	doSendMagicEffect(getThingPosition(cid), CONST_ME_MAGIC_GREEN)
	return true
end
