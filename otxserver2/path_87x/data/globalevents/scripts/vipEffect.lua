function onThink(interval, lastExecution)
	for _, name in ipairs(getOnlinePlayers()) do
	local cid = getPlayerByName(name)
		if getPlayerVipDays(cid) >= 1 then
			doSendMagicEffect(getPlayerPosition(cid), CONST_ME_GIFT_WRAPS)
			doSendAnimatedText(getPlayerPosition(cid), TEXTCOLOR_RED, "VIP!")
		end
	end

	return true
end
