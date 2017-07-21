local targetIdList = {
	--health potions casks
	[28553] = {itemId = 7636, transform = 7618}, [28577] = {itemId = 7636, transform = 7618},
	[28554] = {itemId = 7634, transform = 7588}, [28578] = {itemId = 7634, transform = 7588},
	[28555] = {itemId = 7635, transform = 7591}, [28579] = {itemId = 7635, transform = 7591},
	[28556] = {itemId = 7635, transform = 8473}, [28580] = {itemId = 7635, transform = 8473},
	[28557] = {itemId = 7635, transform = 26031}, [28581] = {itemId = 7635, transform = 26031},
	--mana potions casks
	[28563] = {itemId = 7636, transform = 7620}, [28582] = {itemId = 7636, transform = 7620},
	[28564] = {itemId = 7634, transform = 7589}, [28583] = {itemId = 7634, transform = 7589},
	[28565] = {itemId = 7635, transform = 7590}, [28584] = {itemId = 7635, transform = 7590},
	[28566] = {itemId = 7635, transform = 26029}, [28585] = {itemId = 7635, transform = 26029},
	--spirit potions caks
	[28573] = {itemId = 7635, transform = 8472}, [28587] = {itemId = 7635, transform = 8472},
	[28574] = {itemId = 7635, transform = 26030}, [28588] = {itemId = 7635, transform = 26030},
}

function onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if target:getId() >= 28533 and target:getId() <= 28588 then
		local house = player:getTile():getHouse()
		if house and house:canEditAccessList(SUBOWNER_LIST, player) and house:canEditAccessList(doorId, player) or target:getId() >= 28577 then 
		elseif target:getId() >= 28533 and target:getId() < 28577 then
			player:sendTextMessage(MESSAGE_EVENT_ADVANCE, 'Sorry, casks only can be useds inside house.')
			return false
		else
			return false
		end

		if target then
			local charges = target:getCharges()
			if item:getCount() > charges then
				player:sendTextMessage(MESSAGE_EVENT_ADVANCE, string.format('Sorry, only Remaining %s charges.', charges))
				return false
			end

			local targetId = targetIdList[target:getId()]
			if targetId then
				if item:getId() == targetId.itemId then
					item:transform(targetId.transform)
					charges = charges - item:getCount()
					target:transform(target:getId(), charges)
					player:sendTextMessage(MESSAGE_EVENT_ADVANCE, string.format('Remaining %s charges.', charges)) 
					if charges == 0 then
						target:remove()
					end
				end
			end
		end

		return true
	end
end

