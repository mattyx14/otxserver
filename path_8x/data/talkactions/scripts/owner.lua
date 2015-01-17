local NO_OWNER_PHRASE = {"none", "nobody", "0"}

function onSay(cid, words, param, channel)
	if(param == '') then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Command param required.")
		return true
	end

	local clean = true
	local t = string.explode(param, ",")
	if(t[2]) then
		clean = getBooleanFromString(t[2])
	end

	local name = tostring(t[1])
	if(not name) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Invalid name specified.")
		return true
	end

	local guid = 0
	if(not table.isStrIn(name:lower(), NO_OWNER_PHRASE)) then
		guid = getPlayerGUIDByName(name)
	end

	if(not guid) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Player not found.")
		return true
	end

	local hid = getHouseFromPos(getCreaturePosition(cid))
	if(not hid) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "You are not in a house.")
		return true
	end

	setHouseOwner(hid, guid, clean)
	return true
end
