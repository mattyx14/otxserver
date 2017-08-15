SPELL_BOOK = 2175

function getDistanceBetween(firstPosition, secondPosition)
	local xDif = math.abs(firstPosition.x - secondPosition.x)
	local yDif = math.abs(firstPosition.y - secondPosition.y)
	local posDif = math.max(xDif, yDif)
	if firstPosition.z ~= secondPosition.z then
		posDif = posDif + 15
	end
	return posDif
end

function getFormattedWorldTime()
	local worldTime = getWorldTime()
	local hours = math.floor(worldTime / 60)

	local minutes = worldTime % 60
	if minutes < 10 then
		minutes = '0' .. minutes
	end
	return hours .. ':' .. minutes
end

function getTibianTime()
	local worldTime = getWorldTime()
	local hours = math.floor(worldTime / 60)

	local minutes = worldTime % 60
	if minutes < 10 then
		minutes = '0' .. minutes
	end
	return hours .. ':' .. minutes
end

function doCreatureSayWithRadius(cid, text, type, radiusx, radiusy, position)
	if not position then
		position = Creature(cid):getPosition()
	end

	local spectators, spectator = Game.getSpectators(position, false, true, radiusx, radiusx, radiusy, radiusy)
	for i = 1, #spectators do
		spectator = spectators[i]
		spectator:say(text, type, false, spectator, position)
	end
end

function getBlessingsCost(level)
	if level <= 30 then
		return 2000
	elseif level >= 120 then
		return 20000
	else
		return (level - 20) * 200
	end
end

function getPvpBlessingCost(level)
	if level <= 30 then
		return 2000
	elseif level >= 270 then
		return 50000
	else
		return (level - 20) * 200
	end
end

function getItemAttribute(uid, key)
	local i = ItemType(Item(uid):getId())
	local string_attributes = {
		[ITEM_ATTRIBUTE_NAME] = i:getName(),
		[ITEM_ATTRIBUTE_ARTICLE] = i:getArticle(),
		[ITEM_ATTRIBUTE_PLURALNAME] = i:getPluralName(),
		["name"] = i:getName(),
		["article"] = i:getArticle(),
		["pluralname"] = i:getPluralName()
	}

	local numeric_attributes = {
		[ITEM_ATTRIBUTE_WEIGHT] = i:getWeight(),
		[ITEM_ATTRIBUTE_ATTACK] = i:getAttack(),
		[ITEM_ATTRIBUTE_DEFENSE] = i:getDefense(),
		[ITEM_ATTRIBUTE_EXTRADEFENSE] = i:getExtraDefense(),
		[ITEM_ATTRIBUTE_ARMOR] = i:getArmor(),
		[ITEM_ATTRIBUTE_HITCHANCE] = i:getHitChance(),
		[ITEM_ATTRIBUTE_SHOOTRANGE] = i:getShootRange(),
		["weight"] = i:getWeight(),
		["attack"] = i:getAttack(),
		["defense"] = i:getDefense(),
		["extradefense"] = i:getExtraDefense(),
		["armor"] = i:getArmor(),
		["hitchance"] = i:getHitChance(),
		["shootrange"] = i:getShootRange()
	}

	local attr = Item(uid):getAttribute(key)
	if tonumber(attr) then
		if numeric_attributes[key] then
			return attr ~= 0 and attr or numeric_attributes[key]
		end
	else
		if string_attributes[key] then
			if attr == "" then
				return string_attributes[key]
			end
		end
	end
	return attr
end

function doItemSetAttribute(uid, key, value)
	return Item(uid):setAttribute(key, value)
end

function doItemEraseAttribute(uid, key)
	return Item(uid):removeAttribute(key)
end

function isNumber(str)
	return tonumber(str) ~= nil
end

function isInRange(pos, fromPos, toPos)
	return pos.x >= fromPos.x and pos.y >= fromPos.y and pos.z >= fromPos.z and pos.x <= toPos.x and pos.y <= toPos.y and pos.z <= toPos.z
end

function getAccountNumberByPlayerName(name)
	local player = Player(name)
	if player ~= nil then
		return player:getAccountId()
	end

	local resultId = db.storeQuery("SELECT `account_id` FROM `players` WHERE `name` = " .. db.escapeString(name))
	if resultId ~= false then
		local accountId = result.getNumber(resultId, "account_id")
		result.free(resultId)
		return accountId
	end
	return 0
end

function getMoneyCount(string)
	local b, e = string:find("%d+")
	local money = b and e and tonumber(string:sub(b, e)) or -1
	if isValidMoney(money) then
		return money
	end
	return -1
end

function getMoneyWeight(money)
	local gold = money
	local crystal = math.floor(gold / 10000)
	gold = gold - crystal * 10000
	local platinum = math.floor(gold / 100)
	gold = gold - platinum * 100
	return (ItemType(2160):getWeight() * crystal) + (ItemType(2152):getWeight() * platinum) + (ItemType(2148):getWeight() * gold)
end

function getRealDate()
	local month = tonumber(os.date("%m", os.time()))
	local day = tonumber(os.date("%d", os.time()))

	if month < 10 then
		month = '0' .. month
	end
	if day < 10 then
		day = '0' .. day
	end
	return day .. '/' .. month
end

function getRealTime()
	local hours = tonumber(os.date("%H", os.time()))
	local minutes = tonumber(os.date("%M", os.time()))

	if hours < 10 then
		hours = '0' .. hours
	end
	if minutes < 10 then
		minutes = '0' .. minutes
	end
	return hours .. ':' .. minutes
end

function isValidMoney(money)
	return isNumber(money) and money > 0 and money < 4294967296
end

function iterateArea(func, from, to)
	for z = from.z, to.z do
		for y = from.y, to.y do
			for x = from.x, to.x do
				func(Position(x, y, z))
			end
		end
	end
end

function playerExists(name)
	local resultId = db.storeQuery('SELECT `name` FROM `players` WHERE `name` = ' .. db.escapeString(name))
	if resultId then
		result.free(resultId)
		return true
	end
	return false
end

function Tile.isPz(self)
	return self:hasFlag(TILESTATE_PROTECTIONZONE)
end

function Tile.isHouse(self)
	local house = self:getHouse()
	return not not house
end

function Position.getTile(self)
	return Tile(self)
end

function Player.isDruid(self)
	return table.contains({2, 6}, self:getVocation():getId())
end

function Player.isKnight(self)
	return table.contains({4, 8}, self:getVocation():getId())
end

function Player.isPaladin(self)
	return table.contains({3, 7}, self:getVocation():getId())
end

function Player.isMage(self)
	return table.contains({1, 2, 5, 6}, self:getVocation():getId())
end

function Player.isWarrior(self)
	return table.contains({3, 7, 4, 8}, self:getVocation():getId())
end

function Player.isSorcerer(self)
	return table.contains({1, 5}, self:getVocation():getId())
end

function Player.isPromoted(self)
	local vocation = self:getVocation()
	local promotedVocation = vocation:getPromotion()
	promotedVocation = promotedVocation and promotedVocation:getId() or 0

	return promotedVocation == 0 and vocation:getId() ~= promotedVocation
end

function Player.depositMoney(self, amount)
	if not self:removeMoney(amount) then
		return false
	end

	self:setBankBalance(self:getBankBalance() + amount)
	return true
end

function Player.transferMoneyTo(self, target, amount)
	local balance = self:getBankBalance()
	if amount > balance then
		return false
	end

	local targetPlayer = Player(target)
	if targetPlayer then
		targetPlayer:setBankBalance(targetPlayer:getBankBalance() + amount)
	else
		if not playerExists(target) then
			return false
		end
		db.query("UPDATE `players` SET `balance` = `balance` + '" .. amount .. "' WHERE `name` = " .. db.escapeString(target))
	end

	self:setBankBalance(self:getBankBalance() - amount)
	return true
end

function Player.withdrawMoney(self, amount)
	local balance = self:getBankBalance()
	if amount > balance or not self:addMoney(amount) then
		return false
	end

	self:setBankBalance(balance - amount)
	return true
end

function Item.setDescription(self, description)
	if description ~= '' then
		self:setAttribute(ITEM_ATTRIBUTE_DESCRIPTION, description)
	else
		self:removeAttribute(ITEM_ATTRIBUTE_DESCRIPTION)
	end
end

function Item.setText(self, text)
	if text ~= '' then
		self:setAttribute(ITEM_ATTRIBUTE_TEXT, text)
	else
		self:removeAttribute(ITEM_ATTRIBUTE_TEXT)
	end
end

function Item.setUniqueId(self, uniqueId)
	if type(uniqueId) ~= 'number' or uniqueId < 0 or uniqueId > 65535 then
		return false
	end

	self:setAttribute(ITEM_ATTRIBUTE_UNIQUEID, uniqueId)
end

function Game.getHouseByPlayerGUID(playerGUID)
	local houses, house = Game.getHouses()
	for i = 1, #houses do
		house = houses[i]
		if house:getOwnerGuid() == playerGUID then
			return house
		end
	end
	return nil
end

function Game.getPlayersByAccountNumber(accountNumber)
	local result = {}
	local players, player = Game.getPlayers()
	for i = 1, #players do
		player = players[i]
		if player:getAccountId() == accountNumber then
			result[#result + 1] = player
		end
	end
	return result
end

function Game.getPlayersByIPAddress(ip, mask)
	if not mask then mask = 0xFFFFFFFF end
	local masked = bit.band(ip, mask)
	local result = {}
	local players, player = Game.getPlayers()
	for i = 1, #players do
		player = players[i]
		if bit.band(player:getIp(), mask) == masked then
			result[#result + 1] = player
		end
	end
	return result
end

function Creature.getMonster(self)
	return self:isMonster() and self or nil
end

function Vocation.getBase(self)
	local demotion = self:getDemotion()
	while demotion do
		local tmp = demotion:getDemotion()
		if not tmp then
			return demotion
		end
		demotion = tmp
	end
	return self
end

string.split = function(str, sep)
	local res = {}
	for v in str:gmatch("([^" .. sep .. "]+)") do
		res[#res + 1] = v
	end
	return res
end

string.trim = function(str)
	return str:match'^()%s*$' and '' or str:match'^%s*(.*%S)'
end

string.starts = function(str, substr)
	return string.sub(str, 1, #substr) == substr
end

string.titleCase = function(str)
	return str:gsub("(%a)([%w_']*)", function(first, rest) return first:upper() .. rest:lower() end)
end

table.append = table.insert
table.empty = function (t)
	return next(t) == nil
end

table.find = function (table, value)
	for i, v in pairs(table) do
		if(v == value) then
			return i
		end
	end

	return nil
end

table.contains = function (txt, str)
	for i, v in pairs(str) do
		if(txt:find(v) and not txt:find('(%w+)' .. v) and not txt:find(v .. '(%w+)')) then
			return true
		end
	end

	return false
end
table.isStrIn = table.contains

table.count = function (table, item)
	local count = 0
	for i, n in pairs(table) do
		if(item == n) then
			count = count + 1
		end
	end

	return count
end
table.countElements = table.count

table.getCombinations = function (table, num)
	local a, number, select, newlist = {}, #table, num, {}
	for i = 1, select do
		a[#a + 1] = i
	end

	local newthing = {}
	while(true) do
		local newrow = {}
		for i = 1, select do
			newrow[#newrow + 1] = table[a[i]]
		end

		newlist[#newlist + 1] = newrow
		i = select
		while(a[i] == (number - select + i)) do
			i = i - 1
		end

		if(i < 1) then
			break
		end

		a[i] = a[i] + 1
		for j = i, select do
			a[j] = a[i] + j - i
		end
	end

	return newlist
end

function table.serialize(x, recur)
	local t = type(x)
	recur = recur or {}

	if(t == nil) then
		return "nil"
	elseif(t == "string") then
		return string.format("%q", x)
	elseif(t == "number") then
		return tostring(x)
	elseif(t == "boolean") then
		return t and "true" or "false"
	elseif(getmetatable(x)) then
		error("Can not serialize a table that has a metatable associated with it.")
	elseif(t == "table") then
		if(table.find(recur, x)) then
			error("Can not serialize recursive tables.")
		end
		table.append(recur, x)

		local s = "{"
		for k, v in pairs(x) do
			s = s .. "[" .. table.serialize(k, recur) .. "]"
			s = s .. " = " .. table.serialize(v, recur) .. ","
		end
		s = s .. "}"
		return s
	else
		error("Can not serialize value of type '" .. t .. "'.")
	end
end

function table.unserialize(str)
	return loadstring("return " .. str)()
end

function Player.setExhaustion(self, value, time)
	return self:setStorageValue(value, time + os.time())
end

function Player.getExhaustion(self, value)
	local storage = self:getStorageValue(value)
	if storage <= 0 then
		return 0
	end

	return storage - os.time()
end

-- The following 2 functions can be used for delayed shouted text
function say(param)
	selfSay(text)
	doCreatureSay(param.cid, param.text, 1)
end

function delayedSay(text, delay)
	local delay = delay or 0
	local cid = getNpcCid()
	addEvent(say, delay, {cid = cid, text = text})
end
