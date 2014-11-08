function onThink(cid, interval)
	if(not isCreature(cid)) then
		return
	end

	local skull, skullEnd = getCreatureSkull(cid), getPlayerSkullEnd(cid)
	if(skullEnd > 0 and skull > SKULL_WHITE and os.time() > skullEnd and not getCreatureCondition(cid, CONDITION_INFIGHT)) then
		doPlayerSetSkullEnd(cid, 0, skull)
	end
end
