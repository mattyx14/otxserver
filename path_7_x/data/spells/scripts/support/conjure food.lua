local FOODS = {
	2666, -- meat
	2671, -- ham
	2681, -- grape
	2674, -- aple
	2689, -- bread
	2690, -- roll
	2696 -- cheese
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
