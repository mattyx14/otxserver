function isInArray(array, value, caseSensitive)
	if(caseSensitive == nil or caseSensitive == false) and type(value) == "string" then
		local lowerValue = value:lower()
		for _, _value in ipairs(array) do
			if type(_value) == "string" and lowerValue == _value:lower() then
				return true
			end
		end
	else
		for _, _value in ipairs(array) do
			if (value == _value) then return true end
		end
	end

	return false
end

function doPlayerGiveItem(cid, itemid, amount, subType)
	local item = 0
	if(isItemStackable(itemid)) then
		item = doCreateItemEx(itemid, amount)
		if(doPlayerAddItemEx(cid, item, true) ~= RETURNVALUE_NOERROR) then
			return false
		end
	else
		for i = 1, amount do
			item = doCreateItemEx(itemid, subType)
			if(doPlayerAddItemEx(cid, item, true) ~= RETURNVALUE_NOERROR) then
				return false
			end
		end
	end

	return true
end

function doPlayerGiveItemContainer(cid, containerid, itemid, amount, subType)
	for i = 1, amount do
		local container = doCreateItemEx(containerid, 1)
		for x = 1, getContainerCapById(containerid) do
			doAddContainerItem(container, itemid, subType)
		end

		if(doPlayerAddItemEx(cid, container, true) ~= RETURNVALUE_NOERROR) then
			return false
		end
	end

	return true
end

function doPlayerTakeItem(cid, itemid, amount)
	return getPlayerItemCount(cid, itemid) >= amount and doPlayerRemoveItem(cid, itemid, amount)
end

function doPlayerSellItem(cid, itemid, count, cost)
	if(not doPlayerTakeItem(cid, itemid, count)) then
		return false
	end

	if(not doPlayerAddMoney(cid, cost)) then
		error('[doPlayerSellItem] Could not add money to: ' .. getPlayerName(cid) .. ' (' .. cost .. 'gp).')
	end

	return true
end

function doPlayerWithdrawMoney(cid, amount)
	if(not getBooleanFromString(getConfigInfo('bankSystem'))) then
		return false
	end

	local balance = getPlayerBalance(cid)
	if(amount > balance or not doPlayerAddMoney(cid, amount)) then
		return false
	end

	doPlayerSetBalance(cid, balance - amount)
	return true
end

function doPlayerDepositMoney(cid, amount)
	if(not getBooleanFromString(getConfigInfo('bankSystem'))) then
		return false
	end

	if(not doPlayerRemoveMoney(cid, amount)) then
		return false
	end

	doPlayerSetBalance(cid, getPlayerBalance(cid) + amount)
	return true
end

function doPlayerAddStamina(cid, minutes)
	return doPlayerSetStamina(cid, getPlayerStamina(cid) + minutes)
end

function isPremium(cid)
	return (isPlayer(cid) and (getPlayerPremiumDays(cid) > 0 or getBooleanFromString(getConfigValue('freePremium'))))
end

function getMonthDayEnding(day)
	if(day == "01" or day == "21" or day == "31") then
		return "st"
	elseif(day == "02" or day == "22") then
		return "nd"
	elseif(day == "03" or day == "23") then
		return "rd"
	end

	return "th"
end

function getMonthString(m)
	return os.date("%B", os.time{year = 1970, month = m, day = 1})
end

function getArticle(str)
	return str:find("[AaEeIiOoUuYy]") == 1 and "an" or "a"
end

function doNumberFormat(i)
	local str, found = string.gsub(i, "(%d)(%d%d%d)$", "%1,%2", 1), 0
	repeat
		str, found = string.gsub(str, "(%d)(%d%d%d),", "%1,%2,", 1)
	until found == 0
	return str
end

function doPlayerAddAddons(cid, addon)
	for i = 0, table.maxn(maleOutfits) do
		doPlayerAddOutfit(cid, maleOutfits[i], addon)
	end

	for i = 0, table.maxn(femaleOutfits) do
		doPlayerAddOutfit(cid, femaleOutfits[i], addon)
	end
end

function getTibiaTime(num)
	local minutes, hours = getWorldTime(), 0
	while (minutes > 60) do
		hours = hours + 1
		minutes = minutes - 60
	end

	if(num) then
		return {hours = hours, minutes = minutes}
	end

	return {hours =  hours < 10 and '0' .. hours or '' .. hours, minutes = minutes < 10 and '0' .. minutes or '' .. minutes}
end

function doWriteLogFile(file, text)
	local f = io.open(file, "a+")
	if(not f) then
		return false
	end

	f:write("[" .. os.date("%d/%m/%Y %H:%M:%S") .. "] " .. text .. "\n")
	f:close()
	return true
end

function getExperienceForLevel(lv)
	lv = lv - 1
	return ((50 * lv * lv * lv) - (150 * lv * lv) + (400 * lv)) / 3
end

function doMutePlayer(cid, time, sub)
	local condition = createConditionObject(CONDITION_MUTED, (time == -1 and time or time * 1000))
	if(type(sub) == 'number') then
		setConditionParam(condition, CONDITION_PARAM_SUBID, sub, false)
	end

	return doAddCondition(cid, condition, false)
end

function doSummonCreature(name, pos)
	local cid = doCreateMonster(name, pos, false, false)
	if(not cid) then
		cid = doCreateNpc(name, pos)
	end

	return cid
end

function getPlayersOnlineEx()
	local players = {}
	for i, cid in ipairs(getPlayersOnline()) do
		table.insert(players, getCreatureName(cid))
	end

	return players
end

function isPlayer(cid)
	return isCreature(cid) and cid >= AUTOID_PLAYERS and cid < AUTOID_MONSTERS
end

function isPlayerGhost(cid)
	return isPlayer(cid) and (getCreatureCondition(cid, CONDITION_GAMEMASTER, GAMEMASTER_INVISIBLE, CONDITIONID_DEFAULT) or getPlayerFlagValue(cid, PLAYERFLAG_CANNOTBESEEN))
end

function isMonster(cid)
	return isCreature(cid) and cid >= AUTOID_MONSTERS and cid < AUTOID_NPCS
end

function isNpc(cid)
	-- Npc IDs are over int32_t range (which is default for lua_pushnumber),
	-- therefore number is always a negative value.
	return isCreature(cid) and (cid < 0 or cid >= AUTOID_NPCS)
end

function isUnderWater(cid)
	return isInArray(underWater, getTileInfo(getCreaturePosition(cid)).itemid)
end

function doPlayerAddLevel(cid, amount, round)
	local experience, level, amount = 0, getPlayerLevel(cid), amount or 1
	if(amount > 0) then
		experience = getExperienceForLevel(level + amount) - (round and getPlayerExperience(cid) or getExperienceForLevel(level))
	else
		experience = -((round and getPlayerExperience(cid) or getExperienceForLevel(level)) - getExperienceForLevel(level + amount))
	end

	return doPlayerAddExperience(cid, experience)
end

function doPlayerAddMagLevel(cid, amount)
	local amount = amount or 1
	for i = 1, amount do
		doPlayerAddSpentMana(cid, getPlayerRequiredMana(cid, getPlayerMagLevel(cid, true) + 1) - getPlayerSpentMana(cid), false)
	end

	return true
end

function doPlayerAddSkill(cid, skill, amount, round)
	local amount = amount or 1
	if(skill == SKILL__LEVEL) then
		return doPlayerAddLevel(cid, amount, round)
	elseif(skill == SKILL__MAGLEVEL) then
		return doPlayerAddMagLevel(cid, amount)
	end

	for i = 1, amount do
		doPlayerAddSkillTry(cid, skill, getPlayerRequiredSkillTries(cid, skill, getPlayerSkillLevel(cid, skill) + 1) - getPlayerSkillTries(cid, skill), false)
	end

	return true
end

function isPrivateChannel(channelId)
	return channelId >= CHANNEL_PRIVATE
end

function doBroadcastMessage(text, class)
	local class = class or MESSAGE_STATUS_WARNING
	if(type(class) == 'string') then
		local className = MESSAGE_TYPES[class]
		if(className == nil) then
			return false
		end

		class = className
	elseif(class < MESSAGE_FIRST or class > MESSAGE_LAST) then
		return false
	end

	for _, pid in ipairs(getPlayersOnline()) do
		doPlayerSendTextMessage(pid, class, text)
	end

	if getConfigValue('displayBroadcastLog') then
		print("> Broadcasted message: ".. text .. ".")
	end

	return true
end

function doPlayerBroadcastMessage(cid, text, class, checkFlag, ghost)
	local checkFlag, ghost, class = checkFlag or true, ghost or false, class or TALKTYPE_BROADCAST
	if(checkFlag and not getPlayerFlagValue(cid, PLAYERFLAG_CANBROADCAST)) then
		return false
	end

	if(type(class) == 'string') then
		local className = TALKTYPE_TYPES[class]
		if(className == nil) then
			return false
		end

		class = className
	elseif(class < TALKTYPE_FIRST or class > TALKTYPE_LAST) then
		return false
	end

	for _, pid in ipairs(getPlayersOnline()) do
		doCreatureSay(cid, text, class, ghost, pid)
	end

	print("> " .. getCreatureName(cid) .. " broadcasted message: \"" .. text .. "\".")
	return true
end

function doCopyItem(item, attributes)
	local attributes = ((type(attributes) == 'table') and attributes or { "aid" })

	local ret = doCreateItemEx(item.itemid, item.type)
	for _, key in ipairs(attributes) do
		local value = getItemAttribute(item.uid, key)
		if(value ~= nil) then
			doItemSetAttribute(ret, key, value)
		end
	end

	if(isContainer(item.uid)) then
		for i = (getContainerSize(item.uid) - 1), 0, -1 do
			local tmp = getContainerItem(item.uid, i)
			if(tmp.itemid > 0) then
				doAddContainerItemEx(ret, doCopyItem(tmp, attributes).uid)
			end
		end
	end

	return getThing(ret)
end

function doSetItemText(uid, text, writer, date)
	local thing = getThing(uid)
	if(thing.itemid < 100) then
		return false
	end

	doItemSetAttribute(uid, "text", text)
	if(writer ~= nil) then
		doItemSetAttribute(uid, "writer", tostring(writer))
		if(date ~= nil) then
			doItemSetAttribute(uid, "date", tonumber(date))
		end
	end

	return true
end

function getItemWeightById(itemid, count, precision)
	local item, count, precision = getItemInfo(itemid), count or 1, precision or false
	if(not item) then
		return false
	end

	if(count > 100) then
		-- print a warning, as its impossible to have more than 100 stackable items without "cheating" the count
		print('[Warning] getItemWeightById', 'Calculating weight for more than 100 items!')
	end

	local weight = item.weight * count
	return precission and weight or math.round(weight, 2)
end

function choose(...)
	local arg = {...}
	return arg[math.rand(1, table.maxn(arg))]
end

function doPlayerAddExpEx(cid, amount)
	if(not doPlayerAddExp(cid, amount)) then
		return false
	end

	local position = getThingPosition(cid)
	doPlayerSendTextMessage(cid, MESSAGE_EXPERIENCE, "You gained " .. amount .. " experience.", amount, COLOR_WHITE, position)

	local spectators, name = getSpectators(position, 7, 7), getCreatureName(cid)
	for _, pid in ipairs(spectators) do
		if(isPlayer(pid) and cid ~= pid) then
			doPlayerSendTextMessage(pid, MESSAGE_EXPERIENCE_OTHERS, name .. " gained " .. amount .. " experience.", amount, COLOR_WHITE, position)
		end
	end

	return true
end

function getItemTopParent(uid)
	local parent = getItemParent(uid)
	if(not parent or parent.uid == 0) then
		return nil
	end

	for i = 1, 1000 do
		local tmp = getItemParent(parent.uid)
		if(tmp and tmp.uid ~= 0 and (not parent or parent.uid == 0 or tmp.uid ~= parent.uid)) then
			parent = tmp
		else
			break
		end
	end

	return parent
end

function getItemHolder(uid)
	local parent = getItemParent(uid)
	if(not parent or parent.uid == 0) then
		return nil
	end

	local holder = nil
	for i = 1, 1000 do
		local tmp = getItemParent(parent.uid)
		if(tmp and tmp.uid ~= 0 and (not parent or parent.uid == 0 or tmp.uid ~= parent.uid)) then
			if(tmp.itemid == 1) then -- a creature
				holder = tmp
				break
			end

			parent = tmp
		else
			break
		end
	end

	return holder
end

function valid(f)
	return function(p, ...)
		if(isCreature(p)) then
			return f(p, ...)
		end
	end
end

function addContainerItems(container,items)
	local items_mod = {}
	for _, it in ipairs(items) do
		if( isItemStackable(it.id) and it.count > 100) then
			local c = it.count
			while( c > 100 ) do
				table.insert(items_mod,{id = it.id,count = 100})
				c = c - 100
			end
			if(c > 0) then
				table.insert(items_mod,{id = it.id,count = c})
			end
		else
			table.insert(items_mod,{id = it.id,count = 1})
		end
	end

	local free = getContainerCap(container.uid) - (getContainerSize(container.uid) )
	local count = math.ceil(#items_mod/ free)
	local main_bp = container.uid
	local insert_bp = main_bp
	local counter = 1
	for c,it in ipairs(items_mod) do
		local _c = isItemStackable(it.id) and (it.count > 100 and 100 or it.count) or 1
		if count > 1 then
			if (counter < free) then
				doAddContainerItem(insert_bp, it.id, _c)
			else
				insert_bp = doAddContainerItem(insert_bp, container.itemid, 1)
				count = (#items_mod)-(free-1)
				free = getContainerCap(insert_bp) 
				count = math.ceil(count/ free)
				doAddContainerItem(insert_bp, it.id, _c)
				counter = 1
			end
			counter = counter + 1
		else
			doAddContainerItem(insert_bp, it.id, _c)
		end
	end

	return main_bp
end

function getContainerItemCount(uid, itemid, recursive)
	local c, s = 0, getContainerSize(uid)
	for i = 1, s do
		local thing = getContainerItem(uid, (i - 1))
		if(thing.uid ~= 0) then
			if(recursive and isContainer(thing.uid)) then
				c = c + getContainerItemCount(thing.uid, itemid, recursive)
			end

			if(thing.itemid == itemid) then
				c = c + thing.type
			end
		end
	end

	return c
end

function getContainerItems(uid, itemid, recursive)
	local a, s = {}, getContainerSize(uid)
	for i = 1, s do
		local thing = getContainerItem(uid, (i - 1))
		if(thing.uid ~= 0) then
			if(recursive and isContainer(thing.uid)) then
				a = table.merge(a, getContainerItems(thing.uid, itemid, true))
			end

			if(thing.itemid == itemid) then
				table.insert(a, thing)
			end
		end
	end

	return a
end

-- Focus Save
function doPlayerSaveEx(cid)
	doCreatureSetStorage(cid, "save")
	local result = doPlayerSave(cid)
	doCreatureSetStorage(cid, "save", (os.time() + math.random(30, 90)))
	return result
end
-- Focus Save

function doPlayerBuyItem(cid, itemid, count, cost, charges)
	return doPlayerRemoveMoneyEx(cid, cost) and doPlayerGiveItem(cid, itemid, count, charges)
end

function doPlayerBuyItemContainer(cid, containerid, itemid, count, cost, charges)
	return doPlayerRemoveMoneyEx(cid, cost) and doPlayerGiveItemContainer(cid, containerid, itemid, count, charges)
end

function isRookie(cid, promoted)
	local arr = {0}
	if(promoted) then
		table.remove(arr, 1)
	end

	return isInArray(arr, getPlayerVocation(cid))
end

function doGenerateCode()
	local chars = {}
	for i = 1, 8 do
		local tmp = math.rand(1, (i == 1 and 2 or 3))
		if(tmp == 1) then
			table.insert(chars, math.rand(65, 90))
		elseif(tmp == 2) then
			table.insert(chars, math.rand(97, 122))
		else
			table.insert(chars, math.rand(48, 57))
		end
	end

	return string.format("%c%c%c%c%c%c%c%c", chars[1], chars[2], chars[3], chars[4], chars[5], chars[6], chars[7], chars[8])
end

-- Custom libs
function getRespawnDivider()
	local multiplier = 1
	local size = #getPlayersOnline()
	if size >= 100 and size < 199 then
		multiplier = 2
	elseif size >= 200 and size < 299 then
		multiplier = 3
	elseif size >= 300 then
		multiplier = 4
	end
	return multiplier
end

function setItemOwner(cid, item)
	doItemSetAttribute(item, 'owner', getPlayerGUID(cid))
	doItemSetAttribute(item, 'ownername', getPlayerName(cid))
end

function isItemFluidContainer(itemid)
	local item = getItemInfo(itemid)
	return item and item.group == ITEM_GROUP_FLUID or false
end

function getAccountStorageValue(accid, key)
	local value = db.getResult("SELECT `value` FROM `account_storage` WHERE `account_id` = " .. accid .. " and `key` = " .. key .. " LIMIT 1;")
	if(value:getID() ~= -1) then
		return value:getDataInt("value")
	else
		return -1
	end
	value:free()
end

function setAccountStorageValue(accid, key, value)
	local getvalue = db.getResult("SELECT `value` FROM `account_storage` WHERE `account_id` = " .. accid .. " and `key` = " .. key .. " LIMIT 1;")
	if(getvalue:getID() ~= -1) then
		db.executeQuery("UPDATE `account_storage` SET `value` = " .. accid .. " WHERE `key`=" .. key .. " LIMIT 1');")
		getvalue:free()
		return 1
	else
		db.executeQuery("INSERT INTO `account_storage` (`account_id`, `key`, `value`) VALUES (" .. accid .. ", " .. key .. ", '"..value.."');")
		return 1
	end
end


function doSpawnInArea(monsters, fromPos, toPos, count)
	if not type(monsters) == 'table' then
		return false
	end

	local countFreeTiles = 0
	for i=fromPos.x, toPos.x do
		for j=fromPos.y, toPos.y do
			if isWalkable({x=i, y=j, z=fromPos.z}, true, true, true, false) then
				countFreeTiles = countFreeTiles + 1
				if countFreeTiles >= count then break end
			end
		end	
		if countFreeTiles >= count then break end
	end
	
	for i=1, countFreeTiles do
		local pos = {x=math.random(fromPos.x, toPos.x), y=math.random(fromPos.y, toPos.y), z=fromPos.z}
		while(not isWalkable(pos, true, true, true, false)) do
			pos = {x=math.random(fromPos.x, toPos.x), y=math.random(fromPos.y, toPos.y), z=fromPos.z}
		end
		local monster = monsters[math.random(1, #monsters)]
		doCreateMonster(monster, pos, false, true)
	end

	return true
end

function doCleanArena()
	local monsters = getMonstersInArea(posplayers.pos1, posplayers.pos2)
	for _, cid in pairs(monsters) do
		doRemoveCreature(cid)
	end
end

function doStartWave(waveID)
	if waves[waveID] then
		doSpawnInArea(waves[waveID].monsters, posplayers.pos1, posplayers.pos2, waves[waveID].count)
	end
end

function verificaPlayers(frompos, topos)
	for x = frompos.x, topos.x do
		for y = frompos.y, topos.y do
			if isPlayer(getThingFromPos({x = x, y = y, z = frompos.z, stackpos = 253}).uid) then
				return true
			end
		end
	end
end

function verificaBoss(frompos, topos)
	for x = frompos.x, topos.x  do
		for y = frompos.y, topos.y do
			if isMonster(getThingFromPos({x = x, y = y, z = frompos.z, stackpos = 253}).uid) then
				doRemoveCreature(getThingFromPos({x = x, y = y, z = frompos.z, stackpos = 253}).uid)
			end
		end
	end
end

function removePlayersTime(frompos, topos)
	for x = frompos.x, topos.x  do
		for y = frompos.y, topos.y do
			local remove, clean = true, true
			local pos = {x = x, y = y, z = frompos.z}
			local m = getTopCreature(pos).uid
			if m ~= 0 and isPlayer(m) then
				doTeleportThing(m, getTownTemplePosition(1))
			end
		end
	end
	return true
end

function getTimeString(self)
	local format = {
		{'day', self / 60 / 60 / 24},
		{'hour', self / 60 / 60 % 24},
		{'minute', self / 60 % 60},
		{'second', self % 60}
	}

	local out = {}
	for k, t in ipairs(format) do
		local v = math.floor(t[2])
		if(v > 0) then
			table.insert(out, (k < #format and (#out > 0 and ', ' or '') or ' e ') .. v .. ' ' .. t[1] .. (v ~= 1 and 's' or ''))
		end
	end
	local ret = table.concat(out)
	if ret:len() < 16 and ret:find('second') then
		local a, b = ret:find(' e ')
		ret = ret:sub(b+1)
	end

	return ret
end

function formatTime(t, short)
	local str = ""
	local hour = math.floor(t / 3600)
	local min = math.floor(t / 60) % 60
	local sec = math.floor(t % 60)
	if (hour ~= 0) then
		str = hour .. (short and "h" or " h")..(short and "" or "ora")..(short and "" or (hour > 1 and "s" or "")) .. ", "
	end

	if (min ~= 0) then
		str = str .. min .. (short and "m" or " m")..(short and "" or "inuto") .. (short and "" or (min > 1 and "s" or "")) .. ", "
	end

	if (sec ~= 0) then
		str = str .. sec .. (short and "s" or " s")..(short and "" or "egundo") .. (short and "" or(sec > 1 and "s" or "")).. ", "
	end
	return str ~= "" and str:sub(1, #str - 2):gsub("(.+), (.+)", "%1 e %2", 1) or ""
end

function isPlayerOnline(name)
	local queryResult = db.storeQuery("SELECT `online` FROM `players` WHERE `name` = '"..name.."'")
	local result = result.getDataInt(queryResult, "online") > 0 and true or false
	return result
end

function getOfflinePlayerStorage(guid, storage)
	if not isPlayerOnline(getPlayerNameByGUID(guid)) then
		local queryResult = db.storeQuery("SELECT `value` FROM `player_storage` WHERE `key` = '"..storage.."' and `player_id` = "..guid.."")
		local result = queryResult and result.getDataInt(queryResult, "value") or -1
		return result
	end
end

function setOfflinePlayerStorage(guid, storage, value)
	if not isPlayerOnline(getPlayerNameByGUID(guid)) then
		db.query("UPDATE `player_storage` SET `value` = '"..value.."' WHERE `key` = '"..storage.."' and `player_id` = "..guid.."")
	end
end

function getItemNameByCount(itemID, count)
	if tonumber(count) and count > 1 and isItemStackable(itemID) then
		return getItemInfo(itemID).plural
	end
	return getItemNameById(itemID)
end

function printTable(_table)
	local function getTable(_table, expand, tabs)
		local aux = ""
		if not type(_table) == "table" then
			return _table
		else
			for key,value in pairs(_table) do
				if type(value) == "table" then
				
					for i = 1, tabs -1 do
						aux = aux.."\t"
					end
					
					if type(key)  == "string" then
						aux = aux.. '["'..key..'"] =\t{ \n'..getTable(value, true, tabs +1)
					else
						aux = aux.. "["..key.."] =\t{ \n"..getTable(value, true, tabs +1)
					end
					
					for i = 1, tabs do
						aux = aux.. "\t"
					end
					
					aux = aux.."},\n"				
				else
					if expand then
						for i = 1, tabs -1 do
							aux = aux.. "\t"
						end
					end
					if type(key)  == "string" then
						aux = aux.. '["'..key..'"] = '..(type(value) == "string" and '"'..value..'"' or tostring(value))..",\n"
					else
						aux = aux.. '['..key..'] = '..(type(value) == "string" and '"'..value..'"' or tostring(value))..",\n"
					end
				end
			end
		end

		return aux
	end
	if type(_table) == "table" then
		print(getTable(_table, false, 1))
		return true
	else
		error("Parameter is not a table!")
		return false
	end
end

function canPlayerReceiveItem(cid, itemId, ammount)
	local weight = getItemWeightById(itemId, ammount, true)
	if isItemStackable(itemId) then
		ammount = math.ceil(ammount / 100)
	end

	local freeCap = getPlayerFreeCap(cid)
	local freeSlots = 0
	if weight <= freeCap then
		local backpack = getPlayerSlotItem(cid, CONST_SLOT_BACKPACK)
		if backpack.uid > 0 and isContainer(backpack.uid) then
			return getContainerFreeSlots(backpack.uid) >= ammount
		end
	end

	return false
end

function getContainerFreeSlots(container)
	local size = getContainerSize(container)
	local slots = getContainerCap(container) - size
	for i = 0, size - 1 do
		local item = getContainerItem(container, i)
		if isContainer(item.uid) then
			slots = slots + getContainerFreeSlots(item.uid)
		end
	end

	return slots
end

function convertTimeString(a)
	if(type(tonumber(a)) == "number" and a > 0) then
		if (a <= 3599) then
			local minute = math.floor(a/60)
			local second = a - (60 * minute)
			if(second == 0) then
				return ((minute)..((minute > 1) and " minutes" or " minute"))
			else
				return ((minute ~= 0) and ((minute>1) and minute.." minutes and " or minute.." minute and ").. ((second>1) and second.." seconds" or second.." second") or ((second>1) and second.." seconds" or second.. " second"))
			end
		else
			local hour = math.floor(a/3600)
			local minute = math.floor((a - (hour * 3600))/60)
			local second = (a - (3600 * hour) - (minute * 60))
			if (minute == 0 and second > 0) then
				return (hour..((hour > 1) and " hours and " or " hour and "))..(second..((second > 1) and " seconds" or " second"))
			elseif (second == 0 and minute > 0) then
				return (hour..((hour > 1) and " hours and " or " hour and "))..(minute..((minute > 1) and " minutes" or " minute"))
			elseif (second == 0 and minute == 0) then
				return (hour..((hour > 1) and " hours" or " hour"))
			end

			return (hour..((hour > 1) and " hours, " or " hour, "))..(minute..((minute > 1) and " minutes and " or " minute and "))..(second..((second > 1) and " seconds" or " second"))
		end
	end
end

function convertTimeNumber(a)
	if(type(tonumber(a)) == "number" and a > 0) then
		if (a <= 3599) then
			local minute = math.floor(a/60)
			local second = a - (60 * minute)
			if(second == 0) then
				return ((minute < 10) and "0"..minute..":00" or minute..":00")
			else
				return ((minute ~= 0) and ((minute<10) and "0"..minute..":" or minute..":").. ((second<10) and "0"..second or second) or ((second<10) and "0"..second or second))
			end
		else
			local hour = math.floor(a/3600)
			local minute = math.floor((a - (hour * 3600))/60)
			local second = (a - (3600 * hour) - (minute * 60))
			if (minute == 0 and second > 0) then
				return ((hour < 10) and "0"..hour..":" or hour..":").."00:"..((second < 10) and "0"..second or second)
			elseif (second == 0 and minute > 0) then
				return ((hour < 10) and "0"..hour..":" or hour..":")..((minute < 10) and "0"..minute..":" or minute..":").."00"
			elseif (second == 0 and minute == 0) then
				return ((hour < 10) and "0"..hour..":" or hour..":").."00:00"
			end

			return ((hour < 10) and "0"..hour..":" or hour..":")..((minute < 10) and "0"..minute..":" or minute..":")..((second < 10) and "0"..second or second)
		end
	end
end

function mathtime(table)
local unit = {"sec", "min", "hour", "day"}
	for i, v in pairs(unit) do
		if v == table[2] then
			return table[1]*(60^(v == unit[4] and 2 or i-1))*(v == unit[4] and 24 or 1)
		end
	end

	return error("Bad declaration in mathtime function.")
end

function countDownTeleport(position, duration)
	if (duration == 0) then
		return true
	end

	local minute = duration >= 60
	local dateParameter = minute and "%M:%S" or "%S"
	local color = minute and COLOR_WHITE or COLOR_RED
	doSendAnimatedText(position, os.date(dateParameter, duration), color)
	doSendMagicEffect(position, minute and CONST_ME_MAGIC_BLUE or CONST_ME_MAGIC_RED)

	addEvent(countDownTeleport, 1000, position, duration - 1)
end

function getCreaturesFromArea(fromPos, toPos, onlyPlayers)
	local creatureInArea = {}
	for posx = fromPos.x, toPos.x do
		for posy = fromPos.y, toPos.y do
			for posz = fromPos.z, toPos.z do
				local tmp = getTopCreature({x=posx,y=posy,z=posz}).uid
				if(tmp > 0) then
					for s = 0, 255 do
						search_pos = {x = posx, y = posy, z = posz, stackpos = s}
						tmpCreature = getThingfromPos(search_pos).uid
						if tmpCreature > 0 and isCreature(tmpCreature) then
							if (onlyPlayers and isPlayer(tmpCreature) and not isSummon(tmpCreature)) or (not onlyPlayers and not isSummon(tmpCreature)) then
								if not isInArray(creatureInArea, tmpCreature) then
									table.insert(creatureInArea, tmpCreature)
								end
							end
						end
					end
				end
			end
		end
	end

	return creatureInArea
end

function getMonstersFromArea(fromPos, toPos)
	local monsterInArea = {}
	for posx = fromPos.x, toPos.x do
		for posy = fromPos.y, toPos.y do
			for posz = fromPos.z, toPos.z do
				local tmp = getTopCreature({x=posx,y=posy,z=posz}).uid
				if(tmp > 0) then
					for s = 0, 255 do
						_search_pos = {x = posx, y = posy, z = posz, stackpos = s}
						tmpMonster = getThingfromPos(_search_pos).uid
						if tmpMonster > 0 and isMonster(tmpMonster) and not isSummon(tmpMonster) and not isInArray(monsterInArea, tmpMonster) then
							table.insert(monsterInArea, tmpMonster)
						end
					end
				end
			end
		end
	end

	return monsterInArea
end

function doPlayerReceiveParcel(name, town, items, extraTextLabel, id) 
	if (getPlayerGUIDByName(name) ~= 0) then
		local parcel = doCreateItemEx(2595)
		local label = doAddContainerItem(parcel, 2599)
		doSetItemText(label, name.."\n"..getTownName(town))
		if (extraTextLabel ~= "") then
			local letter = doAddContainerItem(parcel, id)
			doSetItemText(letter, extraTextLabel)
		end

		for i = 1, #items do
			if type(items[i]) == 'table' then
				local tempitem = doAddContainerItem(parcel, tonumber(items[i].id), tonumber(items[i].count) or 1)
			else
				doAddContainerItem(parcel, items[i], 1)
			end
		end

		doPlayerSendMailByName(name, parcel, town)
	else
		debugPrint("doPlayerSendParcel: Player "..name.." not found.")
	end
end

function isWalkable(pos, checkCreatures, checkStairs, checkPZ, checkFields)
	if getTileThingByPos({x = pos.x, y = pos.y, z = pos.z, stackpos = 0}).itemid == 0 then return false end
	if checkCreatures and getTopCreature(pos).uid > 0 then return false end
	if checkPZ and getTilePzInfo(pos) then return false end
	for i = 0, 255 do
		pos.stackpos = i
		local tile = getTileThingByPos(pos)
		if tile.itemid ~= 0 and not isCreature(tile.uid) then
			if hasProperty(tile.uid, CONST_PROP_BLOCKPROJECTILE) or hasProperty(tile.uid, CONST_PROP_IMMOVABLEBLOCKSOLID) or (hasProperty(tile.uid, CONST_PROP_BLOCKPATHFIND) and not((not checkFields and getTileItemByType(pos, ITEM_TYPE_MAGICFIELD).itemid > 0) or hasProperty(tile.uid, CONST_PROP_HASHEIGHT) or hasProperty(tile.uid, CONST_PROP_FLOORCHANGEDOWN) or hasProperty(tile.uid, CONST_PROP_FLOORCHANGEUP))) then
				return false
			elseif checkStairs then
				if hasProperty(tile.uid, CONST_PROP_FLOORCHANGEDOWN) or hasProperty(tile.uid, CONST_PROP_FLOORCHANGEUP) then
					return false
				end
			end
		else
			break
		end
	end
	return true
end

function Split(s, delimiter)
	result = {};
	for match in (s..delimiter):gmatch("(.-)"..delimiter) do
		table.insert(result, match);
	end
	return result;
end

function getPlayerNameById(id)
	local resultName = db.storeQuery("SELECT `name` FROM `players` WHERE `id` = " .. db.escapeString(id))
	if resultName ~= false then
		local name = result.getDataString(resultName, "name")
		result.free(resultName)
		return name
	end
	return 0
end

function getPlayerIdByName(name)
	local resultID = db.storeQuery("SELECT `id` FROM `players` WHERE `name` = " .. db.escapeString(name))
	if resultID ~= false then
		local id = result.getDataString(resultID, "id")
		result.free(resultID)
		return id
	end
	return 0
end

function getPlayerID(cid)
	return getPlayerIdByName(getPlayerName(cid))
end

function getAccountPoints(cid)
	local ret = 0
	local res = db.getResult('select `premium_points` from accounts where name = \''..getPlayerAccount(cid)..'\'')
	if(res:getID() == -1) then
		return false
	else
		ret = res:getDataInt("premium_points")
		res:free()
	end
	return tonumber(ret)
end

function doAccountAddPoints(cid, count)
	return db.query("UPDATE `accounts` SET `premium_points` = '".. getAccountPoints(cid) + count .."' WHERE `name` ='"..getPlayerAccount(cid).."'")
end

function doAccountRemovePoints(cid, count)
	return db.query("UPDATE `accounts` SET `premium_points` = '".. getAccountPoints(cid) - count .."' WHERE `name` ='"..getPlayerAccount(cid).."'")
end

function getItemPeriod(uid)
	local period = doItemGetPeriod(uid)
	if period > 0 then	
		 period = doItemGetPeriod(uid) - os.time()
	end
	return period
end

function getItemsInContainerById(container, itemid) -- Function By Kydrai
	local items = {}
	if isContainer(container) and getContainerSize(container) > 0 then
		for slot=0, (getContainerSize(container)-1) do
			local item = getContainerItem(container, slot)
			if isContainer(item.uid) then
				local itemsbag = getItemsInContainerById(item.uid, itemid)
				for i=0, #itemsbag do
					table.insert(items, itemsbag[i])
				end
			else
				if itemid == item.itemid then
					table.insert(items, item.uid)
				end
			end
		end
	end

	return items
end

--Do not use this script below to check VPS without knowing how to use server line or threads
----------------------------------------------------
local blockeByConfig = getConfigValue('blockedVps') --{"google","amazon","amazon.com","oracle","vultr","azure","google.com"}
local blockeds = {}
for element in blockeByConfig:gmatch("([^;]+)") do
	table.insert(blockeds, element)
end
-----------------------------------------------------
function isInVPS(cid)
	local http = require("socket.http")
	local json = require("json")
	local ipAddress = doConvertIntegerToIp(getPlayerIp(cid))
	local requestUrl = "http://ipinfo.io/" .. ipAddress .. "/json"
	local response, err = http.request(requestUrl)
	local jsonResponse = json.decode(response)
	local org = jsonResponse.org or "Unknown"
	--print("Internet provider: " .. org)
	--print("ip: ".. ipAddress)
	local s = string.explode(org, " ")
	for i = 1, #s do
		if isInArray(blockeds, string.lower(s[i])) then
			return true
		end
	end

	return false
end

function upgradeGuildLevel(cid, guildId)
	local func = db.query or db.executeQuery
	if guildId and guildId ~= 0 then
		local level = 0
		local result = db.getResult('SELECT `guild_level` FROM `guilds` WHERE `id` = '..guildId)
		if result:getID() ~= -1 then
			level = tonumber(result:getDataInt("guild_level"))
			result:free()
		end
		if level < 10 then
			func("UPDATE `guilds` set `guild_level` = guild_level + 1 WHERE `id` = "..guildId)
			doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "Congratulations!! Your guild leveled up\n[Level current]: "..(level+1))
			return true
		else
			doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "Your Guild is already at maximum level.")
			return false
		end
	else
		doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "You don't have a guild!")
		return false
	end
	
	return false
end

function getExclusiveCoins(cid)
	local ret = 0
	local res = db.getResult('select `exclusive_coin` from accounts where name = \''..getPlayerAccount(cid)..'\'')
	if(res:getID() == -1) then
		return false
	else
		ret = res:getDataInt("exclusive_coin")
		res:free()
	end
	return tonumber(ret)
end

function setExclusiveCoins(cid, count)
	return db.query("UPDATE `accounts` SET `exclusive_coin` = '".. getExclusiveCoins(cid) + count .."' WHERE `name` ='"..getPlayerAccount(cid).."'")
end

function removeExclusivePoints(cid, count)
	return db.query("UPDATE `accounts` SET `exclusive_coin` = '".. getExclusiveCoins(cid) - count .."' WHERE `name` ='"..getPlayerAccount(cid).."'")
end

function giveRewardTable(cid, tableReward, parcelitem)
	if type(tableReward) ~= "table" then
		doPlayerSendTextMessage(cid, MESSAGE_FIRST, "Please contact administrador with message '/check giveRewardTable(cid, tableReward) reward not is a table/' ")
		return "notTable"
	end
	if #tableReward < 1 then
		return true
	end
	if not parcelitem then parcelitem = ITEM_PARCEL end
	local parcel = doCreateItemEx(parcelitem)
	for i=1, #tableReward do
		if not isItemStackable(tableReward[i][1]) then
			if tableReward[i][2] > 1 then
				for j=1, tableReward[i][2] do
					local itemCreated = doCreateItemEx(tableReward[i][1], 1)
					doAddContainerItemEx(parcel, itemCreated)
				end
			else
				local itemCreated = doCreateItemEx(tableReward[i][1], 1)
					doAddContainerItemEx(parcel, itemCreated)
			end
		else
			local itemCreated = 0
			if tableReward[i][2] > 100 then
				local flagCem = tableReward[i][2]
				while flagCem > 0 do
					itemCreated = doCreateItemEx(tableReward[i][1], flagCem >= 100 and 100 or flagCem)
					doAddContainerItemEx(parcel, itemCreated)
					flagCem = flagCem - 100
				end
			else
				itemCreated = doCreateItemEx(tableReward[i][1], tableReward[i][2])
				doAddContainerItemEx(parcel, itemCreated)
			end
		end
	end

	return doPlayerAddItemEx(cid, parcel, true) == RETURNVALUE_NOERROR
end

function getPlayerWeaponHand(cid)
	if isCreature(cid) and isPlayer(cid) then
		local left = getPlayerSlotItem(cid, CONST_SLOT_LEFT)
		if left and left.uid ~= 0 and isWeapon(left.uid) and not isShield(left.uid) then
			return left
		else
			local right = getPlayerSlotItem(cid, CONST_SLOT_RIGHT)
			if right and right.uid ~=0 and isWeapon(right.uid) and not isShield(right.uid) then
				return right
			end
		end
	end
	return nil
end

function warnPlayersWithStorage(storage, value, class, message)
	if not value then
		value = 1
	end

	if not class then
		class = MESSAGE_SATUS_CONSOLE_WARNING
	end

	if not storage or not message then
		return
	end

	if #getPlayersOnline() == 0 then
		return
	end

	for _, pid in pairs(getPlayersOnline()) do
		if getPlayerStorageValue(pid, storage) == value then
			doPlayerSendTextMessage(pid, class, message)
		end

		if getPlayerAccess(pid) >= 4 then	
			doPlayerSendTextMessage(pid, class, "Message to those with storage "..storage..message) -- Gms will always receive the messages
		end
	end
end

function getPlayerStorageZero(cid, storage)
	local sto = getPlayerStorageValue(cid, storage)
	return sto > 0 and sto or 0
end

function getPlayerIdByName(name)
	local resultID = db.storeQuery("SELECT `id` FROM `players` WHERE `name` = " .. db.escapeString(name))
	if resultID ~= false then
		local id = result.getDataString(resultID, "id")
		result.free(resultID)
		return id
	end
	return 0
end

function getStorageZero(storage)
	local sto = getGlobalStorageValue(storage)
	return sto > 0 and sto or 0
end

function countTable(table)
	local y = 0
	if type(table) == "table" then
		for _ in pairs(table) do
			y = y + 1
		end
		return y
	end

	return false
end

function getPlayersInArea(frompos, topos)
	local players_ = {}
	local count = 1
	for _, pid in pairs(getPlayersOnline()) do
		if isInArea(getCreaturePosition(pid), frompos, topos) then
			players_[count] = pid
			count = count + 1
		end
	end

	return countTable(players_) > 0 and players_ or false
end

function getMonstersInArea(pos1,pos2)
	local monsters = {}
	if pos1.x and pos1.y and pos2.x and pos2.y and pos1.z == pos2.z then
		for a = pos1.x, pos2.x do
			for b = pos1.y,pos2.y do
				local pos = {x=a,y=b,z=pos1.z}
				if isMonster(getTopCreature(pos).uid) and not isSummon(getTopCreature(pos).uid) then
					table.insert(monsters,getTopCreature(pos).uid)
				end
			end
		end
		return monsters
	else
		return false
	end
end

function getContainerItemsInfo(containerUid)
	local table = {}
	if containerUid and containerUid > 0 then
		local a = 0   
		for i = 0, getContainerSize(containerUid) - 1 do
			local item = getContainerItem(containerUid,i)
			a = a + 1
			table[a] = {uid = item.uid, itemid = item.itemid, quant = item.type}
		end
		return table
	end

	return false
end

function getTableEqualValues(table)
	local ck = {}
	local eq = {}
	if type(table) == "table" then
		if countTable(table) and countTable(table) > 0 then
			for i = 1, countTable(table) do
				if not isInArray(ck, table[i]) then
					ck[i] = table[i]
				else
					eq[i] = table[i]
				end
			end
		return countTable(eq) > 0 and eq or 0
		end
	end

	return false
end

function killuaGetItemLevel(uid)
	local name = getItemName(uid)
	local pos = 0
	for i = 1, #name do
		if string.byte(name:sub(i,i)) == string.byte('+') then
			pos = i + 1
			break
		end
	end

	return tonumber(name:sub(pos,pos))
end

k_table_storage_lib = {
	filtrateString = function(str)
		local tb, x, old, last = {}, 0, 0, 0
		local first, second, final = 0, 0, 0
		if type(str) ~= "string" then
			return tb
		end
		for i = 2, #str-1 do
			if string.byte(str:sub(i,i)) == string.byte(':') then
				x, second, last = x+1, i-1, i+2
				for t = last,#str-1 do
					if string.byte(str:sub(t,t)) == string.byte(',') then
						first = x == 1 and 2 or old
						old, final = t+2, t-1
						local index, var = str:sub(first,second), str:sub(last,final)
						tb[tonumber(index) or tostring(index)] = tonumber(var) or tostring(var)
						break
					end
				end
			end
		end
		return tb
	end,

	translateIntoString = function(tb)
		local str = ""
		if type(tb) ~= "table" then
			return str
		end
		for i, t in pairs(tb) do
			str = str..i..": "..t..", "
		end
		str = "a"..str.."a"
		return tostring(str)
	end
}

function setPlayerTableStorage(cid, key, value)
	return doPlayerSetStorageValue(cid, key, k_table_storage_lib.translateIntoString(value))
end

function getPlayerTableStorage(cid, key)
	return k_table_storage_lib.filtrateString(getPlayerStorageValue(cid, key))
end

function setGlobalTableStorage(key, value)
	return setGlobalStorageValue(key, k_table_storage_lib.translateIntoString(value))
end

function getGlobalTableStorage(key)
	return k_table_storage_lib.filtrateString(getGlobalStorageValue(key))
end

function printTableKillua(table, includeIndices,prnt)
	if includeIndices == nil then
		includeIndices = true
	end

	if prnt == nil then
		prnt = true
	end

	if type(table) ~= "table" then
		error("Argument must be a table")
		return
	end

	local str, c = "{", ""
	for v, b in pairs(table) do
		if type(b) == "table" then
			str = includeIndices and str..c.."["..v.."]".." = "..printTable(b,true,false) or str..c..printTable(b,false,false)
		else
			str = includeIndices and str..c.."["..v.."]".." = "..b or str..c..b
		end
		c = ", "
	end
	str = str.."}"

	if prnt then
		print(str)
	end

	return str
end

function checkString(str)
	local check = true
	for i = 1, #str do
		local letra = string.byte(str:sub(i,i))
		if letra >= string.byte('a') and letra <= string.byte('z') or letra >= string.byte('A') and letra <= string.byte('Z') or letra >= string.byte('0') and letra <= string.byte('9') then
			check = true
		else
			check = false
			break
		end
	end
	return check
end

function isArmor(itemid)
	return getItemInfo(itemid).armor > 0
end

function isWeapon(uid)
	return getItemWeaponType(uid) ~= 0
end

function isShield(uid)
	return getItemWeaponType(uid) == 5
end

function isSword(uid)
	return getItemWeaponType(uid) == 1
end

function isClub(uid)
	return getItemWeaponType(uid) == 2
end

function isAxe(uid)
	return getItemWeaponType(uid) == 3
end

function isBow(uid)
	return getItemWeaponType(uid) == 4
end

function isWand(uid)
	return getItemWeaponType(uid) == 7
end

--[[
	-- IF YOU KNOW HOW TO USE THE SIMPLIFIED MODE, USE BELOW, IF NOT, USE THE STANDARD TABLE
	ResetSystem = {
		back_to_level = 8, -- level that will return
		use_back_level = true,
		resets = {},
		incrementToReset = 250, -- What is the difference in level for each?
		firstResetAt = 1500,	-- what is the level of the FIRST
		
		-- incrementToReset se for 250 e firstResetAt = 1000, resets will be 1000/1250/1500/1750
	}

	for i = 0, 100 do
		local level = i == 0 and 0 or (i == 1 and ResetSystem.firstResetAt or (i - 1) * ResetSystem.incrementToReset + ResetSystem.firstResetAt)
		local exp_percent = ((i - 1) % 10 == 0 and i ~= 0) and 10 or (i % 10) * 10
		if exp_percent == 0 and i > 0 then
			exp_percent = 100
		end

		local damage_percent = i == 1 and 4 or (i == 0 and 0 or (i * 2 + 2))
		local hpmp_percent = i * 15
		ResetSystem.resets[i] = {needed_level = level, exp_percent = exp_percent, damage_percent = damage_percent, hpmp_percent = hpmp_percent}
	end
]]

ResetSystem = {
	back_to_level = 8, -- level that will return
	use_back_level = true,
	resets = {
		[1] =  {needed_level = 1000,  exp_percent = 2,   damage_percent = 2,   hpmp_percent = 10},
		[2] =  {needed_level = 2000,  exp_percent = 4,   damage_percent = 4,   hpmp_percent = 20},
		[3] =  {needed_level = 3000,  exp_percent = 6,   damage_percent = 6,   hpmp_percent = 30},
		[4] =  {needed_level = 4000,  exp_percent = 8,   damage_percent = 8,   hpmp_percent = 40},
		[5] =  {needed_level = 5000,  exp_percent = 10,  damage_percent = 10,  hpmp_percent = 50},
		[6] =  {needed_level = 6000,  exp_percent = 12,  damage_percent = 12,  hpmp_percent = 60},
		[7] =  {needed_level = 7000,  exp_percent = 14,  damage_percent = 14,  hpmp_percent = 70},
		[8] =  {needed_level = 8000,  exp_percent = 16,  damage_percent = 16,  hpmp_percent = 80},
		[9] =  {needed_level = 9000,  exp_percent = 18,  damage_percent = 18,  hpmp_percent = 90},
		[10] = {needed_level = 10000, exp_percent = 20,  damage_percent = 20,  hpmp_percent = 100},
	}
}

function ResetSystem:getCount(pid)
	return getPlayerResets(pid)
end

function ResetSystem:setCount(pid, value)
	setPlayerResets(pid, value)
end

function ResetSystem:addCount(pid)
	self:setCount(pid, self:getCount(pid) + 1)
end

function ResetSystem:getInfo(pid)
	return self.resets[math.min(self:getCount(pid), #self.resets)]
end

function ResetSystem:applyBonuses(pid)
	local bonus = self:getInfo(pid)
	if (bonus and bonus.damage_percent) then
		setPlayerDamageMultiplier(pid, 1.0 + (bonus.damage_percent / 100.0))
	else
		setPlayerDamageMultiplier(pid, 1.0)
	end
end

function ResetSystem:updateHealthAndMana(pid)
	local bonus = self:getInfo(pid)
	if (bonus and bonus.hpmp_percent) then
		local vocationInfo = getVocationInfo(getPlayerVocation(pid))
		if (vocationInfo) then
			local oldMaxHealth = getCreatureMaxHealth(pid)
			local oldMaxMana = getCreatureMaxMana(pid)
			local level = getPlayerLevel(pid)
			local totalHealth = (185 - vocationInfo.healthGain * 8) + (vocationInfo.healthGain * level)
			local totalMana = (185 - vocationInfo.manaGain * 8) + (vocationInfo.manaGain * level)
			local newMaxHealth = math.floor(totalHealth + (totalHealth * (bonus.hpmp_percent / 100)))
			local newMaxMana = math.floor(totalMana + (totalMana * (bonus.hpmp_percent / 100)))
			setCreatureMaxHealth(pid, newMaxHealth)
			setCreatureMaxMana(pid, newMaxMana)

			if (newMaxHealth > oldMaxHealth) then
				doCreatureAddHealth(pid, newMaxHealth - oldMaxHealth)
			elseif (newMaxHealth < oldMaxHealth) then
				doCreatureAddHealth(pid, 1) -- avoids buggy bar
			end

			if (newMaxMana > oldMaxMana) then
				doCreatureAddMana(pid, newMaxMana - oldMaxMana)
			elseif (newMaxMana < oldMaxMana) then
				doCreatureAddMana(pid, 1)
			end
		end
	end
end

function ResetSystem:execute(pid)
	local playerLevel = getPlayerLevel(pid)
	if (playerLevel > self.back_to_level and self.use_back_level) then
		doPlayerAddExperience(pid, getExperienceForLevel(self.back_to_level) - getPlayerExperience(pid))
		playerLevel = self.back_to_level
	end

	self:addCount(pid)
	self:applyBonuses(pid)
	self:updateHealthAndMana(pid)
	local bonus = self:getInfo(pid)
	if (bonus) then
		local message = "You made your " .. self:getCount(pid) .. "° reset."
		if (bonus.damage_percent) then
			message = message .. "\n+" .. bonus.damage_percent .. "% of damage"
		end

		if (bonus.hpmp_percent) then
			message = message .. "\n+" .. bonus.hpmp_percent .. "% of life and mana"
		end

		if (bonus.exp_percent) then
			message = message .. "\n+" .. bonus.exp_percent .. "% of EXP"
		end
		doPlayerSendTextMessage(pid, MESSAGE_EVENT_ADVANCE, message)
	end
end
