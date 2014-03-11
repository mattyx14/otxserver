function onSay(cid, words, param, channel)
	if(param == '') then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Command param required.")
		return true
	end

	local account, tmp = getAccountIdByName(param), false
	if(account == 0) then
		account = getAccountIdByAccount(param)
		if(account == 0) then
			doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Player or account '" .. param .. "' does not exists.")
			return true
		end

		tmp = true
	end

	local ban = getBanData(account, BAN_ACCOUNT)
	if(ban and doRemoveAccountBanishment(account)) then
		local name = param
		if(tmp) then
			name = account
		end

		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, name .. " has been " .. (ban.expires == -1 and "undeleted" or "unbanned") .. ".")
	end

	if(tmp) then
		return true
	end

	tmp = getIpByName(param)
	if(isIpBanished(tmp) and doRemoveIpBanishment(tmp)) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "IP Banishment on " .. doConvertIntegerToIp(ip) .. " has been lifted.")
	end

	local guid = getPlayerGUIDByName(param, true)
	if(guid == nil) then
		return true
	end

	ban = getBanData(guid, BAN_PLAYER, PLAYERBAN_LOCK)
	if(ban and doRemovePlayerBanishment(guid, PLAYERBAN_LOCK)) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Namelock from " .. param .. " has been removed.")
	end

	ban = getBanData(guid, BAN_PLAYER, PLAYERBAN_BANISHMENT)
	if(ban and doRemovePlayerBanishment(guid, PLAYERBAN_BANISHMENT)) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, param .. " has been " .. (ban.expires == -1 and "undeleted" or "unbanned") .. ".")
	end

	return true
end
