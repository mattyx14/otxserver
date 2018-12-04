--[[
	1~3 => Element Type
	4~6 => Total Time (segundos - 20 h)
	7~9 => Time Passed (seconds)
]]

--[[ Icon List --
	0 = NONE
	1 ~ 3   = Critical
	4 ~ 6   = Death
	7 ~ 9   = Earth
	10 ~ 12 = Energy
	13 ~ 15 = Fire
	16 ~ 18 = Holy
	19 ~ 21 = Ice
	22 ~ 24 = Fire (grey | not used)
	25 ~ 27 = Shielding (grey | not used)
	28 ~ 30 = Reduction Earth
	31 ~ 33 = Reduction Energy
	34 ~ 36 = Reduction Fire
	37 ~ 39 = Reduction Holy
	40 ~ 42 = Reduction Ice
	43 ~ 45 = Shielding (white | not used)
	46 ~ 48 = Life Leech
	49 ~ 51 = Mana Leech
	52 ~ 54 = Axe Fighting
	55 ~ 57 = Club Fighting
	58 ~ 60 = Distance Fighting
	61 ~ 63 = Fist Fighting
	64 ~ 66 = Magic Level
	67 ~ 69 = Shielding Fighting
	70 ~ 72 = Sword Fighting
	73 ~ 75 = Speed
]]

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
		IconID = 13,
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
		IconID = 7,
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
		IconID = 19,
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
		IconID = 10,
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
		IconID = 4,
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
		IconID = 46,
		Description = "converts % of damage to HP with a chance of 100%.",
		Levels = {"Basic", "Intricate", "Powerful"},
		LevelsPercent = {5, 10, 25},
		Weapons = {"axe", "club", "sword", "wand", "rod", "bow", "armor"},
		Items = {{10602, 25}, {10550, 15}, {10580, 5}}
	},
	{
		Name = "Void",
		Category = "Mana Leech",
		Type = "manapointsleech",
		IconID = 49,
		Description = "converts % of damage to MP with a chance of 100%.",
		Levels = {"Basic", "Intricate", "Powerful"},
		LevelsPercent = {3, 5, 8},
		Weapons = {"axe", "club", "sword", "wand", "rod", "bow", "helmet"},
		Items = {{12448, 25}, {22534, 25}, {25386, 5}}
	},
	{
		Name = "Strike",
		Category = "Critical Hit",
		Type = "criticaldamage",
		IconID = 1,
		Description = "raises crit hit damage by % and crit hit chance by 10%.",
		Levels = {"Basic", "Intricate", "Powerful"},
		LevelsPercent = {15, 25, 50},
		Weapons = {"axe", "club", "sword", "bow"},
		Items = {{12400, 20}, {11228, 25}, {25384, 5}}
	},
	{
		Name = "Lich Shroud",
		Category = "Death Damage",
		Type = "absorbPercentDeath",
		IconID = 25,
		Description = "reduces death damage by %.",
		Levels = {"Basic", "Intricate", "Powerful"},
		LevelsPercent = {3, 8, 15},
		Weapons = {"armor", "shield"},
		Items = {{12422, 25}, {24663, 20}, {10577, 5}}
	},
	{
		Name = "Snake Skin",
		Category = "Eart Damage",
		Type = "absorbPercentEarth",
		IconID = 28,
		Description = "reduces earth damage by %.",
		Levels = {"Basic", "Intricate", "Powerful"},
		LevelsPercent = {3, 8, 15},
		Weapons = {"armor", "shield"},
		Items = {{20103, 25}, {10611, 20}, {12658, 10}}
	},
	{
		Name = "Hide Dragon",
		Category = "Fire Damage",
		Type = "absorbPercentFire",
		IconID = 34,
		Description = "reduces fire damage by %.",
		Levels = {"Basic", "Intricate", "Powerful"},
		LevelsPercent = {3, 8, 15},
		Weapons = {"armor", "shield"},
		Items = {{5877, 25}, {18425, 10}, {12614, 5}}
	},
	{
		Name = "Quara Scale",
		Category = "Ice Damage",
		Type = "absorbPercentIce",
		IconID = 40,
		Description = "reduces ice damage by %.",
		Levels = {"Basic", "Intricate", "Powerful"},
		LevelsPercent = {3, 8, 15},
		Weapons = {"armor", "shield"},
		Items = {{11212, 25}, {11224, 15}, {15425, 10}}
	},
	{
		Name = "Cloud Fabric",
		Category = "Energy Damage",
		Type = "absorbPercentEnergy",
		IconID = 31,
		Description = "reduces energy damage by %.",
		Levels = {"Basic", "Intricate", "Powerful"},
		LevelsPercent = {3, 8, 15},
		Weapons = {"armor", "shield"},
		Items = {{10561, 25}, {15482, 15}, {10582, 10}}
	},
	{
		Name = "Demon Presence",
		Category = "Holy Damage",
		Type = "absorbPercentHoly",
		IconID = 37,
		Description = "reduces holy damage by %.",
		Levels = {"Basic", "Intricate", "Powerful"},
		LevelsPercent = {3, 8, 15},
		Weapons = {"armor", "shield"},
		Items = {{10556, 25}, {10555, 25}, {11221, 20}}
	},
	{
		Name = "Swiftness",
		Category = "Increase Speed",
		Type = "speed",
		IconID = 73,
		Description = "raises walking speed by %.",
		Levels = {"Basic", "Intricate", "Powerful"},
		LevelsPercent = {10, 15, 20},
		Weapons = {"boots"},
		Items = {{19738, 15}, {11219, 25}, {15484, 20}}
	},
	{
		Name = "Chop",
		Category = "Increase Axe Fighting",
		Type = "skillAxe",
		IconID = 52,
		Description = "raises axe fighting skill by %.",
		Levels = {"Basic", "Intricate", "Powerful"},
		LevelsPercent = {1, 2, 4},
		Weapons = {"axe", "helmet"},
		Items = {{11113, 20}, {12403, 25}, {23571, 20}}
	},
	{
		Name = "Slash",
		Category = "Increase Sword Fighting",
		Type = "skillSword",
		IconID = 70,
		Description = "raises sword fighting skill by %.",
		Levels = {"Basic", "Intricate", "Powerful"},
		LevelsPercent = {1, 2, 4},
		Weapons = {"sword", "helmet"},
		Items = {{10608, 25}, {23573, 25}, {10571, 5}}
	},
	{
		Name = "Bash",
		Category = "Increase Club Fighting",
		Type = "skillClub",
		IconID = 52,
		Description = "raises club fighting skill by %.",
		Levels = {"Basic", "Intricate", "Powerful"},
		LevelsPercent = {1, 2, 4},
		Weapons = {"club", "helmet"},
		Items = {{10574, 20}, {24845, 15}, {11322, 10}}
	},
	{
		Name = "Precision",
		Category = "Increase Distance Fighting",
		Type = "skillDist",
		IconID = 58,
		Description = "raises distance fighting skill by %.",
		Levels = {"Basic", "Intricate", "Powerful"},
		LevelsPercent = {1, 2, 4},
		Weapons = {"bow", "helmet"},
		Items = {{12420, 25}, {21311, 20}, {11215, 10}}
	},
	{
		Name = "Blockade",
		Category = "Increase Shielding",
		Type = "skillShield",
		IconID = 67,
		Description = "raises shielding skill by %.",
		Levels = {"Basic", "Intricate", "Powerful"},
		LevelsPercent = {1, 2, 4},
		Weapons = {"shield", "helmet"},
		Items = {{10558, 20}, {12659, 25}, {22533, 25}}
	},
	{
		Name = "Epiphany",
		Category = "Increase Magic Level",
		Type = "magiclevelpoints",
		IconID = 64,
		Description = "raises magic level by %.",
		Levels = {"Basic", "Intricate", "Powerful"},
		LevelsPercent = {1, 2, 4},
		Weapons = {"wand", "rod", "helmetmage"},
		Items = {{10552, 25}, {12408, 15}, {11226, 15}}
	},
	{
		Name = "Featherweight",
		Category = "Increase Capacity",
		Type = "capacity",
		IconID = 64,
		Description = "raises capacity by %.",
		Levels = {"Basic", "Intricate", "Powerful"},
		LevelsPercent = {3, 8, 15},
		Weapons = {"backpack"},
		Items = {{28998, 25}, {29006, 10}, {22539, 5}}
	}
}

local Weapons = {
	["armor"] = {21692, 2500, 2656, 2464, 2487, 2494, 15407, 2492, 2503, 12607, 2505, 2466, 23538, 10296, 2476, 3968, 2472, 7463, 8888, 23537, 2486, 15406, 8891, 18404},
	["shield"] = {2528, 2537, 2518, 15491, 2534, 2535, 2536, 2542, 2539, 2519, 2520, 25382, 25414, 15411, 2516, 2514, 2522, 2533, 2531, 21707, 10289, 6433, 6391, 7460, 2524, 15413, 21697, 3974, 12644, 10297, 10294, 2509, 10364, 15453, 25411, 2217, 2175, 8900, 8901, 29003, 22422, 22423, 22424},
	["boots"] = {9931, 3982, 15410, 2646, 24637, 5462, 18406, 2645, 25412, 21708},
	["helmet"] = {2499, 2139, 3972, 2458, 2491, 2497, 2493, 2502, 12645, 7458, 2471, 10298, 10299, 20132, 2662, 10291, 2498, 24848, 5741, 25410, 2475, 11302},
	["helmetmage"] = {10016, 2323, 12630, 11368, 8820, 10570, 9778},
	["bow"] = {8855, 7438, 15643, 21696, 10295, 18454, 25522, 8857, 8854, 22416, 22417, 22418, 8850, 8851, 8852, 8853, 2455, 8849, 25523, 16111, 21690, 22419, 22420, 22421, 25885, 25886, 25943, 25947, 25983, 25987},
	["backpack"] = {1988, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 3940, 5801, 3960, 5926, 5949, 7342, 9774, 10518, 10519, 10521, 10522, 11119, 11241, 11243, 11244, 11263, 15645, 15646, 16007, 18393, 18394, 21475, 22696, 23666, 23816, 24740, 26181, 27061, 27063},
	["wand"] = {8920, 8921, 8922, 2191, 29006, 29004, 25887, 25951, 25991},
	["rod"] = {8910, 8911, 24839, 29006, 29004, 25888, 25955, 25995},
	["axe"] = {2429, 2454, 2426, 2427, 2414, 2415, 2443, 7380, 7389, 11323, 7455, 2447, 7412, 8926, 7419, 7453, 2430, 2435, 3962, 15451, 7434, 7435, 6553, 15492, 7456, 8925, 18451, 2431, 8924, 25931, 11305, 25383, 22404, 22405, 22407, 22408, 22409, 25881, 25882, 25927, 25931, 25967, 25971},
	["club"] = {2391, 2423, 7415, 2445, 7424, 2452, 2444, 7426, 7414, 7452, 7429, 7421, 15414, 7410, 15647, 20093, 7430, 7431, 23543, 2453, 8929, 12648, 7423, 2436, 2424, 7451, 7437, 2421, 8928, 18452, 25418, 22410, 22411, 22412, 22413, 22414, 22415, 25883, 25884, 25935, 25939},
	["sword"] = {7407, 2393, 2383, 7382, 7383, 7384, 7385, 7403, 2413, 7405, 7406, 7391, 7392, 11309, 12613, 7417, 2376, 2400, 7404, 7402, 12649, 2438, 8930, 2451, 11395, 2407, 7416, 11307, 7418, 6528, 7408, 18465, 8931, 22398, 22399, 22400, 22401, 22402, 22403, 25879, 25880, 25919, 25923, 25959, 25963}
}

local ImbuingInfo = {
	[1] = {Price = 5000, Protection = 10000, Percent = 90},
	[2] = {Price = 25000, Protection = 30000, Percent = 70},
	[3] = {Price = 100000, Protection = 50000, Percent = 50}
}

local imbuingShrineIds = {
	27728, 27729, 27850, 27851
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
				for index, id in pairs(imbuingShrineIds) do
					if tile:getItemById(id) then
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
			if (enchant and type(enchant) ~= 'number' and enchant:find(level) and enchant:find(Imbuements[i].Name)) then
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

function Player.applyImbuement(self, msg)
	if (not haveImbuingShrine(self)) then
		sendImbuementError(self, "An error ocurred, please reopen imbuement window.", ErrorMessages.MESSAGEDIALOG_IMBUEMENT_ERROR)
		return false
	end

	local item = lastItemImbuing[self:getGuid()]
	if not item then
		sendImbuementError(self, "Cannot find item, please contact an Administrator.", ErrorMessages.MESSAGEDIALOG_IMBUEMENT_ERROR)
		return false
	end

	local slot, choiceId, useProtection = msg:getByte(), msg:getU32(), msg:getByte()
	local myImbuements = getImbuementEquip(getEquipById(item:getId()))
	local imbuingLevel = 0
	local imbuementNow, index = nil, 0

	for i = 1, item:getType():getImbuingSlots() do
		existsImbuement, enchantLevel = getActiveImbuement(item, i)
		if existsImbuement then
			myImbuements = mergeImbuementList(myImbuements, existsImbuement)
		end
	end

	for k = 1, #myImbuements do
		for i = 1, 3 do
			index = index + 1
			if not imbuementNow and index == choiceId then
				imbuementNow = myImbuements[k]
			end
		end
	end

	for i = 3, index, 3 do
		if choiceId >= i-2 and choiceId <= i then
			imbuingLevel = math.abs((i - 2) - choiceId) + 1
			break
		end
	end

	if (not imbuementNow) then
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
	if self:getAccountType() < 5 then
		for j = 1, imbuingLevel do
			local itemID, itemCount = imbuementNow.Items[j][1], imbuementNow.Items[j][2]
			if (self:getItemCount(itemID) < itemCount) then
				sendImbuementError(self, "You don't have all necessary items.", ErrorMessages.MESSAGEDIALOG_IMBUEMENT_ROLL_FAILED)
				return false
			end

			self:removeItem(itemID, itemCount)
		end
	end

	if (item:isActiveImbuement(slot+3)) then
		sendImbuementError(self, "An error ocurred, please reopen imbuement window.", ErrorMessages.MESSAGEDIALOG_IMBUEMENT_ERROR)
		return false
	end

	local applyChance = math.random(100)
	if (ImbuingInfo[imbuingLevel].Percent < applyChance and useProtection == 0) then
		sendImbuementError(self, "Item failed to apply imbuement.", ErrorMessages.MESSAGEDIALOG_IMBUEMENT_ROLL_FAILED)
		return false
	end

	item:setSpecialAttribute(slot, imbuementNow.Levels[imbuingLevel].. " " ..imbuementNow.Name, slot+3, 72000, slot+6, 0)
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
	if (not item or not item:isItem() or not item:isImbuementEquip()) then
		self:sendTextMessage(MESSAGE_EVENT_ADVANCE, "This item is not imbuable.")
		return false
	end

	for slot = 1, 10 do
		if (self:getSlotItem(slot) and self:getSlotItem(slot):getUniqueId() == item:getUniqueId()) then
			self:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You cannot imbue an equipped item.")
			return false
		end
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
			msg:addU16(existsImbuement.IconID+(enchantLevel-1)) -- Icon ID (wtf?)
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
			if self:getAccountType() < 5 then
				msg:addU16(self:getItemCount(myImbuements[k].Items[j][1]))
			else
				msg:addU16(999)
			end
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
	if (enchant == 0) then
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
			if (name:find(v.Name) and name:find(v.Levels[j]) and name ~= 0) then
				return v.LevelsPercent[j]
			end
		end
	end

	return nil
end
