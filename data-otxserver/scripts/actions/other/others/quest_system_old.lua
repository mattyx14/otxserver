local questSystemOld = Action()

function questSystemOld.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if player:getStorageValue(storage) > 0 and player:getGroup():getId() < GROUP_TYPE_GAMEMASTER then
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "The " .. ItemType(item.itemid):getName() .. " is empty.")
		return true
	end

	if item.uid <= 100 or item.uid >= 44613 then
		return false
	end

	local itemType = ItemType(item.uid)
	if itemType:getId() == 0 then
		return false
	end

	local itemWeight = itemType:getWeight()
	local playerCap = player:getFreeCapacity()
	if player:getStorageValue(item.uid) == -1 then
		if playerCap >= itemWeight then
			player:sendTextMessage(MESSAGE_EVENT_ADVANCE, 'You have found a ' .. itemType:getName() .. '.')
			player:addItem(item.uid, 1)
			player:setStorageValue(item.uid, 1)
		else
			player:sendTextMessage(MESSAGE_EVENT_ADVANCE, 'You have found a ' .. itemType:getName() .. ' weighing ' .. itemWeight .. ' oz it\'s too heavy.')
		end
	else
		player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "It is empty.")
	end
	return true
end

questSystemOld:id(2472, 2478, 2480, 2481, 2482, 4073, 4077, 5674, 5675, 7160, 7161)
questSystemOld:register()
