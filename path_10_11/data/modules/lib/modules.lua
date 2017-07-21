function addPlayerEvent(callable, delay, playerId, ...)
	local player = Player(playerId)
	if not player then
		return false
	end

	addEvent(function(callable, playerId, ...)
		local player = Player(playerId)
		if player then
			pcall(callable, player, ...)
		end
	end, delay, callable, player.uid, ...)
end
