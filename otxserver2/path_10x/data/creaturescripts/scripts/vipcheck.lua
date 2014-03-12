function onLogin(cid)
	if getPlayerVipDays(cid) >= 1 then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "You have ".. getPlayerVipDays(cid) .." vip days left.")
	end

	return true
end
