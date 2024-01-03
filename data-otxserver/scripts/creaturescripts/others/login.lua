local function onMovementRemoveProtection(playerId, oldPos, time)
	local player = Player(playerId)
	if not player then
		return true
	end

	local playerPos = player:getPosition()
	if (playerPos.x ~= oldPos.x or playerPos.y ~= oldPos.y or playerPos.z ~= oldPos.z) or player:getTarget() then
		player:kv():remove("combat-protection")
		return true
	end

	addEvent(onMovementRemoveProtection, 1000, playerId, oldPos, time - 1)
end

local function protectionZoneCheck(playerName)
	doRemoveCreature(playerName)
	return true
end

local playerLogin = CreatureEvent("PlayerLogin")

function playerLogin.onLogin(player)
	if player:getLastLoginSaved() == 0 then
		player:sendOutfitWindow()
		db.query("UPDATE `players` SET `istutorial` = 0 where `id`=" .. player:getGuid())
		-- Open channels
		if table.contains({ TOWNS_LIST.DAWNPORT, TOWNS_LIST.DAWNPORT_TUTORIAL }, player:getTown():getId()) then
			player:openChannel(3) -- World chat
		else
			player:openChannel(3) -- World chat
			player:openChannel(5) -- Advertsing main
		end
	else
		player:sendTextMessage(MESSAGE_STATUS, SERVER_MOTD)
		player:sendTextMessage(MESSAGE_LOGIN, string.format("Your last visit in " .. SERVER_NAME .. ": %s.", os.date("%d. %b %Y %X", player:getLastLoginSaved())))
	end

	-- Reset bosstiary time
	local lastSaveServerTime = GetDailyRewardLastServerSave()
	if lastSaveServerTime >= player:getLastLoginSaved() then
		player:setRemoveBossTime(1)
	end

	if isPremium(player) then
		player:setStorageValue(Storage.PremiumAccount, 1)
	end

	local playerId = player:getId()
	DailyReward.init(playerId)

	player:loadSpecialStorage()

	-- Boosted creature
	player:sendTextMessage(MESSAGE_BOOSTED_CREATURE, "Today's boosted creature: " .. Game.getBoostedCreature() .. " \
	Boosted creatures yield more experience points, carry more loot than usual and respawn at a faster rate.")

	-- Boosted boss
	player:sendTextMessage(MESSAGE_BOOSTED_CREATURE, "Today's boosted boss: " .. Game.getBoostedBoss() .. " \
	Boosted bosses contain more loot and count more kills for your Bosstiary.")

	if SCHEDULE_EXP_RATE ~= 100 then
		if SCHEDULE_EXP_RATE > 100 then
			player:sendTextMessage(
				MESSAGE_BOOSTED_CREATURE,
				"Exp Rate Event! Monsters yield more experience points than usual \
			Happy Hunting!"
			)
		else
			player:sendTextMessage(MESSAGE_BOOSTED_CREATURE, "Exp Rate Decreased! Monsters yield less experience points than usual.")
		end
	end

	if SCHEDULE_SPAWN_RATE ~= 100 then
		if SCHEDULE_SPAWN_RATE > 100 then
			player:sendTextMessage(
				MESSAGE_BOOSTED_CREATURE,
				"Spawn Rate Event! Monsters respawn at a faster rate \
			Happy Hunting!"
			)
		else
			player:sendTextMessage(MESSAGE_BOOSTED_CREATURE, "Spawn Rate Decreased! Monsters respawn at a slower rate.")
		end
	end

	if SCHEDULE_LOOT_RATE ~= 100 then
		if SCHEDULE_LOOT_RATE > 100 then
			player:sendTextMessage(
				MESSAGE_BOOSTED_CREATURE,
				"Loot Rate Event! Monsters carry more loot than usual \
			Happy Hunting!"
			)
		else
			player:sendTextMessage(MESSAGE_BOOSTED_CREATURE, "Loot Rate Decreased! Monsters carry less loot than usual.")
		end
	end

	if SCHEDULE_SKILL_RATE ~= 100 then
		if SCHEDULE_SKILL_RATE > 100 then
			player:sendTextMessage(
				MESSAGE_BOOSTED_CREATURE,
				"Skill Rate Event! Your skills progresses at a higher rate \
			Happy Hunting!"
			)
		else
			player:sendTextMessage(MESSAGE_BOOSTED_CREATURE, "Skill Rate Decreased! Your skills progresses at a lower rate.")
		end
	end

	-- Loyalty system
	player:initializeLoyaltySystem()

	-- Stamina
	_G.NextUseStaminaTime[playerId] = 1

	-- EXP Stamina
	_G.NextUseXpStamina[playerId] = 1

	-- Concoction Duration
	_G.NextUseConcoctionTime[playerId] = 1

	-- Rewards
	local rewards = #player:getRewardList()
	if rewards > 0 then
		player:sendTextMessage(MESSAGE_LOGIN, string.format("You have %d %s in your reward chest.", rewards, rewards > 1 and "rewards" or "reward"))
	end

	-- Update player id
	local stats = player:inBossFight()
	if stats then
		stats.playerId = player:getId()
	end

	local isProtected = player:kv():get("combat-protection") or 0
	if isProtected < 1 then
		player:kv():set("combat-protection", 1)
		onMovementRemoveProtection(playerId, player:getPosition(), 10)
	end

	-- Set Client XP Gain Rate --
	if configManager.getBoolean(configKeys.XP_DISPLAY_MODE) then
		local baseRate = player:getFinalBaseRateExperience()
		baseRate = baseRate * 100
		if configManager.getBoolean(configKeys.VIP_SYSTEM_ENABLED) then
			local vipBonusExp = configManager.getNumber(configKeys.VIP_BONUS_EXP)
			if vipBonusExp > 0 and player:isVip() then
				vipBonusExp = (vipBonusExp > 100 and 100) or vipBonusExp
				baseRate = baseRate * (1 + (vipBonusExp / 100))
				player:sendTextMessage(MESSAGE_BOOSTED_CREATURE, "Normal base xp is: " .. baseRate .. "%, because you are VIP, bonus of " .. vipBonusExp .. "%")
			end
		end
		player:setBaseXpGain(baseRate)
	end

	local staminaBonus = player:getFinalBonusStamina()
	player:setStaminaXpBoost(staminaBonus * 100)

	player:getFinalLowLevelBonus()

	if _G.OnExerciseTraining[player:getId()] then
		-- onLogin & onLogout
		stopEvent(_G.OnExerciseTraining[player:getId()].event)
		_G.OnExerciseTraining[player:getId()] = nil
		player:setTraining(false)
	end
	return true
end

playerLogin:register()
