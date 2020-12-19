local loginEvents = CreatureEvent("LoginEvents")
function loginEvents.onLogin(player)

	local events = {
		--Others
		"AdvanceSave",
		"BestiaryOnKill",
		"BossParticipation",
		"DropLoot",
		"PlayerDeath",
		"PreyLootBonusKill",
		"RookgaardAdvance",
		"SummonLogin",
		"SummonThink"
	}

	for i = 1, #events do
		player:registerEvent(events[i])
	end
	return true
end
loginEvents:register()
