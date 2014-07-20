local statue = {
	[1444] = SKILL_SWORD,
	[1449] = SKILL_AXE,
	[3705] = SKILL_CLUB,
	[3739] = SKILL_DISTANCE,
	[1448] = SKILL__MAGLEVEL
}

function onUse(cid, item, fromPosition, itemEx, toPosition)
	if isPlayerPzLocked(cid) then
		return false
	end

	if item.actionid == 1000 and getPlayerPremiumDays(cid) > 0 then
		doPlayerSetOfflineTrainingSkill(cid, statue[item.itemid])
		doRemoveCreature(cid)
	else
		doPlayerSendDefaultCancel(cid, RETURNVALUE_YOUNEEDPREMIUMACCOUNT)
	end

	return true
end
