function onKill(player, creature)
	if not player:isPlayer() then return true end
	if not creature:isMonster() or creature:getMaster() then return true end
	local bosses = {
		["lady tenebris"] = {stg = Storage.ForgottenKnowledge.LadyTenebrisKilled, value = 1},
		["jaul"] = {stg = Storage.MisidiaQuest.JaulKilled, value = 1},
		["izcandar champion of winter"] = {stg = Storage.AnsharaPOI.IzcandarWinterKilled, value = 1},
		["infernus"] = {stg = Storage.AnsharaPOI.InfernusKilled, value = 1},
	}
	local monsterName = creature:getName():lower()
	local boss = bosses[monsterName]
	if boss then
		for playerid, damage in pairs(creature:getDamageMap()) do
			local p = Player(playerid)
			if p then
				if p:getStorageValue(boss.stg) < boss.value then
					p:setStorageValue(boss.stg, boss.value)
				end
			end
		end
	end
	return true
end
