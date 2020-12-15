local config = {
	demonOakIds = {8288, 8289, 8290, 8291},
	sounds = {
		'MY ROOTS ARE SHARP AS A SCYTHE! FEEL IT?!?',
		'CURSE YOU!',
		'RISE, MINIONS, RISE FROM THE DEAD!!!!',
		'AHHHH! YOUR BLOOD MAKES ME STRONG!',
		'GET THE BONES, HELLHOUND! GET THEM!!',
		'GET THERE WHERE I CAN REACH YOU!!!',
		'ETERNAL PAIN AWAITS YOU! NICE REWARD, HUH?!?!',
		'YOU ARE GOING TO PAY FOR EACH HIT WITH DECADES OF TORTURE!!',
		'ARGG! TORTURE IT!! KILL IT SLOWLY MY MINION!!'
	},
	bonebeastChance = 90,
	bonebeastCount = 4,
	waves = 10,
	questArea = {
		fromPosition = {x = 469, y = 1425, z = 7},
		toPosition = {x = 489, y = 1439, z = 7}
	},
	summonPositions = {
		{x = 473, y = 1429, z = 7},
		{x = 471, y = 1432, z = 7},
		{x = 476, y = 1431, z = 7},
		{x = 482, y = 1435, z = 7},
		{x = 476, y = 1437, z = 7},
		{x = 483, y = 1432, z = 7},
		{x = 482, y = 1429, z = 7},
		{x = 478, y = 1428, z = 7}
	},
	summons = {
		[8288] = {
			[5] = {'Braindeath', 'Braindeath', 'Braindeath', 'Bonebeast'},
			[10] = {'Betrayed Wraith', 'Betrayed Wraith'}
		},
		[8289] = {
			[5] = {'Lich', 'Lich', 'Lich'},
			[10] = {'Dark Torturer', 'Blightwalker'}
		},
		[8290] = {
			[5] = {'Banshee', 'Banshee', 'Banshee'},
			[10] = {'Grim Reaper'}
		},
		[8291] = {
			[5] = {'Giant Spider', 'Giant Spider', 'Lich'},
			[10] = {'Undead Dragon', 'Hand of Cursed Fate'}
		}
	},
	storages = {
		[8288] = Storage.DemonOak.AxeBlowsBird,
		[8289] = Storage.DemonOak.AxeBlowsLeft,
		[8290] = Storage.DemonOak.AxeBlowsRight,
		[8291] = Storage.DemonOak.AxeBlowsFace
	}
}

local function getRandomSummonPosition()
	return config.summonPositions[math.random(#config.summonPositions)]
end

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if not isInArray(config.demonOakIds, target.itemid) then
		return true
	end

	local totalProgress = 0
	for k,v in pairs(config.storages) do
		totalProgress = totalProgress + math.max(0, player:getStorageValue(v))
	end

	local spectators, hasMonsters = Game.getSpectators(DEMON_OAK_POSITION, false, false, 9, 9, 6, 6), false
	for i = 1, #spectators do
		if spectators[i]:isMonster() then
			hasMonsters = true
			break
		end
	end

	local isDefeated = totalProgress == (#config.demonOakIds * (config.waves + 1))
	if (config.killAllBeforeCut or isDefeated)
			and hasMonsters then
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, 'You need to kill all monsters first.')
		return true
	end

	if isDefeated then
		player:teleportTo(DEMON_OAK_KICK_POSITION)
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, 'Tell Oldrak about your great victory against the demon oak.')
		player:setStorageValue(Storage.DemonOak.Done, 1)
		player:setStorageValue(Storage.DemonOak.Progress, 3)
		return true
	end

	local cStorage = config.storages[target.itemid]
	local progress = math.max(player:getStorageValue(cStorage), 1)
	if progress >= config.waves + 1 then
		toPosition:sendMagicEffect(CONST_ME_POFF)
		return true
	end

	local isLastCut = totalProgress == (#config.demonOakIds * (config.waves + 1) - 1)
	local summons = config.summons[target.itemid]
	if summons and summons[progress] then
		-- Summon a single demon on the last hit
		if isLastCut then
			Game.createMonster('Demon', getRandomSummonPosition(), false, true)

		-- Summon normal monsters otherwise
		else
			for i = 1, #summons[progress] do
				Game.createMonster(summons[progress][i], getRandomSummonPosition(), false, true)
			end
		end

	-- if it is not the 5th or 10th there is only a chance to summon bonebeasts
	elseif math.random(100) >= config.bonebeastChance then
		for i = 1, config.bonebeastCount do
			Game.createMonster('Bonebeast', getRandomSummonPosition(), false, true)
		end
	end

	player:say(isLastCut and 'HOW IS THAT POSSIBLE?!? MY MASTER WILL CRUSH YOU!! AHRRGGG!' or config.sounds[math.random(#config.sounds)], TALKTYPE_MONSTER_YELL, false, player, DEMON_OAK_POSITION)
	toPosition:sendMagicEffect(CONST_ME_DRAWBLOOD)
	player:setStorageValue(cStorage, progress + 1)
	player:say('-krrrrak-', TALKTYPE_MONSTER_YELL, false, player, toPosition)
	doTargetCombatHealth(0, player, COMBAT_EARTHDAMAGE, -170, -210, CONST_ME_BIGPLANTS)
	return true
end