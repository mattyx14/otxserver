function onThink(interval)
local counter = 0
local players = getOnlinePlayers()
	if players[1] ~= nil then
		local l = (#players > 50 and 50 or #players)
		for i = 1, l do
			local tmpPlayer = Player(players[i])
			if tmpPlayer then
				if tmpPlayer:getIp() == 0 then
					counter = counter + 1
				end
			end
		end
		if counter >= (l / 2) and l > 4 then
			ddos.enabled = true
			if not ddos.broadcasted then
				broadcastMessage("Server is under attack, monsters have been disabled.", MESSAGE_STATUS_WARNING)
			end
		else
			if ddos.enabled then
				ddos.enabled = false
				ddos.broadcasted = false
			end
		end
	end
	return true
end
