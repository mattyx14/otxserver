function onLogin(player)
	local events = {
		"PlayerDeath",
		"AdvanceSave",
		"DropLoot",
		"BossParticipation",
		"petlogin",
		"petthink",
		"bonusPreyLootKill"
	}
	
	for i = 1, #events do
		player:registerEvent(events[i])
	end

	return true
end
