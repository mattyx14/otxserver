local function saveServer()
	cleanMap()
	broadcastMessage("Clean map completed and the next clean at on 2 hours.", MESSAGE_STATUS_CONSOLE_RED)
end

function onThink(interval)
	broadcastMessage("Cleaning map in 5 minutes.", MESSAGE_STATUS_WARNING)
	addEvent(saveServer, 5 * 60 * 1000)
	return true
end
