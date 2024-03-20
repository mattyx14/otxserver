local config = {
	healPlayerOnLevel = true,
	saveOnAdvance = true,
	fastSave = true,
}

function onAdvance(cid, skill, oldLevel, newLevel)
	if(skill == SKILL__EXPERIENCE) then
		return true
	end

	if config.saveOnAdvance then
		doPlayerSave(cid, config.fastSave)
	end

	if(skill == SKILL__LEVEL and config.healPlayerOnLevel) then
		doCreatureAddHealth(cid, getCreatureMaxHealth(cid) - getCreatureHealth(cid))
		doCreatureAddMana(cid, getCreatureMaxMana(cid) - getCreatureMana(cid))
	end

	return true
end
