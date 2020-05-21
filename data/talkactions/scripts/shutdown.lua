shutdownEvent = 0

function onSay(cid, words, param, channel)
	if(param == '') then
		doSetGameState(GAMESTATE_SHUTDOWN)
		return true
	end

	params = string.explode(param, ",")
	local action, reason, mins  = "", "", 0
	if(not tonumber(params[1])) then
		action = string.trim(params[1]:lower())
	else
		mins = string.trim(params[1])
		if(table.maxn(params) > 1) then
			reason = params[2]
		end
	end

	if(action) then
		if(action == "cancel" or action == "stop") then
			if(shutdownEvent ~= 0) then
				stopEvent(shutdownEvent)
				shutdownEvent = 0
				doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "Shutdown cancelled.")
			else
				doPlayerSendTextMessage(cid, MESSAGE_STATUS_WARNING, "Server is not in the shutdown phase.")
			end
			return true
		elseif(action == "kill") then
			os.exit()
			return true
		end
	end

	mins = tonumber(mins)
	if(not mins or mins < 0) then
		doPlayerSendCancel(cid, "Numeric param may not be lower than 0.")
		return true
	end

	if(shutdownEvent ~= 0) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_WARNING, "Server is already in a shutdown state. To cancel shutdown use the \"/shutdown stop\" command.")
		return true
	end

	return prepareShutdown(math.abs(math.ceil(mins)), reason)
end

function prepareShutdown(minutes, reason)
	if(minutes <= 0) then
		doSetGameState(GAMESTATE_SHUTDOWN)
		return false
	end

	local change, r = 5, (reason ~= "" and " Reason: "..reason or "")
	if(minutes == 1) then
		doBroadcastMessage("Server is going down in " .. minutes .. " minute, please log out now!" .. r)
	elseif(minutes <= 5) then
		doBroadcastMessage("Server is going down in " .. minutes .. " minutes, please log out." .. r)
		change = 1
	else
		doBroadcastMessage("Server is going down in " .. minutes .. " minutes." .. r)
	end

	shutdownEvent = addEvent(prepareShutdown, (change * 60 * 1000), minutes - change, reason)
	return true
end
