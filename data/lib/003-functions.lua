-- Basic --
function playerExists(name)
	local resultId = db.storeQuery('SELECT `name` FROM `players` WHERE `name` = ' .. db.escapeString(name))
	if resultId then
		result.free(resultId)
		return true
	end
	return false
end

function isValidMoney(money)
	return isNumber(money) and money > 0 and money < 4294967296
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

function teleportAllPlayersFromArea(fromArea, toPos)
	for x = fromArea[1].x, fromArea[2].x do
		for y = fromArea[1].y, fromArea[2].y do
			for z = fromArea[1].z, fromArea[2].z do
				local creature = Tile(Position(x, y, z)):getTopCreature()
				if creature and creature:isPlayer() then
					creature:teleportTo(toPos)
					toPos:sendMagicEffect(CONST_ME_TELEPORT)
					creature:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You were teleported out by the gnomish emergency device.")
				end
			end
		end
	end
	return true
end

function removeBoss(fromArea, bossName)
	for x = fromArea[1].x, fromArea[2].x do
		for y = fromArea[1].y, fromArea[2].y do
			for z = fromArea[1].z, fromArea[2].z do
				local creature = Tile(Position(x, y, z)):getTopCreature()
				if creature and creature:isMonster() and creature:getName():lower() == bossName:lower() then
					creature:remove()
				end
			end
		end
	end
end

--[[Arena Quest
function clearArena(fromPos, toPos)
	-- local exitPosition = Position(33049, 31017, 2)
	for x = fromPos.x, toPos.x do
		for y = fromPos.y, toPos.y do
			for z = fromPos.z, toPos.z do
				local creature = Tile(x, y, z):getTopCreature()
				if creature then
					if creature:isPlayer() then
						creature:teleportTo(exitPosition)
						exitPosition:sendMagicEffect(CONST_ME_TELEPORT)
					else
						creature:remove()
					end
				end
			end
		end
	end
end
]]

-- Item --
function Item.setText(self, text)
	if text ~= '' then
		self:setAttribute(ITEM_ATTRIBUTE_TEXT, text)
	else
		self:removeAttribute(ITEM_ATTRIBUTE_TEXT)
	end
end

function Item.setDescription(self, description)
	if description ~= '' then
		self:setAttribute(ITEM_ATTRIBUTE_DESCRIPTION, description)
	else
		self:removeAttribute(ITEM_ATTRIBUTE_DESCRIPTION)
	end
end

-- Party --
function Party.getVocationCount(self)
	local count = 1
	local bits = bit.lshift(1, self:getLeader():getVocation():getBase():getId())

	local members = self:getMembers()
	for i = 1, #members do
		local vocationId = members[i]:getVocation():getBase():getId()
		local vocation = bit.lshift(1, vocationId)
		if bit.band(bits, vocation) == vocation then
			return false
		end

		bits = bit.bor(bits, bit.lshift(1, vocationId))
		count = count + 1
	end

	return count
end

-- Player --
function Player.allowMovement(self, allow)
	return self:setStorageValue(Storage.blockMovementStorage, allow and -1 or 1)
end

function Player.hasAllowMovement(self)
	return self:getStorageValue(Storage.blockMovementStorage) ~= 1
end

function Player.withdrawMoney(self, amount)
	local balance = self:getBankBalance()
	if amount > balance or not self:addMoney(amount) then
		return false
	end

	self:setBankBalance(balance - amount)
	return true
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

function Player.isPromoted(self)
	local vocation = self:getVocation()
	if vocation:getId() == 0 or vocation:getPromotion():getId() == 0 then
		return true
	end

	return false
end

-- Tile --
function Tile.relocateTo(self, toPosition)
	if self:getPosition() == toPosition then
		return false
	end

	if not Tile(toPosition) then
		return false
	end

	for i = self:getThingCount() - 1, 0, -1 do
		local thing = self:getThing(i)
		if thing then
			if thing:isItem() then
				if ItemType(thing:getId()):isMovable() then
					thing:moveTo(toPosition)
				end
			elseif thing:isCreature() then
				thing:teleportTo(toPosition)
			end
		end
	end
	return true
end

function Tile.isPz(self)
	return self:hasFlag(TILESTATE_PROTECTIONZONE)
end

function Tile.isHouse(self)
	local house = self:getHouse()
	return house and true or false
end

-- Vocation --
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
