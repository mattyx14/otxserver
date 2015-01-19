function onUse(cid, item, fromPosition, itemEx, toPosition)
	local rand, effect = math.random(1, 100), CONST_ME_TELEPORT
	if((rand >= 50) and (rand < 83)) then
		doSummonCreature("Spider", fromPosition)
	elseif((rand >= 83) and (rand < 97)) then
		doSummonCreature("Poison Spider", fromPosition)
	elseif((rand >= 97) and (rand < 100)) then
		doSummonCreature("Tarantula", fromPosition)
	elseif(rand == 100) then
		doSummonCreature("Giant Spider", fromPosition)
	else
		effect = CONST_ME_POFF
	end

	doSendMagicEffect(fromPosition, effect)
	return false
end
