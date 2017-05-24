PreySystem = {
	Developer = "Charles (Cjaker)",
	Version = "1.0",
	LastUpdate = "24/05/2017 - 05:32 (AM)"
}

local RerollStorages = {
	[0] = 8420390,
	[1] = 8420391,
	[2] = 8420392
}

local ColumnId = {
	[512] = 0,
	[513] = 1,
	[514] = 2
}

local ServerPackets = {
	ShowDialog = 0xED,
	PreyRerollPrice = 0xE9,
	PreyData = 0xE8,
	PreyTimeLeft = 0xE7
}

local ClientPackets = {
	RequestData = 0xED,
	PreyAction = 0xEB
}

local monstersGenerated = {}

function getUnlockedColumn(player)
	local resultId = db.storeQuery("SELECT * FROM players WHERE name = " ..db.escapeString(player:getName()))
	if (resultId ~= false) then
		return result.getDataInt(resultId, "prey_column")
	end

	return nil
end

local function getMonsterList(player, column)
	local resultId = db.storeQuery("SELECT * FROM player_prey WHERE player_id = " ..player:getGuid())
	if (resultId ~= false) then
		if (monstersGenerated[player:getGuid()] == nil) then
			monstersGenerated[player:getGuid()] = {}
		end

		local preyMonsters = {}
		repeat
			local mName = result.getDataString(resultId, "name")
			local mIndex = result.getDataInt(resultId, "mindex")
			local mColumn = result.getDataInt(resultId, "mcolumn")
			if (mColumn == column) then
				preyMonsters[#preyMonsters+1] = {Name = mName, Index = mIndex, Column = mColumn}
				table.insert(monstersGenerated[player:getGuid()], mName)
			end
		until not result.next(resultId)
		result.free(resultId)

		if (#preyMonsters == 0) then
			monstersGenerated[player:getGuid()] = {}
			return nil
		end

		return preyMonsters
	end

	return nil
end

local function tableContains(table, value)
	for i = 1, #table do
		if (table[i] == value) then
			return true
		end
	end

	return false
end

local function createMonstersColumn(player, column)
	local monsters, newTable = getPreyMonsters(), {}
	local count = 1

	if (monstersGenerated[player:getGuid()] == nil) then
		monstersGenerated[player:getGuid()] = {}
	end

	-- Get New List
	repeat
		local randomName = monsters[math.random(#monsters)]
		if (not tableContains(monstersGenerated[player:getGuid()], randomName)) then
			if (MonsterType(randomName) and 
				MonsterType(randomName):getOutfit().lookType > 0) then
				newTable[count] = {Name = randomName, Index = count, Column = column}
				table.insert(monstersGenerated[player:getGuid()], randomName)
				count = count + 1
			end
		end
	until count == 10

	for i = 1, #newTable do
		db.query("INSERT INTO player_prey SET player_id = " ..player:getGuid().. ", name = " ..db.escapeString(newTable[i].Name).. ", mindex = " ..(i-1).. ", mcolumn = " ..column)
	end

	return newTable
end

function sendPreyData(player, msg2)
	local msg = NetworkMessage()
	local columnUnlocked = getUnlockedColumn(player)
	if (not columnUnlocked) then
		columnUnlocked = 0
	end

	local atualColumn = nil

	if (type(msg2) == 'number') then
		atualColumn = msg2
	else
		atualColumn = msg2:getByte()
	end

	Monsters = nil

	if (atualColumn == 10) then
		atualColumn = 2
	end

	local FreeRerollTime = player:getNextFreroll(atualColumn)

	msg:addByte(ServerPackets.PreyData) -- Server Prey Data
	msg:addByte(atualColumn) -- Atual Column

	local timeLeft = player:getStaminaBonus(atualColumn)
	local Bonus = loadBonus(player, atualColumn)

	if (columnUnlocked >= atualColumn) then
		if (Bonus and timeLeft > 0) then
			msg:addByte(0x02) -- Type action (STATE_SELECTION | LOAD PREY)
		else
			Monsters = getMonsterList(player, atualColumn)
			if (not Monsters) then
				Monsters = createMonstersColumn(player, atualColumn)
			end

			msg:addByte(0x03)
			msg:addByte(#Monsters)
		end
	else
		msg:addByte(0x00)
		msg:addByte(0x00)
	end

	if (columnUnlocked >= atualColumn) then
		if (Bonus and timeLeft > 0) then
			local mType = MonsterType(Bonus.Name)
			local mLook = mType:getOutfit()
			msg:addString(Bonus.Name)
			msg:addU16(mLook.lookType)
			msg:addByte(mLook.lookHead or 0x00) -- outfit
			msg:addByte(mLook.lookBody or 0x00) -- outfit
			msg:addByte(mLook.lookLegs or 0x00) -- outfit
			msg:addByte(mLook.lookFeet or 0x00) -- outfit
			msg:addByte(mLook.lookAddons or 0x00) -- outfit
			msg:addByte(Bonus.Type) -- Type
			msg:addU16(Bonus.Value) -- Value
			msg:addByte(Bonus.Grade) -- 1~10 Grade
			msg:addU16(timeLeft) -- Time Left Bonus
		else
			if (Bonus) then
				player:removeBonus(atualColumn)
			end

			for i, v in pairs(Monsters) do
				msg:addString(v.Name)
				local mLook = MonsterType(v.Name):getOutfit()
				msg:addU16(mLook.lookType or 21)
				msg:addByte(mLook.lookHead or 0x00) -- outfit
				msg:addByte(mLook.lookBody or 0x00) -- outfit
				msg:addByte(mLook.lookLegs or 0x00) -- outfit
				msg:addByte(mLook.lookFeet or 0x00) -- outfit
				msg:addByte(mLook.lookAddons or 0x00) -- outfit
			end
		end
	end

	msg:addU16(FreeRerollTime) -- Time to next free roll

	msg:addByte(0xEC)
	player:sendResource("prey", player:getBonusReroll()) -- Bonus Reroll
	player:sendResource("bank", player:getBankBalance())
	player:sendResource("inventory", player:getMoney())
	msg:addByte(ServerPackets.PreyRerollPrice)
	msg:addU32(player:getRerollPrice())
	msg:sendToPlayer(player)
end

local function getMonsterName(player, column, index)
	local resultId = db.storeQuery("SELECT name FROM player_prey WHERE mcolumn = " ..column.. " AND mindex = " ..index.." AND player_id = "..player:getGuid())
	if (not resultId) then
		return false
	end

	return result.getDataString(resultId, "name")
end

local function sendError(player, error)
	local msg = NetworkMessage()
	msg:addByte(ServerPackets.ShowDialog)
	msg:addByte(0x15) -- Type Dialog Error (0x14 => Normal Message)
	msg:addString(error)
	msg:sendToPlayer(player)
end

function CheckPrey(player, msg)
	local PreyColumn = msg:getByte() -- Column
	local PreyAction = msg:getByte() -- Type Action (2 == select)

	if (PreyAction == 0) then
		player:preyRerollList(PreyColumn)
	elseif (PreyAction == 1) then
		-- Bonus Reroll
		player:bonusReroll(PreyColumn)
	elseif (PreyAction == 2) then
		local PreyIndex = msg:getByte() -- monster index
		--print(PreyColumn.. " e " ..PreyAction.. " e " ..PreyIndex)
		if (getUnlockedColumn(player) < PreyColumn) then
			return sendError(player, "[ERROR] You don't have this column unlocked.")
		end

		local mName = getMonsterName(player, PreyColumn, PreyIndex)
		if (not mName) then
			return sendError(player, "[ERROR] Monster name don't exists in list.")
		end

		local mType = MonsterType(mName)
		if (not mType) then
			return sendError(player, "[ERROR] This monster don't exists in Server.")
		end

		SelectPrey(player, PreyColumn, mType)
	end
end

local function getBonusValue(min, max, steps)
	local retValue = 0
	local retGrade = 0
	local random = math.random(1, 10)
	while (retValue == 0) do
		if (retGrade >= 10) then
			retGrade = 0
		end

		for i = min, max, steps do
			retGrade = retGrade + 1
			if (retGrade == random) then
				retValue = i
				break
			end
		end
	end

	return retValue, retGrade
end

function getRandomBonus(player, column, name, bonusReroll)
	-- BONUS_DAMAGE_BOOST
	-- BONUS_DAMAGE_REDUCTION
	-- BONUS_XP_BONUS
	-- BONUS_IMPROVED_LOOT
	-- BONUS_NONE
	local BonusValues = {
		[0] = {Min = 7, Max = 25, Steps = 2},
		[1] = {Min = 12, Max = 30, Steps = 2},
		[2] = {Min = 13, Max = 40, Steps = 3},
		[3] = {Min = 13, Max = 40, Steps = 3}
	}

	local randomBonus = math.random(0, 3)
	
	if (player:isActive(column) and
		not bonusReroll) then
		return false
	end

	local Bonus = BonusValues[randomBonus]
	local BonusValue, BonusGrade = getBonusValue(Bonus.Min, Bonus.Max, Bonus.Steps)
	local retTable = {Type = randomBonus, Left = 7200, Value = BonusValue, Grade = BonusGrade}

	player:setPreyStamina(column, 7200)
	player:setPreyType(column, retTable.Type)
	player:setPreyValue(column, retTable.Value)
	player:setPreyName(column, name)
	return retTable
end

function getBonusGrade(bonusType, bonusValue)
	local BonusValues = {
		[0] = {Min = 7, Max = 25, Steps = 2},
		[1] = {Min = 12, Max = 30, Steps = 2},
		[2] = {Min = 13, Max = 40, Steps = 3},
		[3] = {Min = 13, Max = 40, Steps = 3}
	}

	local Bonus = BonusValues[bonusType]
	local Grade = 0
	for i = Bonus.Min, Bonus.Max, Bonus.Steps do
		Grade = Grade + 1
		if (i >= bonusValue) then
			return Grade
		end
	end

	return nil
end

function loadBonus(player, column)
	local retTable = {
		Type = player:getPreyType(column), 
		Value = player:getPreyValue(column),
		Name = player:getPreyName(column)
	}

	retTable.Grade = getBonusGrade(retTable.Type, retTable.Value)

	if (retTable.Name == "") then
		return false
	end
	
	return retTable
end

function SelectPrey(player, PreyColumn, mType, bonusReroll)
	local msg = NetworkMessage()
	msg:addByte(ServerPackets.PreyData)
	msg:addByte(PreyColumn)

	local mLook = mType:getOutfit()
	if (not mLook) then
		return sendError(player, "[ERROR] Monster is invalid, please contact Administrator.")
	end

	local newBonus = getRandomBonus(player, PreyColumn, mType:getName(), bonusReroll)
	if (not newBonus and not bonusReroll) then
		return sendError(player, "[ERROR] You can't select a prey with bonus active.")
	end

	if (not bonusReroll) then
		player:removePreyMonster(mType:getName())
	end

	player:setPreyStamina(PreyColumn, 7200)

	msg:addByte(0x02)
	msg:addString(mType:getName())
	msg:addU16(mLook.lookType)
	msg:addByte(mLook.lookHead or 0x00) -- outfit
	msg:addByte(mLook.lookBody or 0x00) -- outfit
	msg:addByte(mLook.lookLegs or 0x00) -- outfit
	msg:addByte(mLook.lookFeet or 0x00) -- outfit
	msg:addByte(mLook.lookAddons or 0x00) -- outfit
	msg:addByte(newBonus.Type) -- Type
	msg:addU16(newBonus.Value) -- Value
	msg:addByte(newBonus.Grade) -- 1~10 Grade
	msg:addU16(newBonus.Left) -- Time Left Bonus
	msg:addU16(player:getNextFreroll(PreyColumn)) -- Next Free Reroll
	msg:sendToPlayer(player)
end

function onRecvbyte(player, msg, byte)
	if (byte == ClientPackets.RequestData) then
		monstersGenerated[player:getGuid()] = nil
		sendPreyData(player, msg)
	elseif (byte == ClientPackets.PreyAction) then
		CheckPrey(player, msg)
	end
end

function Player.getBonusReroll(self)
	local resultId = db.storeQuery("SELECT bonus_reroll FROM players WHERE id = " ..self:getGuid())
	if (resultId) then
		return result.getDataInt(resultId, "bonus_reroll")
	end

	return nil
end

function Player.getRerollPrice(self)
	return (self:getLevel()/2) * 500
end

function Player.preyRerollList(self, column)
	local rerollStorage = RerollStorages[column]
	if (self:getStorageValue(rerollStorage) > 0) then
		local priceReroll = self:getRerollPrice()
		if (not self:removeMoneyNpc(self:getRerollPrice())) then
			sendError(self, "[ERROR] You don't have " ..priceReroll.. " gold.")
		end
	end

	self:removeBonus(column)
	db.query("DELETE FROM player_prey WHERE player_id = " ..self:getGuid().. " AND mcolumn = " ..column)
	self:setStorageValue(rerollStorage, (os.time() / 60.000) + 1200)
	sendPreyData(self, column)
end

function Player.getNextFreroll(self, column)
	local FreeRerollTime = self:getStorageValue(RerollStorages[column])
	if (FreeRerollTime == -1) then
		self:setStorageValue(RerollStorages[column], 0)
		FreeRerollTime = 0
	end

	if (FreeRerollTime > 0) then
		if (FreeRerollTime < (os.time() / 60.000)) then
			FreeRerollTime = 0
			self:setStorageValue(RerollStorages[column], 0)
		else
			FreeRerollTime = FreeRerollTime - (os.time() / 60.000)
			self:setStorageValue(RerollStorages[column], (os.time() / 60.000) + FreeRerollTime)
		end
	end

	return FreeRerollTime
end

function Player.setBonusReroll(self, value)
	db.query("UPDATE players SET bonus_reroll = " ..value.. " WHERE id = " ..self:getGuid())
end

function Player.addBonusReroll(self, value)
	db.query("UPDATE players SET bonus_reroll = bonus_reroll + " ..value.. " WHERE id = " ..self:getGuid())
end

function Player.removeBonus(self, column)
	self:setPreyStamina(column, 0)
	self:setPreyType(column, 0)
	self:setPreyValue(column, 0)
	self:setPreyName(column, "")
end

function Player.getBonusMonster(self, column)
	return self:getPreyName(column)
end

function Player.getBonusInfo(self, column)
	return loadBonus(self, column)
end

function Player.isActive(self, column)
	local timeLeft, preyValue = self:getPreyStamina(column), self:getPreyValue(column)
	if (timeLeft > 0 and preyValue > 0) then
		return true
	end

	return false
end

function Player.sendResource(self, resourceType, value)
	local typeByte = 0
	if (resourceType == "bank") then
		typeByte = 0x00
	elseif (resourceType == "inventory") then
		typeByte = 0x01
	elseif (resourceType == "prey") then
		typeByte = 0x0A
	end

	local msg = NetworkMessage()
	msg:addByte(0xEE)
	msg:addByte(typeByte)
	msg:addU64(value)
	msg:sendToPlayer(self)
end

function Player.isActiveByName(self, column, name)
	if (self:getPreyStamina(column) > 0 and self:getPreyName(column) == name) then
		return true
	end

	return false
end

function Player.bonusReroll(self, column)
	local bonusReroll = self:getBonusReroll()
	if (bonusReroll == 0) then
		return sendError(self, "[ERROR] You don't have Bonus Reroll.")
	end

	if (not self:isActive(column)) then
		return sendError(self, "[ERROR] You don't have a active bonus.")
	end

	self:setBonusReroll(bonusReroll-1)
	local bonusMonster = self:getBonusMonster(column)
	self:removeBonus(column)

	local msg = NetworkMessage()
	msg:addByte(0xEE)
	msg:addByte(0x0A)
	msg:addU64(bonusReroll-1)
	msg:sendToPlayer(self)

	SelectPrey(self, column, MonsterType(bonusMonster), true)
end

function Player.getStaminaBonus(self, column)
	return self:getPreyStamina(column)
end

function Player.setStaminaBonus(self, column, value)
	self:setPreyStamina(column, value)
end

function Player.removePreyMonster(self, name)
	db.query("DELETE FROM player_prey WHERE player_id = " ..self:getGuid().. " AND name = " ..db.escapeString(name))
end

function Player.addPreySlot(self)
	db.query("UPDATE players SET prey_column = 2 WHERE id = " ..self:getGuid())
end

function Player.getPreySlots(self)
	local resultId = db.storeQuery("SELECT * FROM players WHERE name = " ..db.escapeString(self:getName()))
	if (resultId ~= false) then
		return result.getDataInt(resultId, "prey_column")
	end

	return nil
end

function Player.sendPreyTimeLeft(self, column, time)
	local msg = NetworkMessage()
	msg:addByte(ServerPackets.PreyTimeLeft)
	msg:addByte(column)
	msg:addU16(time)
	msg:sendToPlayer(self)
end