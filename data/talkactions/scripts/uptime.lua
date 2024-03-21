function onSay(cid, words, param, channel)
	if(not checkExhausted(cid, 666, 10)) then
		return true
	end

	local tmp = getWorldUpTime()
	local hours = math.ceil(tmp / 3600) - 1
	local minutes = math.ceil((tmp - (3600 * hours)) / 60)
	if(minutes == 60) then
		minutes = 0
		hours = hours + 1
	end

	doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Uptime: " .. hours .. " hours and " .. minutes .. " minutes.")
	return true
end
