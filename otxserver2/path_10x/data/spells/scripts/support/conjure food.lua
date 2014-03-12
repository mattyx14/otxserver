local FOODS = {
	ITEM_FOOD_MEAT,
	ITEM_HAM,
	ITEM_FOOD_GRAPE,
	ITEM_FOOD_APLE,
	ITEM_FOOD_BREAD,
	ITEM_FOOD_ROOL,
	ITEM_FOOD_CHEESE
}

function onCastSpell(cid, var)
	local size = table.maxn(FOODS)
	if(not doPlayerAddItem(cid, FOODS[math.random(1, size)])) then
		doPlayerSendDefaultCancel(cid, RETURNVALUE_NOTPOSSIBLE)
		doSendMagicEffect(getThingPosition(cid), CONST_ME_POFF)
		return false
	end

	if(math.random(1, 100) > 50) then
		doPlayerAddItem(cid, FOODS[math.random(1, size)])
	end

	doSendMagicEffect(getThingPosition(cid), CONST_ME_MAGIC_GREEN)
	return true
end
