local worlds = {
	[WORLDTYPE_OPTIONAL] = "Optional PvP",
	[WORLDTYPE_OPEN] = "Open PvP",
	[WORLDTYPE_HARDCORE] = "Hardcore PvP"
}

function onSay(cid, words, param, channel)
	local world = worlds[getWorldType()]
	if(not world) then
		return true
	end

	doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "World type is currently set to " .. world .. ".")
	return true
end
