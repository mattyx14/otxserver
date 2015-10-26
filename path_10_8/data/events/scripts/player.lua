function Player:onBrowseField(position)
	return true
end

-- low level experience bonus
-- bonus is a floating percentage, use 1 for 100% bonus
local expBonusDelta = expBonus.maxlevel - expBonus.minlevel -- don't touch
local expBonus = {
	minlevel = 2,
	maxlevel = 50,
	bonus = 1
}

-- minlevel and multiplier are MANDATORY
-- maxlevel is OPTIONAL, but is considered infinite by default
-- create a stage with minlevel 1 and no maxlevel to disable stages
local experienceStages = {
	{
		minlevel = 1,
		maxlevel = 8,
		multiplier = 7
	}, {
		minlevel = 9,
		maxlevel = 20,
		multiplier = 6
	}, {
		minlevel = 21,
		maxlevel = 50,
		multiplier = 5
	}, {
		minlevel = 51,
		maxlevel = 100,
		multiplier = 4
	}, {
		minlevel = 101,
		multiplier = 3
	}
}

local skillStages = {
	{
		minlevel = 10,
		maxlevel = 50,
		multiplier = 3
	}, {
		minlevel = 51,
		maxlevel = 100,
		multiplier = 2
	}, {
		minlevel = 101,
		multiplier = 1
	}
}

local magicLevelStages = {
	{
		minlevel = 10,
		maxlevel = 50,
		multiplier = 3
	}, {
		minlevel = 51,
		maxlevel = 100,
		multiplier = 2
	}, {
		minlevel = 101,
		multiplier = 1
	}
}

function Player:onLook(thing, position, distance)
	local description = "You see " .. thing:getDescription(distance)
	if self:getGroup():getAccess() then
		if thing:isItem() then
			description = string.format("%s\nItem ID: %d", description, thing:getId())

			local actionId = thing:getActionId()
			if actionId ~= 0 then
				description = string.format("%s, Action ID: %d", description, actionId)
			end

			local uniqueId = thing:getAttribute(ITEM_ATTRIBUTE_UNIQUEID)
			if uniqueId > 0 and uniqueId < 65536 then
				description = string.format("%s, Unique ID: %d", description, uniqueId)
			end

			local itemType = thing:getType()

			local transformEquipId = itemType:getTransformEquipId()
			local transformDeEquipId = itemType:getTransformDeEquipId()
			if transformEquipId ~= 0 then
				description = string.format("%s\nTransforms to: %d (onEquip)", description, transformEquipId)
			elseif transformDeEquipId ~= 0 then
				description = string.format("%s\nTransforms to: %d (onDeEquip)", description, transformDeEquipId)
			end

			local decayId = itemType:getDecayId()
			if decayId ~= -1 then
				description = string.format("%s\nDecays to: %d", description, decayId)
			end
		elseif thing:isCreature() then
			local str = "%s\nHealth: %d / %d"
			if thing:getMaxMana() > 0 then
				str = string.format("%s, Mana: %d / %d", str, thing:getMana(), thing:getMaxMana())
			end
			description = string.format(str, description, thing:getHealth(), thing:getMaxHealth()) .. "."
		end

		local position = thing:getPosition()
		description = string.format(
			"%s\nPosition: %d, %d, %d",
			description, position.x, position.y, position.z
		)

		if thing:isCreature() then
			if thing:isPlayer() then
				description = string.format("%s\nIP: %s.", description, Game.convertIpToString(thing:getIp()))
			end
		end
	end
	self:sendTextMessage(MESSAGE_INFO_DESCR, description)
end

function Player:onLookInBattleList(creature, distance)
	local description = "You see " .. creature:getDescription(distance)
	if self:getGroup():getAccess() then
		local str = "%s\nHealth: %d / %d"
		if creature:getMaxMana() > 0 then
			str = string.format("%s, Mana: %d / %d", str, creature:getMana(), creature:getMaxMana())
		end
		description = string.format(str, description, creature:getHealth(), creature:getMaxHealth()) .. "."

		local position = creature:getPosition()
		description = string.format(
			"%s\nPosition: %d, %d, %d",
			description, position.x, position.y, position.z
		)

		if creature:isPlayer() then
			description = string.format("%s\nIP: %s", description, Game.convertIpToString(creature:getIp()))
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

function Player:onMoveItem(item, count, fromPosition, toPosition)
	return true
end

function Player:onMoveCreature(creature, fromPosition, toPosition)
	return true
end

function Player:onTurn(direction)
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

local function getRateFromTable(t, level, default)
	for _, rate in ipairs(t) do
		if level >= rate.minlevel then
			if rate.maxlevel == nil or level <= rate.maxlevel then
				return rate.multiplier
			end
		end
	end

	return default
end

function Player:onGainExperience(source, exp, rawExp)
	if not source or source:isPlayer() then
		return exp
	end

	local level = self:getLevel()

	-- Soul regeneration
	local vocation = self:getVocation()
	if self:getSoul() < vocation:getMaxSoul() and exp >= level then
		soulCondition:setParameter(CONDITION_PARAM_SOULTICKS, vocation:getSoulGainTicks() * 1000)
		self:addCondition(soulCondition)
	end

	-- Apply experience stage multiplier
	exp = exp * getRateFromTable(experienceStages, level, configManager.getNumber(configKeys.RATE_EXPERIENCE))

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

	-- Apply low level bonus
	local multiplier = expBonus.bonus * math.min(1, math.max(0, (expBonus.maxlevel - level) / (expBonusDelta))) + 1
		exp = exp * multiplier

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
		return tries * getRateFromTable(magicStages, self:getSkillLevel(skill), configManager.getNumber(configKeys.RATE_MAGIC))
	end
	return tries * getRateFromTable(skillStages, self:getSkillLevel(skill), configManager.getNumber(configKeys.RATE_SKILL))
end

function Player:onSave(GUID)
	return true
end
