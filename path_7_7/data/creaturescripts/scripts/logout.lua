function onLogout(player)
	local playerId = player:getId()
	db.query("DELETE FROM `players_online` WHERE `player_id` = " .. playerId .. ";")

	local stats = player:inBossFight()
	if stats then
		-- Player logged out (or died) in the middle of a boss fight, store his damageOut
		local boss = Monster(stats.bossId)
		if boss then
			local dmgOut = boss:getDamageMap()[playerId]
			if dmgOut then
				stats.damageOut = (stats.damageOut or 0) + dmgOut.total
			end
		end
	end

	return true
end
