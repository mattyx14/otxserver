local config = {
	tibianTime = true,
	twentyFour = true -- only if tibianTime = false
}

function onUse(cid, item, fromPosition, itemEx, toPosition)
	local str = ""
	if(config.tibianTime) then
		local var = getTibiaTime()
		str = var.hours .. ':' .. var.minutes
	elseif(config.twentyFour) then
		str = os.date('%H:%M')
	else
		str = os.date('%I:%M %p')
	end

	doPlayerSendTextMessage(cid, MESSAGE_INFO_DESCR, "The time is " .. str .. ".")
	return true
end
