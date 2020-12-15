function onLogin(player)
	local events = {
		"PlayerDeath",
		"AdvanceSave",
		"DropLoot",
		"BossParticipation",
		"petlogin",
		"petthink",
		"bonusPreyLootKill",
		"BestiaryOnKill",

		-- DarkKonia
		"onadvance_reward",
		"KillBoss",
		"VampireKill",
		"BossesForgotten",
		"KillingInTheNameOfKills",
		"KillingInTheNameOfKillss",
		"KillingInTheNameOfKillsss",
	}

	for i = 1, #events do
		player:registerEvent(events[i])
	end

	return true
end
