function addPlayerEvent(callable, delay, player, ...)
	addEvent(function(callable, playerId, ...)
		local player = Player(playerId)
		if player then
			pcall(callable, player, ...)
		end
	end, delay, callable, player.uid, ...)
end
