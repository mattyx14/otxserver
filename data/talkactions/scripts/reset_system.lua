function onSay(cid, words, param, channel)
	if getConfigValue('resetSystemEnable') ~= true then
		return doPlayerSendTextMessage(cid, MESSAGE_STATUS_WARNING, "[Reset System]: Error! This system are disabled.")
	end

	local position = getThingPosition(cid)
	local tile = getTileInfo(position)
	if (not tile or not tile.protection) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_SMALL, "You can only use this command in protection zone.")
		doSendMagicEffect(position, CONST_ME_POFF)
		return true
	end

	local resetsCount = ResetSystem:getCount(cid)
	if (resetsCount >= #ResetSystem.resets) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_SMALL, "You have already reached the maximum level of resets.")
		doSendMagicEffect(position, CONST_ME_POFF)
		return true
	end

	local resetInfo = ResetSystem.resets[resetsCount + 1]
	if (not resetInfo) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_SMALL, "It is not possible to reset.")
		doSendMagicEffect(position, CONST_ME_POFF)
		return true
	end

	if (resetInfo.needed_level > getPlayerLevel(cid)) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_SMALL, "You need to be level " .. resetInfo.needed_level .. " or greater.")
		doSendMagicEffect(position, CONST_ME_POFF)
		return true
	end

	ResetSystem:execute(cid)
	doSendMagicEffect(position, CONST_ME_MAGIC_BLUE)
	doPlayerSave(cid)
	return true
end
