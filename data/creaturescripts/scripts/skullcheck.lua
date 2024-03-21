function onThink(cid, interval)
	if(not isCreature(cid)) then
		return true
	end

	local now = os.time()
	local skull, skullEnd = getCreatureSkull(cid), getPlayerSkullEnd(cid)
	if(skullEnd > 0 and skull > SKULL_WHITE and now > skullEnd and not getCreatureCondition(cid, CONDITION_INFIGHT)) then
		doPlayerSetSkullEnd(cid, 0, skull)
		doPlayerSave(cid)
	end
	return true
end
