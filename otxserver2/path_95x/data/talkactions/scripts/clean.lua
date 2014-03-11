local cleanEvent = 0

function onSay(cid, words, param, channel)
	if(param == '') then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Collected " .. doCleanMap() .. " items.")
		return true
	end

	local t = string.explode(param, ",")
	if(t[1] == 'tile') then
		local removeLoaded = false
		if(t[2]) then
			removeLoaded = getBooleanFromString(t[2])
		end

		doCleanTile(getCreaturePosition(cid), removeLoaded)
		return true
	end

	if(not tonumber(param)) then
		doPlayerSendCancel(cid, "Command numeric param required.")
		return true
	end

	stopEvent(cleanEvent)
	prepareClean(tonumber(param), cid)
	return true
end

function prepareClean(minutes, cid)
	if(minutes == 0) then
		if(isPlayer(cid)) then
			doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Cleaned " .. doCleanMap() .. " items.")
		end

		doBroadcastMessage("Game map cleaned.")
	elseif(minutes > 0) then
		if(minutes == 1) then
			doBroadcastMessage("Game map cleaning in " .. minutes .. " minute, please pick up all your items.")
		else
			doBroadcastMessage("Game map cleaning in " .. minutes .. " minutes.")
		end

		cleanEvent = addEvent(prepareClean, 60000, minutes - 1, cid)
	end
end
