function onLogin(cid)
	if getConfigValue('resetSystemEnable') then
		ResetSystem:applyBonuses(cid)
		ResetSystem:updateHealthAndMana(cid)
		registerCreatureEvent(cid, "RSGainExperience")
		registerCreatureEvent(cid, "RSAdvance")
	end
	return true
end

function onAdvance(cid, skill, oldLevel, newLevel)
	if (skill == SKILL__LEVEL) and getConfigValue('resetSystemEnable') then
		ResetSystem:updateHealthAndMana(cid)
	end
	return true
end
