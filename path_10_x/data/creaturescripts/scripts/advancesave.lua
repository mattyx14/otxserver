local config = {
	healPlayerOnLevel = true
}

function onAdvance(cid, skill, oldLevel, newLevel)
	doPlayerSave(cid, true)

	if(skill == SKILL__EXPERIENCE) then
		return true
	end

	if(skill == SKILL__LEVEL and config.healPlayerOnLevel) then
		doCreatureAddHealth(cid, getCreatureMaxHealth(cid) - getCreatureHealth(cid))
		doCreatureAddMana(cid, getCreatureMaxMana(cid) - getCreatureMana(cid))
	end

	return true
end
