local shutdownAtServerSave = false
local cleanMapAtServerSave = false

local function serverSave()
	if shutdownAtServerSave then
		Game.setGameState(GAME_STATE_SHUTDOWN)
	else
		Game.setGameState(GAME_STATE_CLOSED)

		if cleanMapAtServerSave then
			cleanMap()
		end

		Game.setGameState(GAME_STATE_NORMAL)
	end
end

local function firstServerSaveWarning()
	broadcastMessage("Server is saving game in one minute. Please go to a safe place.", MESSAGE_STATUS_WARNING)
	addEvent(serverSave, 60000)
end

function onThink(interval)
	broadcastMessage("Server is saving game in 3 minutes. Please go to a safe place.", MESSAGE_STATUS_WARNING)
	Game.setGameState(GAME_STATE_STARTUP)
	addEvent(firstServerSaveWarning, 120000)
	return not shutdownAtServerSave
end