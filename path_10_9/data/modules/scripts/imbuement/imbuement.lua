--[[
	1~3 => Element Type
	4~6 => Total Time (segundos - 20 h)
	7~9 => Time Passed (seconds)
]]--
ImbuingSystem = {
	Developer = "Charles (Cjaker)",
	Version = "1.0",
	LastUpdate = "24/05/2017 - 03:50 (AM)"
}

local Imbuements = {
	{
		Name = "Scorch",
		Category = "Elemental Damage (Fire)",
		Type = "firedamage",
		Description = "Converts % of the physical damage to fire damage.",
		Levels = {"Basic", "Intricate", "Powerful"},
		LevelsPercent = {10, 25, 50},
		Weapons = {"axe", "club", "sword"},
		Items = {{10553, 25}, {5920, 5}, {5954, 5}}
	},
	{
		Name = "Venom",
		Category = "Elemental Damage (Earth)",
		Type = "earthdamage",
		Description = "Converts % of the physical damage to earth damage.",
		Levels = {"Basic", "Intricate", "Powerful"},
		LevelsPercent = {10, 25, 50},
		Weapons = {"axe", "club", "sword"},
		Items = {{10603, 25}, {10557, 5}, {23565, 5}}
	},
	{
		Name = "Frost",
		Category = "Elemental Damage (Ice)",
		Type = "icedamage",
		Description = "Converts % of the physical damage to ice damage.",
		Levels = {"Basic", "Intricate", "Powerful"},
		LevelsPercent = {10, 25, 50},
		Weapons = {"axe", "club", "sword"},
		Items = {{10567, 25}, {10578, 20}, {24170, 1}}
	},
	{
		Name = "Electrify",
		Category = "Elemental Damage (Energy)",
		Type = "energydamage",
		Description = "Converts % of the physical damage to energy damage.",
		Levels = {"Basic", "Intricate", "Powerful"},
		LevelsPercent = {10, 25, 50},
		Weapons = {"axe", "club", "sword"},
		Items = {{21310, 25}, {24631, 5}, {26164, 1}}
	},
	{
		Name = "Reap",
		Category = "Elemental Damage (Death)",
		Type = "deathdamage",
		Description = "Converts % of the physical damage to death damage.",
		Levels = {"Basic", "Intricate", "Powerful"},
		LevelsPercent = {10, 25, 50},
		Weapons = {"axe", "club", "sword"},
		Items = {{12440, 25}, {10564, 20}, {11337, 5}}
	},
	{
		Name = "Vampirism",
		Category = "Life Leech",
		Type = "hitpointsleech",
		Description = "converts % of damage to HP with a chance of 100%",
		Levels = {"Basic", "Intricate", "Powerful"},
		LevelsPercent = {5, 10, 25},
		Weapons = {"axe", "club", "sword", "wand", "rod", "bow", "armor"},
		Items = {{10602, 25}, {10550, 15}, {10580, 5}}
	}
}

local Weapons = {
	["axe"] = {3962, 7412, 18451, 8926, 2414, 11305, 7419, 2435, 7453, 2415, 2427, 8924, 15492, 7435, 7455, 7456, 2443, 25383, 7434, 6553, 8925, 2431, 2447, 15451, 11323},
	["club"] = {7414, 7426, 2453, 7429, 15647, 7431, 7430, 23543, 2444, 2452, 20093, 7424, 25418, 18452, 8928, 7421, 15414, 7410, 7437, 7451, 2424, 2436, 7423, 12648, 7452, 8929, 2421},
	["sword"] = {7404, 7403, 12649, 7416, 2407, 2413, 7385, 7382, 2451, 8930, 2438, 2393, 7407, 7405, 2400, 7418, 7417, 18465, 2376, 7391, 6528, 8931, 12613, 11309, 7408, 11307}
}

local ImbuingInfo = {
	[1] = {Price = 5000, Protection = 10000, Percent = 90},
	[2] = {Price = 25000, Protection = 30000, Percent = 70},
	[3] = {Price = 100000, Protection = 50000, Percent = 50}
}

local imbuingShrineIds = {
	27716, 27717, 27728, 27729, 27850, 27851
}

local ImbuementElements = {
	"firedamage", "earthdamage", "energydamage", "deathdamage", "icedamage"
}

local ErrorMessages = {
	MESSAGEDIALOG_IMBUEMENT_ERROR = 1,
	MESSAGEDIALOG_IMBUEMENT_ROLL_FAILED = 2,
	MESSAGEDIALOG_IMBUING_STATION_NOT_FOUND = 3,
	MESSAGEDIALOG_CLEARING_CHARM_SUCCESS = 10,
	MESSAGEDIALOG_CLEARING_CHARM_ERROR = 11
}

local ClientPackets = {
	ApplyImbuement = 0xD5,
	ClearImbuement = 0xD6
}

function onRecvbyte(player, msg, byte)
	if (byte == ClientPackets.ApplyImbuement) then
		-- Apply Imbuement
		player:applyImbuement(msg)
	elseif (byte == ClientPackets.ClearImbuement) then
		-- Clear Imbuement
		player:clearImbuement(msg)
	end
end

local function tableContains(table, value)
	for i = 1, #table do
		if (table[i] == value) then
			return true
		end
	end

	return false
end

local function haveImbuingShrine(player) 
	for x = -1, 1 do
		for y = -1, 1 do
			local posX, posY, posZ = player:getPosition().x+x, player:getPosition().y+y, player:getPosition().z
			local tile = Tile(posX, posY, posZ)
			if (tile) then
				local topItem = tile:getTopTopItem()
				if (topItem) then
					if (tableContains(imbuingShrineIds, topItem:getId())) then
						return true
					end
				end
			end
		end
	end

	return false
end

local function getEquipById(id)
	for i, v in pairs(Weapons) do
		if (tableContains(v, id)) then
			return i
		end
	end

	return nil
end

local function getImbuementEquip(equip)
	local tableReturn = {}
	for i = 1, #Imbuements do
		if (tableContains(Imbuements[i].Weapons, equip)) then
			tableReturn[#tableReturn+1] = Imbuements[i]
		end
	end

	return tableReturn
end

local function getActiveImbuement(item, slot)
	for i = 1, #Imbuements do
		for j = 1, 3 do
			local level = Imbuements[i].Levels[j]
			local enchant = item:getSpecialAttribute(slot)
			if (enchant:find(level) and enchant:find(Imbuements[i].Name)) then
				return Imbuements[i], j
			end
		end
	end

	return nil
end

local function getImbuementByIndex(index, id)
	local equip = getEquipById(id)
	local myImbuements = getImbuementEquip(equip)
	local tmpIndex = 0
	for i = 1, #myImbuements do
		for k = 1, 3 do
			tmpIndex = tmpIndex + 1
			if (index == tmpIndex) then
				return myImbuements[i], k
			end
		end
	end

	return nil
end

local function sendImbuementError(self, message, errorType)
	local msg = NetworkMessage()
	msg:addByte(0xED)
	msg:addByte(errorType or 0x01)
	msg:addString(message)
	msg:sendToPlayer(self)
end

local function mergeImbuementList(table1, table2)
	local newTable = {}
	for i, v in pairs(table1) do
		if (v.Name ~= table2.Name and not (tableContains(ImbuementElements, v.Type) and tableContains(ImbuementElements, table2.Type))) then
			newTable[#newTable+1] = v
		end
	end

	return newTable
end

local function getNewList(item)
	local equip = getEquipById(item:getId())
	local myImbuements = getImbuementEquip(equip)
	local imbuingSlots = item:getType():getImbuingSlots()

	for i = 1, imbuingSlots do
		if (item:isActiveImbuement(i+3)) then
			local existsImbuement, enchantLevel = getActiveImbuement(item, i)
			myImbuements = mergeImbuementList(myImbuements, existsImbuement)
		end
	end

	return myImbuements
end

function Player.applyImbuement(self, msg)
	if (not haveImbuingShrine(self)) then
		sendImbuementError(self, "An error ocurred, please reopen imbuement window.", ErrorMessages.MESSAGEDIALOG_IMBUEMENT_ERROR)
		return false
	end

	local item = lastItemImbuing[self:getGuid()]
	if (item == nil) then
		sendImbuementError(self, "Cannot find item, please contact an Administrator.", ErrorMessages.MESSAGEDIALOG_IMBUEMENT_ERROR)
		return false
	end

	local slot, choiceId, useProtection = msg:getByte(), msg:getU32(), msg:getByte()
	local myImbuement, imbuingLevel = getImbuementByIndex(choiceId, item:getId())
	local imbuementsNow = getNewList(item)
	local index = 0
	for i = 1, #imbuementsNow do
		for j = 1, item:getType():getImbuingSlots() do
			index = index + 1
			if (choiceId == index) then
				myImbuement, imbuingLevel = imbuementsNow[i], j
				break
			end
		end
	end

	if (not myImbuement) then
		sendImbuementError(self, "Cannot find imbuement data, please contact an Administrator.", ErrorMessages.MESSAGEDIALOG_IMBUEMENT_ERROR)
		return false
	end

	local imbuingPrice = ImbuingInfo[imbuingLevel].Price
	if (useProtection == 1) then
		imbuingPrice = imbuingPrice + ImbuingInfo[imbuingLevel].Protection
	end

	if (not self:removeMoneyNpc(imbuingPrice)) then
		sendImbuementError(self, "You don't have enough money " ..imbuingPrice.. " gps.", ErrorMessages.MESSAGEDIALOG_IMBUEMENT_ROLL_FAILED)
		return false
	end

	slot = slot + 1

	for j = 1, imbuingLevel do
		local itemID, itemCount = myImbuement.Items[j][1], myImbuement.Items[j][2]
		if (self:getItemCount(itemID) < itemCount) then
			sendImbuementError(self, "You don't have all necessary items.", ErrorMessages.MESSAGEDIALOG_IMBUEMENT_ROLL_FAILED)
			return false
		end

		self:removeItem(itemID, itemCount)
	end

	if (item:isActiveImbuement(slot+3)) then
		sendImbuementError(self, "An error ocurred, please reopen imbuement window.", ErrorMessages.MESSAGEDIALOG_IMBUEMENT_ERROR)
		return false
	end
	
	local applyChance = math.random(100)
	if (ImbuingInfo[imbuingLevel].Percent < applyChance) then
		sendImbuementError(self, "Item failed to apply imbuement.", ErrorMessages.MESSAGEDIALOG_IMBUEMENT_ROLL_FAILED)
		return false
	end

	item:setSpecialAttribute(slot, myImbuement.Levels[imbuingLevel]..myImbuement.Name, slot+3, 72000, slot+6, 0)
	self:openImbuementWindow(item)
end

function Player.clearImbuement(self, msg)
	if (not haveImbuingShrine(self)) then
		sendImbuementError(self, "Sorry, not possible.", ErrorMessages.MESSAGEDIALOG_CLEARING_CHARM_ERROR)
		return false
	end

	local item = lastItemImbuing[self:getGuid()]
	if (item == nil) then
		sendImbuementError(self, "Cannot find item, please send this message to a Administrator.", ErrorMessages.MESSAGEDIALOG_CLEARING_CHARM_ERROR)
		return false
	end

	local weaponSlot = msg:getByte()
	if (not weaponSlot) then
		sendImbuementError(self, "Sorry, not possible.", ErrorMessages.MESSAGEDIALOG_CLEARING_CHARM_ERROR)
		return false
	end

	weaponSlot = weaponSlot + 1

	if (not item:isActiveImbuement(weaponSlot + 3)) then
		sendImbuementError(self, "Sorry, not possible.", ErrorMessages.MESSAGEDIALOG_CLEARING_CHARM_ERROR)
		return false
	end

	if (not self:removeMoneyNpc(15000)) then
		sendImbuementError(self, "You don't have enough money 15000 gps.", ErrorMessages.MESSAGEDIALOG_CLEARING_CHARM_ERROR)
		return false
	end
	
	item:setSpecialAttribute(weaponSlot, 0, weaponSlot+3, 0, weaponSlot+6, 0)
	self:openImbuementWindow(item)
	sendImbuementError(self, "Item clean success!", ErrorMessages.MESSAGEDIALOG_CLEARING_CHARM_SUCCESS)
end

function Player.closeImbuementWindow(self)
	local msg = NetworkMessage()
	msg:addByte(0xEC)
	msg:sendToPlayer(self)
end

function Player.openImbuementWindow(self, item)
	if (not item:isImbuementEquip()) then
		self:sendTextMessage(MESSAGE_EVENT_ADVANCE, "This item is not imbuable.")
		return false
	end

	if (self:getSlotItem(CONST_SLOT_LEFT) and self:getSlotItem(CONST_SLOT_LEFT):getUniqueId() == item:getUniqueId()) then
		self:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You cannot imbue an equipped item.")
		return false
	end

	local msg = NetworkMessage()
	local itemID = item:getId()
	local equip = getEquipById(item:getId())
	local myImbuements = getImbuementEquip(equip)
	local imbuingSlots = item:getType():getImbuingSlots()
	lastItemImbuing[self:getGuid()] = item

	msg:addByte(0xEB)
	msg:addItemId(itemID) -- Item to put slots imbuement
	msg:addByte(imbuingSlots) -- Loop Exists Imbuement and slot (soon)

	for i = 1, imbuingSlots do
		if (item:isActiveImbuement(i+3)) then
			local existsImbuement, enchantLevel = getActiveImbuement(item, i)
			myImbuements = mergeImbuementList(myImbuements, existsImbuement)
			msg:addByte(0x01) -- No have imbuement (byte 1 need more packets)
			msg:addU32(900) -- Start Read Imbuement Data
			msg:addString(existsImbuement.Levels[enchantLevel].. " " ..existsImbuement.Name) -- Name Element
			local newDescription = existsImbuement.Description:gsub(" %%", " " ..existsImbuement.LevelsPercent[enchantLevel].."%%")
			msg:addString(newDescription.. "\nLasts for 20h 0min while fighting.") -- Description
			msg:addString(existsImbuement.Category) -- Type Imbuement
			msg:addU16(4) -- Icon ID (wtf?)
			msg:addU32(72000) -- duration in seconds (20hrs)
			msg:addByte(0x01) -- premium true
			msg:addByte(enchantLevel) -- Loop Length astral sources
			for j = 1, enchantLevel do
				local itemID, itemName = existsImbuement.Items[j][1], ItemType(existsImbuement.Items[j][1]):getName()
				msg:addItemId(itemID or 2160) -- Astral ID
				msg:addString(itemName or "") -- Astral Name
				msg:addU16(existsImbuement.Items[j][2]) -- Astral Necessary count
			end
			msg:addU32(ImbuingInfo[enchantLevel].Price)
			msg:addByte(ImbuingInfo[enchantLevel].Percent)
			msg:addU32(ImbuingInfo[enchantLevel].Protection) -- End Read Imbuement Data
			msg:addU32(item:getTimeImbuement(i+3)) -- Remaining Seconds
			msg:addU32(15000) -- Clear Cost Gold
		else
			msg:addByte(0x00)
		end
	end

	msg:addU16(#myImbuements*3) -- Loop Read Imbuement data
	local index = 0
	for k = 1, #myImbuements do
		for i = 1, 3 do
			index = index + 1
			msg:addU32(index) -- Start Read Imbuement Data
			msg:addString(myImbuements[k].Levels[i].. " " ..myImbuements[k].Name) -- Name Element
			local newDescription = myImbuements[k].Description:gsub(" %%", " " ..myImbuements[k].LevelsPercent[i].."%%")
			msg:addString(newDescription.. "\nLasts for 20h 0min while fighting.") -- Description
			msg:addString(myImbuements[k].Category) -- Type Imbuement
			msg:addU16(1) -- Icon ID (wtf?)
			msg:addU32(72000) -- duration in seconds (20hrs)
			if (i > 1) then
				msg:addByte(0x01) -- premium true
			else
				msg:addByte(0x00) -- premium false
			end
			msg:addByte(i) -- Loop Length astral sources
			for j = 1, i do
				local itemID, itemName = myImbuements[k].Items[j][1], ItemType(myImbuements[k].Items[j][1]):getName()
				msg:addItemId(itemID or 2160) -- Astral ID
				msg:addString(itemName or "") -- Astral Name
				msg:addU16(myImbuements[k].Items[j][2]) -- Astral Necessary count
			end
			msg:addU32(ImbuingInfo[i].Price)
			msg:addByte(ImbuingInfo[i].Percent)
			msg:addU32(ImbuingInfo[i].Protection) -- End Read Imbuement Data
		end
	end

	msg:addU32(#myImbuements*3)
	for k = 1, #myImbuements do
		for j = 1, 3 do
			msg:addItemId(myImbuements[k].Items[j][1])
			msg:addU16(self:getItemCount(myImbuements[k].Items[j][1]))
		end
	end
	self:sendResource("bank", self:getBankBalance())
	self:sendResource("inventory", self:getMoney())
	msg:sendToPlayer(self)
end

function Item:isActiveImbuement(index)
	local time = self:getSpecialAttribute(index)
	if (time and time > 0) then
		return true
	end

	return false
end

function Item:getTimeImbuement(index)
	local time = self:getSpecialAttribute(index)
	if (time and time > 0) then
		return time
	end

	return false
end

function Item:isImbuementEquip()
	if (not self) then
		return false
	end

	if (self:getType() and self:getType():getImbuingSlots() > 0) then
		return true
	end

	return false
end

function Item:getImbuementType(slot)
	local enchant = self:getSpecialAttribute(slot)
	if (not enchant) then
		return false
	end

	for i, v in pairs(Imbuements) do
		for j = 1, 3 do
			if (enchant:find(v.Name) and enchant:find(v.Levels[j])) then
				return v.Type
			end
		end
	end

	return nil
end

function Item:getImbuementPercent(name)
	for i, v in pairs(Imbuements) do
		for j = 1, 3 do
			if (name:find(v.Name) and name:find(v.Levels[j])) then
				return v.LevelsPercent[j]
			end
		end
	end

	return nil
end