local statue = {
	[18488] = SKILL_SWORD,
	[18489] = SKILL_AXE,
	[18490] = SKILL_CLUB,
	[18491] = SKILL_DISTANCE,
	[18492] = SKILL__MAGLEVEL
}

function onUse(cid, item, fromPosition, itemEx, toPosition)
	if item.actionid == 1000 then
		return false
	end

	OfflineLevel = getConfigValue('levelToOfflineTraining')
	if getPlayerLevel(cid) <= OfflineLevel then
		return false
	end

	if getPlayerPremiumDays(cid) > 0 then
		doPlayerSetOfflineTrainingSkill(cid, statue[item.itemid])
		doRemoveCreature(cid)
	else
		doPlayerSendDefaultCancel(cid, RETURNVALUE_YOUNEEDPREMIUMACCOUNT)
	end

	return true
end
