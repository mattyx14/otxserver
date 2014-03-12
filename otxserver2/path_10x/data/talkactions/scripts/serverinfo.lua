local config = {
	rateExperience = getConfigInfo('rateExperience'),
	rateSkill = getConfigInfo('rateSkill'),
	rateLoot = getConfigInfo('rateLoot'),
	rateMagic = getConfigInfo('rateMagic'),
	rateSpawnMin = getConfigInfo('rateSpawnMin'),
	rateSpawnMax = getConfigInfo('rateSpawnMax'),
	protectionLevel = getConfigInfo('protectionLevel'),
	stages = getBooleanFromString(getConfigInfo('experienceStages'))
}

function onSay(cid, words, param, channel)
	local exp = config.rateExperience
	if(config.stages) then
		exp = getExperienceStage(getPlayerLevel(cid), getVocationInfo(getPlayerVocation(cid)).experienceMultiplier)
	end

	doPlayerPopupFYI(cid, "Server Information:\n\nExperience rate: x" .. exp .. "\nSkills rate: x" .. config.rateSkill .. "\nLoot rate: x" .. config.rateLoot .. "\nMagic rate: x" .. config.rateMagic .. "\nSpawns rate: x" .. config.rateSpawnMin .. " - x" .. config.rateSpawnMax .. "\nProtection level: " .. config.protectionLevel)
	return true
end
