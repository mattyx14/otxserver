local specialQuests = {
	-- 
}

local questsExperience = {
	-- 
}

local questLog = {
	-- 
}

local function copyContainerItem(originalContainer, newContainer)
	for i = 0, originalContainer:getSize() - 1 do
		local originalItem = originalContainer:getItem(i)
		local newItem = Game.createItem(originalItem.itemid, originalItem.type)
		if not newItem then
			logger.error("[questSystem.copyContainerItem] failed to create item: {}", originalItem.itemid)
			return false
		end
		newItem:setActionId(originalItem:getActionId())
		newItem:setAttribute(ITEM_ATTRIBUTE_DESCRIPTION, originalItem:getAttribute(ITEM_ATTRIBUTE_DESCRIPTION))

		if originalItem:isContainer() then
			copyContainerItem(Container(originalItem.uid), Container(newItem.uid))
		end
		newContainer:addItemEx(newItem)
	end
end

local hotaQuest = { 50950, 50951, 50952, 50953, 50954, 50955 }

local questSystem = Action()

function questSystem.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	local storage = specialQuests[item.actionid]
	if not storage then
		storage = item.uid
		if storage > 65535 then
			return false
		end
	end

	if player:getStorageValue(storage) > 0 and player:getGroup():getId() < GROUP_TYPE_GAMEMASTER then
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "The " .. ItemType(item.itemid):getName() .. " is empty.")
		return true
	end

	local items, reward = {}
	local size = item:isContainer() and item:getSize() or 0
	if size == 0 then
		reward = Game.createItem(item.itemid, item.type)
		if not reward then
			logger.error("[questSystem.onUse] failed to create reward item")
			return false
		end

		local itemActionId = item:getActionId()
		if itemActionId then
			reward:setActionId(itemActionId)
		end
		local itemDescription = item:getAttribute(ITEM_ATTRIBUTE_DESCRIPTION)
		if itemDescription then
			reward:setAttribute(ITEM_ATTRIBUTE_DESCRIPTION, itemDescription)
		end
	else
		local container = Container(item.uid)
		if not container then
			logger.error("[questSystem.onUse] failed to create container")
			return false
		end
		for i = 0, container:getSize() - 1 do
			local originalItem = container:getItem(i)
			local newItem = Game.createItem(originalItem.itemid, originalItem.type)
			if not newItem then
				logger.error("[questSystem.onUse] failed to create new item")
				return false
			end
			local newActionId = originalItem:getActionId()
			if newActionId then
				newItem:setActionId(newActionId)
			end
			local newDescription = item:getAttribute(ITEM_ATTRIBUTE_DESCRIPTION)
			if newDescription then
				newItem:setAttribute(ITEM_ATTRIBUTE_DESCRIPTION, newDescription)
			end

			if originalItem:isContainer() then
				copyContainerItem(Container(originalItem.uid), Container(newItem.uid))
			end
			items[#items + 1] = newItem
		end

		if size == 1 then
			reward = items[1]
		end
	end

	local result = ""
	if reward then
		local ret = ItemType(reward.itemid)
		if ret:isRune() then
			result = ret:getArticle() .. " " .. ret:getName() .. " (" .. reward.type .. " charges)"
		elseif ret:isStackable() and reward:getCount() > 1 then
			result = reward:getCount() .. " " .. ret:getPluralName()
		elseif ret:getArticle() ~= "" then
			result = ret:getArticle() .. " " .. ret:getName()
		else
			result = ret:getName()
		end
	else
		if size > 20 then
			reward = Game.createItem(item.itemid, 1)
		elseif size > 8 then
			reward = Game.createItem(2854, 1)
		else
			reward = Game.createItem(2853, 1)
		end

		for i = 1, size do
			local tmp = items[i]
			if reward:addItemEx(tmp) ~= RETURNVALUE_NOERROR then
				logger.warn("[questSystem.onUse] - Could not add quest reward to container")
			end
		end
		local ret = ItemType(reward.itemid)
		result = ret:getArticle() .. " " .. ret:getName()
	end

	if player:addItemEx(reward) ~= RETURNVALUE_NOERROR then
		local weight = reward:getWeight()
		if player:getFreeCapacity() < weight then
			player:sendCancelMessage(string.format("You have found %s weighing %.2f oz. You have no capacity.", result, (weight / 100)))
		else
			player:sendCancelMessage("You have found " .. result .. ", but you have no room to take it.")
		end
		return true
	end

	if questsExperience[storage] then
		player:addExperience(questsExperience[storage], true)
	end

	if questLog[storage] then
		player:setStorageValue(questLog[storage], 1)
	end

	player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You have found " .. result .. ".")
	player:setStorageValue(storage, 1)
	return true
end

for index, value in pairs(specialQuests) do
	questSystem:aid(index)
end

questSystem:aid(2000)
questSystem:register()
