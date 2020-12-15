local vampireBoss = {
	['zevelon duskbringer'] = {cStorage = Storage.VampireQuest.BossStorageDusk, cGlobalStorage = Storage.VampireQuest.zevelonDuskbringer},
	['sir valorcrest'] = {cStorage = Storage.VampireQuest.BossStorageVarlor, cGlobalStorage = Storage.VampireQuest.sirValorcrest},
	['diblis the fair'] = {cStorage = Storage.VampireQuest.BossStorageFair, cGlobalStorage = Storage.VampireQuest.diblisTheFair},
	['arachir the ancient one'] = {cStorage = Storage.VampireQuest.BossStorageOne, cGlobalStorage = Storage.VampireQuest.arachirTheAncientOne},
}

function onKill(creature, target)
	if not target:isMonster() then
		return true
	end

	local bossName = target:getName()
	local bossConfig = vampireBoss[bossName:lower()]
	if not bossConfig then
		return true
	end

	if bossConfig.cGlobalStorage then
		Game.setStorageValue(bossConfig.cGlobalStorage, 0)
	end

	if bossConfig.cStorage and creature:getStorageValue(bossConfig.cStorage) < 1 then
		creature:setStorageValue(bossConfig.cStorage, 1)
	end

	creature:say('You slayed ' .. bossName .. '.', TALKTYPE_MONSTER_SAY)
	return true
end
