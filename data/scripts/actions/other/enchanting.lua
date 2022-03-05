local config = {
	manaCost = 300,
	soulCost = 2,
}

local spheres = {
	[675] = {VOCATION.BASE_ID.PALADIN},
	[676] = {VOCATION.BASE_ID.SORCERER},
	[677] = {VOCATION.BASE_ID.DRUID},
	[678] = {VOCATION.BASE_ID.KNIGHT}
}

local enchantableGems = {3030, 3029, 3032, 3033}
local enchantableItems = {3447}

local enchantingAltars = {
	{146, 147, 148, 149},
	{150, 151, 152, 153},
	{158, 159, 160, 161},
	{154, 155, 156, 157}
}

local enchantedGems = {676, 675, 677, 678}
local enchantedItems = {
	[3447] = {761, 762, 773, 764}
}

local enchanting = Action()

function enchanting.onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if item.itemid == 3030 and target.itemid == 3229 then
		target:transform(3230)
		target:decay()
		item:remove(1)
		toPosition:sendMagicEffect(CONST_ME_MAGIC_RED)
		return true
	end

	if item.itemid == 676 and isInArray({3123, 9020}, target.itemid) then
		target:transform(9019)
		item:remove(1)
		toPosition:sendMagicEffect(CONST_ME_MAGIC_RED)
		return true
	end

	if isInArray(enchantableGems, item.itemid) then
		local subtype = item.type
		if subtype == 0 then
			subtype = 1
		end

		local mana = config.manaCost * subtype
		if player:getMana() < mana then
			player:say('Not enough mana, separate one gem in your backpack and try again.', TALKTYPE_MONSTER_SAY)
			return false
		end

		local soul = config.soulCost * subtype
		if player:getSoul() < soul then
			player:sendCancelMessage(RETURNVALUE_NOTENOUGHSOUL)
			return false
		end

		local targetId = table.find(enchantableGems, item.itemid)
		if not targetId or not isInArray(enchantingAltars[targetId], target.itemid) then
			return false
		end

		player:addMana(-mana)
		player:addSoul(-soul)
		item:transform(enchantedGems[targetId])
		player:addManaSpent(items.valuables.mana)
		player:getPosition():sendMagicEffect(CONST_ME_HOLYDAMAGE)
		return true
	end

	if item.itemid == 677 and isInArray({9035, 9040}, target.itemid) then
		target:transform(target.itemid - 1)
		target:decay()
		item:remove(1)
		toPosition:sendMagicEffect(CONST_ME_MAGIC_GREEN)
		return true
	end

	if isInArray(enchantedGems, item.itemid) then
		if not isInArray(enchantableItems, target.itemid) then
			fromPosition:sendMagicEffect(CONST_ME_POFF)
			return false
		end

		local targetId = table.find(enchantedGems, item.itemid)
		if not targetId then
			return false
		end

		local subtype = target.type
		if not isInArray({3447, 8077}, target.itemid) then
			subtype = 1000
		end

		target:transform(enchantedItems[target.itemid][targetId], subtype)
		target:getPosition():sendMagicEffect(CONST_ME_MAGIC_RED)
		item:remove(1)
		return true
	end
	return false
end

enchanting:id(675, 676, 677, 678)
enchanting:register()
