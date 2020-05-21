function onThink(cid, interval)
	if(not isCreature(cid)) then
		return
	end

	local now = os.time()
	local skull, skullEnd = getCreatureSkull(cid), getPlayerSkullEnd(cid)
	if(skullEnd > 0 and skull > SKULL_WHITE and now > skullEnd and not getCreatureCondition(cid, CONDITION_INFIGHT)) then
		doPlayerSetSkullEnd(cid, 0, skull)
	end

	local save = getCreatureStorage(cid, "save")
	if(now > save) then
		doPlayerSaveEx(cid)
	end
end
