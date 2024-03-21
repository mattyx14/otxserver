-- position of the "monster", in the monster room
local monsterPosition = {x=90, y=128, z=7}
-- position of the phrase "boost", in the showcase room
local lootBoostPosition = {x=89, y=128, z=7}
-- position of the phrase "Exp", in the showcase room
local expBoostPosition = {x=91, y=128, z=7}

local BOOST_SYSTEM_MONSTER_NAME_STORAGE = 12380
local BOOST_SYSTEM_LOOT_BONUS_STORAGE = 12381
local BOOST_SYSTEM_EXP_BONUS_STORAGE = 12382

string.upperAllFirst = string.upperAllFirst or function (str)
	return string.gsub(' ' .. str, '%W%l', string.upper):sub(2)
end

-- list of monsters that will be boosted and days
local days = {
	["Sunday"]    = {"rat", "rotworm", "demon"},
	["Monday"]    = {"rat", "rotworm", "demon"},
	["Tuesday"]   = {"rat", "rotworm", "demon"},
	["Wednesday"] = {"rat", "rotworm", "demon"},
	["Thursday"]  = {"rat", "rotworm", "demon"},
	["Friday"]    = {"rat", "rotworm", "demon"},
	["Saturday"]  = {"rat", "rotworm", "demon"},
}

function onStartup()
	local monster = days[os.date("%A")]
	if not monster then
		return true 
	end

	local monsterToday = monster[math.random(1, #monster)]
	local func = db.query or db.executeQuery
	doSetStorage(BOOST_SYSTEM_MONSTER_NAME_STORAGE, monsterToday:lower())
	doSetStorage(BOOST_SYSTEM_LOOT_BONUS_STORAGE, math.random(10, 50))
	doSetStorage(BOOST_SYSTEM_EXP_BONUS_STORAGE, math.random(20, 48))
	local getMonster = doCreateMonster(monsterToday, monsterPosition, false, true)
	doCreatureSetLookDirection(getMonster, EAST)
	func("INSERT INTO monster_boost (monster, loot, exp) VALUES ('"..monsterToday.."', '"..getStorage(BOOST_SYSTEM_LOOT_BONUS_STORAGE).."', '"..getStorage(BOOST_SYSTEM_EXP_BONUS_STORAGE).."')")
	return true
end

function onThink()
	local monsterName = getStorage(BOOST_SYSTEM_MONSTER_NAME_STORAGE)
	if monsterName == EMPTY_STORAGE then 
		return true
	end

	local creature = getTopCreature(monsterPosition)
	if not creature or creature.uid == 0 then 
		local getMonster = doCreateMonster(monsterName, monsterPosition, false, true)
		doCreatureSetLookDirection(getMonster, EAST)
	elseif getCreatureName(creature.uid):lower() ~= monsterName then
		doRemoveCreature(creature.uid)
		local getMonster = doCreateMonster(monsterName, monsterPosition, false, true)
		doCreatureSetLookDirection(getMonster, EAST)
	end

	--doSendAnimatedText(monsterPosition, string.upperAllFirst(monsterName), COLOR_LIGHTBLUE)
	doSendAnimatedText(lootBoostPosition, "Loot +" .. getStorage(BOOST_SYSTEM_LOOT_BONUS_STORAGE) .. "%", 194)
	doSendAnimatedText(expBoostPosition, "Exp +" .. getStorage(BOOST_SYSTEM_EXP_BONUS_STORAGE) .. "%", 194)

	return true
end

--- ### FUNCTIONS TO WINDOWS ### ---
--[[
function onStartup()
	local dayOfWeek = os.date("%A")
	local monsterList = days[dayOfWeek]
	if not monsterList then
		return true 
	end

	math.randomseed(os.clock() * 1000000000)
	local selectedMonsterIndex = math.random(1, #monsterList)
	local selectedMonster = monsterList[selectedMonsterIndex]
	local func = db.query or db.executeQuery

	doSetStorage(BOOST_SYSTEM_MONSTER_NAME_STORAGE, string.lower(selectedMonster))
	doSetStorage(BOOST_SYSTEM_EXP_BONUS_STORAGE, math.random(10, 15))
	doSetStorage(BOOST_SYSTEM_LOOT_BONUS_STORAGE, math.random(10, 50))
	local getMonster = doCreateMonster(selectedMonster, monsterPosition, false, true)
	doCreatureSetLookDirection(getMonster, EAST)

	func("INSERT INTO monster_boost (monster, loot, exp) VALUES ('"..selectedMonster.."', '"..getStorage(BOOST_SYSTEM_LOOT_BONUS_STORAGE).."', '"..getStorage(BOOST_SYSTEM_EXP_BONUS_STORAGE).."')")
	return true
end

function onThink()
	local monsterName = getStorage(BOOST_SYSTEM_MONSTER_NAME_STORAGE)
	if monsterName == EMPTY_STORAGE then 
		return true
	end

	local creature = getTopCreature(monsterPosition)
	if not creature or creature.uid == 0 then 
		local getMonster = doCreateMonster(monsterName, monsterPosition, false, true)
		doCreatureSetLookDirection(getMonster, EAST)
	elseif string.lower(getCreatureName(creature.uid)) ~= monsterName then
		doRemoveCreature(creature.uid)
		local getMonster = doCreateMonster(monsterName, monsterPosition, false, true)
		doCreatureSetLookDirection(getMonster, EAST)
	end

	--doSendAnimatedText(monsterPosition, string.upperAllFirst(monsterName), COLOR_LIGHTBLUE)
	doSendAnimatedText(lootBoostPosition, "Loot +" .. getStorage(BOOST_SYSTEM_LOOT_BONUS_STORAGE) .. "%", 194)
	doSendAnimatedText(expBoostPosition, "Exp +" .. getStorage(BOOST_SYSTEM_EXP_BONUS_STORAGE) .. "%", 194)
	return true
end]]--