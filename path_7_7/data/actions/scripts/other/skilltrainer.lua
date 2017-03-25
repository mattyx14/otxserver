local statues = {
	[] = SKILL_SWORD,
	[] = SKILL_AXE,
	[] = SKILL_CLUB,
	[] = SKILL_DISTANCE,
	[] = SKILL_MAGLEVEL
}

function onUse(player, item, fromPosition, target, toPosition)
	local skill = statues[item:getId()]
	if not player:isPremium() then
		player:sendCancelMessage(RETURNVALUE_YOUNEEDPREMIUMACCOUNT)
		return true
	end

	if player:isPzLocked() then
		return false
	end

	player:setOfflineTrainingSkill(skill)
	player:remove()
	return true
end
