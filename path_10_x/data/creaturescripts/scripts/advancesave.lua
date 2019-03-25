local config = {
	savePlayer = true,
	healPlayerOnLevel = true,
	effectType = 30
}

function onAdvance(cid, skill, oldLevel, newLevel)
	if(skill == SKILL__EXPERIENCE) then
		return true
	end

	if(skill == SKILL__LEVEL and config.healPlayerOnLevel) then
		doCreatureAddHealth(cid, getCreatureMaxHealth(cid) - getCreatureHealth(cid))
		doCreatureAddMana(cid, getCreatureMaxMana(cid) - getCreatureMana(cid))
		doSendMagicEffect(pos, config.effectType)
	end

	if(config.savePlayer) then
		doPlayerSave(cid, true)
	end

	return true
end
