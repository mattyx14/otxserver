local BOOST_SYSTEM_MONSTER_NAME_STORAGE = 12380
local BOOST_SYSTEM_LOOT_BONUS_STORAGE = 12381
local BOOST_SYSTEM_EXP_BONUS_STORAGE = 12382

function onLogin(cid)
	registerCreatureEvent(cid, "MBKill")
	local monsterName = getStorage(BOOST_SYSTEM_MONSTER_NAME_STORAGE)
	if monsterName == EMPTY_STORAGE then 
		return true
	end

	doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_ORANGE, "[Monster Boost]: Today's monster boosted is: >> "..monsterName.." <<")
	return true
end

--this system not increase loot at autoloot by source :s
local ignoredList = {1987}
local function addBonusLoot(cid, position, name)
	local corpse = 0
	for i = 0, 255 do
		position.stackpos = i
		corpse = getTileThingByPos(position)
		if corpse.uid > 0 and isCorpse(corpse.uid) then
			break
		end
	end

	local loot_bonus = getStorage(BOOST_SYSTEM_LOOT_BONUS_STORAGE)
	local monsterLoot = getMonsterLootList(name)
	local mainbp = doCreateItemEx(1987, 1)
	for i, loot in pairs(monsterLoot) do
		if math.random(1, 100000) <= (loot.chance + (loot.chance * (loot_bonus/100))) then
			if #ignoredList > 0 then
				if (not isInArray(ignoredList, loot.id)) then
					doAddContainerItem(mainbp, loot.id, loot.countmax and math.random(1, loot.countmax) or 1)
				end
			else
				doAddContainerItem(mainbp, loot.id, loot.countmax and math.random(1, loot.countmax) or 1)
			end
		end
		doAddContainerItemEx(corpse.uid, mainbp) 
	end
end

function onKill(cid, target, mostDamage)
	if not isMonster(target) then 
		return true
	end

	-- mostDamage is creatureId who gives more damage to creature killed
	-- can use 'isPlayer(mostDamage)' or some function to this creature
	-- getPlayerLevel(mostDamage)
	-- if (cid == mostDamage)...
	
	-- only gives exp to most damager
	--[[if(cid ~= mostDamage) then
		return true
	end]]--
	
	local bonusMonster = getStorage(BOOST_SYSTEM_MONSTER_NAME_STORAGE)
	local targetName = string.lower(getCreatureName(target))
	if targetName ~= bonusMonster then
		return true
	end

	local monsterExp = getMonsterInfo(targetName).experience
	local rate = getExperienceStage(getPlayerLevel(cid))
	local expBonus = getStorage(BOOST_SYSTEM_EXP_BONUS_STORAGE) 
	local exp = math.ceil((monsterExp * rate) * (expBonus / 82.6))
	doPlayerAddExperience(cid, exp)
	addEvent(addBonusLoot, 10, cid, getThingPos(target), targetName) 
	addEvent(doSendAnimatedText, 50, getThingPos(cid), "+" .. exp .. " exp", 19)
	return true
end
