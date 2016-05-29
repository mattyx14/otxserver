CONST_SLOT_TWOHANDED = 19
local slotconfig = {
	[49] = CONST_SLOT_HEAD,
	[50] = CONST_SLOT_NECKLACE,
	[56] = CONST_SLOT_ARMOR,
	[48] = CONST_SLOT_LEFT,
	[304] = CONST_SLOT_RING,
	[112] = CONST_SLOT_LEGS,
	[176] = CONST_SLOT_FEET,
	[560] = CONST_SLOT_AMMO,
	[2096] = CONST_SLOT_TWOHANDED
}

function onRecvbyte(player, msg, byte)
	local clientId = msg:getU16()
	local item = player:getItemByClietnId(clientId)
	local itemType = Game.getItemIdByClientId(clientId)
	
	if not item then
		item = player:getItemById(itemType:getTransformEquipId())
		if not item then
			return
		end
	end

	local slot = slotconfig[itemType:getSlotPosition()]
	
	if slot == CONST_SLOT_LEFT and itemType:getWeaponType() == WEAPON_SHIELD then
		slot = CONST_SLOT_RIGHT
	end

	if slot == CONST_SLOT_TWOHANDED then
		local slotItemLeft = player:getSlotItem(CONST_SLOT_LEFT)
		local slotItemRight = player:getSlotItem(CONST_SLOT_RIGHT)

		if slotItemLeft == item then
			slotItemLeft:moveToSlot(player, CONST_SLOT_WHEREEVER)
		elseif slotItemRight == item then
			slotItemRight:moveToSlot(player, CONST_SLOT_WHEREEVER)
		else
			if slotItemLeft ~= nil then
				slotItemLeft:moveToSlot(player, CONST_SLOT_WHEREEVER)
			end

			if slotItemRight then
				slotItemRight:moveToSlot(player, CONST_SLOT_WHEREEVER)
			end

			if itemType:getWeaponType() == WEAPON_SHIELD then
				item:moveToSlot(player, CONST_SLOT_RIGHT)
			else
				item:moveToSlot(player, CONST_SLOT_LEFT)
			end
		end
	else
		local slotItem = player:getSlotItem(slot)
		if not slotItem then
			tem:moveToSlot(player, slot)
		elseif slotIem == item then
			item:moveToSlot(player, CONST_SLOT_WHEREEVER)
		else
			slotItem:moveToSlot(player, CONST_SLOT_WHEREEVER)
			item:moveToSlot(player, slot)
		end
	end
	return true
end
