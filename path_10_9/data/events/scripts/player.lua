-- No move items with actionID 8000
-- Players cannot throw items on teleports if set to true
local blockTeleportTrashing = true

-- Internal Use
ITEM_STORE_INBOX = 26052
GOLD_POUNCH = 26377

function Player:onBrowseField(position)
	return true
end

function Player:onLook(thing, position, distance)
	local description = 'You see '
	if thing:isItem() then
		local itemType = thing:getType()
		if (itemType and itemType:getImbuingSlots() > 0) then
			local imbuingSlots = "Imbuements: ("
			for i = 1, itemType:getImbuingSlots() do
				local specialAttr = thing:getSpecialAttribute(i)
				local time = 0
				if (thing:getSpecialAttribute(i+3)) then
					time = getTime(thing:getSpecialAttribute(i+3))
				end
				
				if (specialAttr) then
					if (i ~= itemType:getImbuingSlots()) then
						imbuingSlots = imbuingSlots.. "" ..specialAttr.." " ..time..", "
					else
						imbuingSlots = imbuingSlots.. "" ..specialAttr.." " ..time..")."
					end
				else
					if (i ~= itemType:getImbuingSlots()) then
						imbuingSlots = imbuingSlots.. "Empty Slot, "
					else
						imbuingSlots = imbuingSlots.. "Empty Slot)."
					end
				end
			end
			description = string.gsub(description, "It weighs", imbuingSlots.. "\nIt weighs")
		end
	else
		description = description .. thing:getDescription(distance)
	end

	if self:getGroup():getAccess() then
		if thing:isItem() then
			description = string.format('%s\nItem ID: %d', description, thing.itemid)

			local actionId = thing.actionid
			if actionId ~= 0 then
				description = string.format('%s, Action ID: %d', description, actionId)
			end

			local uniqueId = thing:getAttribute(ITEM_ATTRIBUTE_UNIQUEID)
			if uniqueId > 0 and uniqueId < 65536 then
				description = string.format('%s, Unique ID: %d', description, uniqueId)
			end

			description = description .. '.'
			local itemType = thing:getType()

			local transformEquipId = itemType:getTransformEquipId()
			local transformDeEquipId = itemType:getTransformDeEquipId()
			if transformEquipId ~= 0 then
				description = string.format('%s\nTransforms to: %d (onEquip)', description, transformEquipId)
			elseif transformDeEquipId ~= 0 then
				description = string.format('%s\nTransforms to: %d (onDeEquip)', description, transformDeEquipId)
			end

			local decayId = itemType:getDecayId()
			if decayId ~= -1 then
				description = string.format('%s\nDecays to: %d', description, decayId)
			end
		elseif thing:isCreature() then
			local str = '%s\nHealth: %d / %d'
			if thing:getMaxMana() > 0 then
				str = string.format('%s, Mana: %d / %d', str, thing:getMana(), thing:getMaxMana())
			end
			description = string.format(str, description, thing:getHealth(), thing:getMaxHealth()) .. '.'
		end

		local position = thing:getPosition()
		description = string.format(
			'%s\nPosition: %d, %d, %d',
			description, position.x, position.y, position.z
		)

		if thing:isCreature() and thing:isPlayer() then
			description = string.format('%s\nIP: %s.', description, Game.convertIpToString(thing:getIp()))
		end
	end
	self:sendTextMessage(MESSAGE_INFO_DESCR, description)
end

function Player:onLookInTrade(partner, item, distance)
	self:sendTextMessage(MESSAGE_INFO_DESCR, "You see " .. item:getDescription(distance))
end

function Player:onLookInShop(itemType, count)
	return true
end

function Player:onMoveItem(item, count, fromPosition, toPosition, fromCylinder, toCylinder)
	-- Store Inbox
	local containerIdFrom = fromPosition.y - 64
	local containerFrom = self:getContainerById(containerIdFrom)
	if (containerFrom) then
		if (containerFrom:getId() == ITEM_STORE_INBOX and toPosition.y >= 1 and toPosition.y <= 11 and toPosition.y ~= 3) then
			self:sendCancelMessage(RETURNVALUE_CONTAINERNOTENOUGHROOM)
			return false
		end
	end

	local containerTo = self:getContainerById(toPosition.y-64)
	if (containerTo) then
		if (containerTo:getId() == ITEM_STORE_INBOX) then
			self:sendCancelMessage(RETURNVALUE_CONTAINERNOTENOUGHROOM)
			return false
		end
		-- Gold Pounch
		if (containerTo:getId() == GOLD_POUNCH) then
			if (not (item:getId() == ITEM_CRYSTAL_COIN or item:getId() == ITEM_PLATINUM_COIN or item:getId() == ITEM_GOLD_COIN)) then
				self:sendCancelMessage("You can move only money to this container.")
				return false
			end
		end
	end

	-- No move items with actionID 8000
	if item:getActionId() == NOT_MOVEABLE_ACTION then
		self:sendCancelMessage(RETURNVALUE_NOTPOSSIBLE)
		return false
	end

	-- Check two-handed weapons
	if toPosition.x ~= CONTAINER_POSITION then
		return true
	end

	if item:getTopParent() == self and bit.band(toPosition.y, 0x40) == 0 then
		local itemType, moveItem = ItemType(item:getId())
		if bit.band(itemType:getSlotPosition(), SLOTP_TWO_HAND) ~= 0 and toPosition.y == CONST_SLOT_LEFT then
			moveItem = self:getSlotItem(CONST_SLOT_RIGHT)
		elseif itemType:getWeaponType() == WEAPON_SHIELD and toPosition.y == CONST_SLOT_RIGHT then
			moveItem = self:getSlotItem(CONST_SLOT_LEFT)
			if moveItem and bit.band(ItemType(moveItem:getId()):getSlotPosition(), SLOTP_TWO_HAND) == 0 then
				return true
			end
		end

		if moveItem then
			local parent = item:getParent()
			if parent:getSize() == parent:getCapacity() then
				self:sendTextMessage(MESSAGE_STATUS_SMALL, Game.getReturnMessage(RETURNVALUE_CONTAINERNOTENOUGHROOM))
				return false
			else
				return moveItem:moveTo(parent)
			end
		end
	end

	-- Reward System
	if toPosition.x == CONTAINER_POSITION then
		local containerId = toPosition.y - 64
		local container = self:getContainerById(containerId)
		if not container then
			return true
		end

		-- Do not let the player insert items into either the Reward Container or the Reward Chest
		local itemId = container:getId()
		if itemId == ITEM_REWARD_CONTAINER or itemId == ITEM_REWARD_CHEST then
			self:sendCancelMessage(RETURNVALUE_NOTPOSSIBLE)
			return false
		end

		-- The player also shouldn't be able to insert items into the boss corpse
		local tile = Tile(container:getPosition())
		for _, item in ipairs(tile:getItems() or { }) do
			if item:getAttribute(ITEM_ATTRIBUTE_CORPSEOWNER) == 2^31 - 1 and item:getName() == container:getName() then
				self:sendCancelMessage(RETURNVALUE_NOTPOSSIBLE)
				return false
			end
		end
	end

	-- Do not let the player move the boss corpse.
	if item:getAttribute(ITEM_ATTRIBUTE_CORPSEOWNER) == 2^31 - 1 then
		self:sendCancelMessage(RETURNVALUE_NOTPOSSIBLE)
		return false
	end

	-- Players cannot throw items on reward chest
	local tile = Tile(toPosition)
	if tile and tile:getItemById(ITEM_REWARD_CHEST) then
		self:sendCancelMessage(RETURNVALUE_NOTPOSSIBLE)
		self:getPosition():sendMagicEffect(CONST_ME_POFF)
		return false
	end

	-- Players cannot throw items on teleports
	if blockTeleportTrashing and toPosition.x ~= CONTAINER_POSITION then
		local thing = Tile(toPosition):getItemByType(ITEM_TYPE_TELEPORT)
		if thing then
			self:sendCancelMessage(RETURNVALUE_NOTPOSSIBLE)
			self:getPosition():sendMagicEffect(CONST_ME_POFF)
			return false
		end
	end

	--[[-- Do not stop trying this test
	-- No move parcel very heavy
	if item:getWeight() > 90000 and item:getId() == ITEM_PARCEL then
		self:sendCancelMessage('YOU CANNOT MOVE PARCELS TOO HEAVY.')
		return false
	end

	-- No move if item count > 26 items
	if tile and tile:getItemCount() > 26 then
		self:sendCancelMessage(RETURNVALUE_NOTPOSSIBLE)
		return false
	end

	if tile and tile:getItemById(370) then -- Trapdoor
		self:sendCancelMessage(RETURNVALUE_NOTPOSSIBLE)
		self:getPosition():sendMagicEffect(CONST_ME_POFF)
		return false
	end]]

	return true
end

function Player:onMoveCreature(creature, fromPosition, toPosition)
	return true
end

function Player:onReport(message, position, category)
	if self:getAccountType() == ACCOUNT_TYPE_NORMAL then
		return false
	end

	local name = self:getName()
	local file = io.open("data/reports/" .. name .. " report.txt", "a")

	if not file then
		self:sendTextMessage(MESSAGE_EVENT_DEFAULT, "There was an error when processing your report, please contact a gamemaster.")
		return true
	end

	io.output(file)
	io.write("------------------------------\n")
	io.write("Name: " .. name)
	if category == BUG_CATEGORY_MAP then
		io.write(" [Map position: " .. position.x .. ", " .. position.y .. ", " .. position.z .. "]")
	end
	local playerPosition = self:getPosition()
	io.write(" [Player Position: " .. playerPosition.x .. ", " .. playerPosition.y .. ", " .. playerPosition.z .. "]\n")
	io.write("Comment: " .. message .. "\n")
	io.close(file)

	self:sendTextMessage(MESSAGE_EVENT_DEFAULT, "Your report has been sent to " .. configManager.getString(configKeys.SERVER_NAME) .. ".")
	return true
end

function Player:onTurn(direction)
	if self:getGroup():getAccess() and self:getDirection() == direction then
		local nextPosition = self:getPosition()
		nextPosition:getNextPosition(direction)

		self:teleportTo(nextPosition, true)
	end

	return true
end

function Player:onTradeRequest(target, item)
	return true
end

function Player:onTradeAccept(target, item, targetItem)
	return true
end

local soulCondition = Condition(CONDITION_SOUL, CONDITIONID_DEFAULT)
soulCondition:setTicks(4 * 60 * 1000)
soulCondition:setParameter(CONDITION_PARAM_SOULGAIN, 1)

local function useStamina(player)
	local staminaMinutes = player:getStamina()
	if staminaMinutes == 0 then
		return
	end

	local playerId = player:getId()
	local currentTime = os.time()
	local timePassed = currentTime - nextUseStaminaTime[playerId]
	if timePassed <= 0 then
		return
	end

	if timePassed > 60 then
		if staminaMinutes > 2 then
			staminaMinutes = staminaMinutes - 2
		else
			staminaMinutes = 0
		end
		nextUseStaminaTime[playerId] = currentTime + 120
	else
		staminaMinutes = staminaMinutes - 1
		nextUseStaminaTime[playerId] = currentTime + 60
	end
	player:setStamina(staminaMinutes)
end

-- useStaminaPrey
local function useStaminaPrey(player, name)
	for i = 1, 3 do
		if (player:isActiveByName(i-1, name)) then
			local staminaMinutes = player:getPreyStamina(i-1)/60
			if (staminaMinutes > 0) then
				local playerId = player:getId()+i
				local currentTime = os.time()
				local timePassed = currentTime - nextUseStaminaPrey[playerId].Time
				if timePassed > 0 then
					if timePassed > 60 then
						if staminaMinutes > 2 then
							staminaMinutes = staminaMinutes - 2
						else
							staminaMinutes = 0
						end

						nextUseStaminaPrey[playerId].Time = currentTime + 120
					else
						staminaMinutes = staminaMinutes - 1
						nextUseStaminaPrey[playerId].Time = currentTime + 60
					end
				end

				player:setPreyStamina(i-1, staminaMinutes*60)
				player:sendPreyTimeLeft(i-1, staminaMinutes*60)
			end
		end
	end
end

-- exp card
local BONUS_EXP_STORAGE = 61398
local BONUS_EXP_MULT = 1.3

local configexp =  {
	["Monday"] = 1.0,
	["Tuesday"] = 1.0,
	["Wednesday"] = 1.0,
	["Thursday"] = 1.0,
	["Friday"] = 1.0,
	["Saturday"] = 2.0,
	["Sunday"] = 2.0
}

function Player:onGainExperience(source, exp, rawExp)
	if not source or source:isPlayer() then
		return exp
	end

	exp = exp * configexp[os.date("%A")]

	-- Soul regeneration
	local vocation = self:getVocation()
	if self:getSoul() < vocation:getMaxSoul() and exp >= self:getLevel() then
		soulCondition:setParameter(CONDITION_PARAM_SOULTICKS, vocation:getSoulGainTicks() * 1000)
		self:addCondition(soulCondition)
	end

	-- Apply experience stage multiplier
	exp = exp * Game.getExperienceStage(self:getLevel())

	-- Prey System -> BOOST_EXP
	for i = 1, 3 do
		if (self:isActive(i-1)) then
			local bonusInfo = self:getBonusInfo(i-1)
			if (bonusInfo.Type == 2 and source:getName() == bonusInfo.Name) then
				exp = exp + math.floor(exp * (bonusInfo.Value/100))
				break
			end
		end
	end

	-- Stamina modifier
	if configManager.getBoolean(configKeys.STAMINA_SYSTEM) then
		useStamina(self)

		local staminaMinutes = self:getStamina()
		if staminaMinutes > 2400 and self:isPremium() then
			exp = exp * 1.5
		elseif staminaMinutes <= 840 then
			exp = exp * 0.5
		end
	end

	-- Prey Stamina Modifier
	useStaminaPrey(self, source:getName())

	-- Exp Card
	if self:getStorageValue(BONUS_EXP_STORAGE) - os.time() > 0 then
		exp = exp * BONUS_EXP_MULT
	end

	return exp
end

function Player:onLoseExperience(exp)
	return exp
end

function Player:onGainSkillTries(skill, tries)
	if APPLY_SKILL_MULTIPLIER == false then
		return tries
	end

	if skill == SKILL_MAGLEVEL then
		return tries * configManager.getNumber(configKeys.RATE_MAGIC)
	end
	return tries * configManager.getNumber(configKeys.RATE_SKILL)
end

local function getHours(seconds)
	return math.floor((seconds/60)/60)
end

local function getMinutes(seconds)
	return math.floor(seconds/60)
end

local function getTime(seconds)
	local hours, minutes = getHours(seconds), getMinutes(seconds)
	if (minutes > 59) then
		minutes = minutes-hours*60
	end

	if (minutes < 10) then
		minutes = "0" ..minutes
	end

	return hours..":"..minutes.. "h"
end
