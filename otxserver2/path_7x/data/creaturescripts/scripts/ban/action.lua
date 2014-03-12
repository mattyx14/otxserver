local ACCESS = {
	[1] = { 8 },
	[2] = { 1, 2, 4, 5, 7, 9 },
	[3] = { 1, 2, 3, 4, 5, 6, 7, 9 }
}

function onChannelRequest(cid, channel, custom)
	unregisterCreatureEvent(cid, "Ban_Action")
	if(not custom or type(channel) ~= 'number') then
		doPlayerSendCancel(cid, "Invalid action.")
		return false
	end

	if(not isInArray(ACCESS[getPlayerAccess(cid)], channel)) then
		doPlayerSendCancel(cid, "You cannot do this action.")
		return false
	end

	local output = "Name:\n\nComment:\n"
	if(isInArray({1, 5}, channel)) then
		output = "Name:\n\n(Optional) Length:\n\nComment:\n"
	end

	doShowTextDialog(cid, 2599, output, true, 1024)
	doCreatureSetStorage(cid, "banConfig", table.serialize({
		type = (channel > 4 and 2 or 1),
		subType = channel
	}))

	registerCreatureEvent(cid, "Ban_Finish")
	return false
end