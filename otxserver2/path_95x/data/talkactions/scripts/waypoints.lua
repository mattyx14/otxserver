function onSay(cid, words, param, channel)
	if(param ~= '') then
		doWaypointAddTemporial(param, getCreaturePosition(cid))
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Temporial waypoint saved.")
		return true
	end

	local str = ""
	for i, waypoint in ipairs(getWaypointList()) do
		str = str .. waypoint.name .. "\n"
	end

	doShowTextDialog(cid, ITEM_ACTION_BOOK, str)
	return true
end
