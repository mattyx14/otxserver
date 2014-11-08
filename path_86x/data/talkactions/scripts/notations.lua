function onSay(cid, words, param, channel)
	if(param == '') then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Command param required.")
		return true
	end

	local ret = getAccountIdByName(param)
	if(ret == 0) then
		ret = getAccountIdByAccount(param)
		if(ret == 0) then
			doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Player or account '" .. param .. "' does not exists.")
			return true
		end
	end

	local list = getBanList(BAN_NOTATION, ret)
	if(type(list) ~= 'table' or table.maxn(list) <= 0) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Such account or player does not have any notation.")
		return true
	end

	ret = "Notations for account " .. ret .. "\n"
	for i, ban in ipairs(list) do
		local tmp = ban.adminId ~= 0 and getPlayerNameByGUID(ban.adminId) or "unknown"
		ret = ret .. "\nAdded at " .. os.date("%c", ban.added) .. " by " .. tmp .. " with comment: " .. ban.comment .. "."
	end

	doPlayerPopupFYI(cid, ret)
	return true
end
