local config = {
	hardcoreManaSpent = getConfigValue("addManaSpentInPvPZone"),
	manaCost = 300,
	soulCost = 2
}

function onUse(cid, item, fromPosition, itemEx, toPosition)
	if(item.itemid == 2147 and itemEx.itemid == 2342) then
		doTransformItem(itemEx.uid, 2343)
		doDecayItem(itemEx.uid)
		doRemoveItem(item.uid, 1)

		doSendMagicEffect(toPosition, CONST_ME_MAGIC_RED)
		return true
	end

	if(item.itemid == 7760 and isInArray({9934, 10022}, itemEx.itemid)) then
		doTransformItem(itemEx.uid, 9933)
		doRemoveItem(item.uid, 1)

		doSendMagicEffect(toPosition, CONST_ME_MAGIC_RED)
		return true
	end

	if(item.itemid == 7761 and isInArray({9949, 9954}, itemEx.itemid)) then
		doTransformItem(itemEx.uid, itemEx.itemid - 1)
		doRemoveItem(item.uid, 1)

		doSendMagicEffect(toPosition, CONST_ME_MAGIC_GREEN)
		return true
	end

	if(isInArray(enchantableGems, item.itemid)) then
		local subtype = item.type
		if(subtype == 0) then
			subtype = 1
		end

		local mana = config.manaCost * subtype
		if(getPlayerMana(cid) < mana) then
			doPlayerSendDefaultCancel(cid, RETURNVALUE_NOTENOUGHMANA)
			return true
		end

		local soul = config.soulCost * subtype
		if(getPlayerSoul(cid) < soul) then
			doPlayerSendDefaultCancel(cid, RETURNVALUE_NOTENOUGHSOUL)
			return true
		end

		local a = table.find(enchantableGems, item.itemid)
		if(a == nil or not isInArray(enchantingAltars[a], itemEx.itemid)) then
			return false
		end

		doPlayerAddMana(cid, -mana)
		doPlayerAddSoul(cid, -soul)

		doTransformItem(item.uid, enchantedGems[a])
		if(not getPlayerFlagValue(cid, PlayerFlag_NotGainMana) and (not getTileInfo(getThingPosition(cid)).hardcore or config.hardcoreManaSpent)) then
			doPlayerAddSpentMana(cid, mana)
		end

		doSendMagicEffect(fromPosition, CONST_ME_HOLYDAMAGE)
		return true
	end
 
	if(isInArray(enchantedGems, item.itemid)) then
		if(not isInArray(enchantableItems, itemEx.itemid)) then
			doSendMagicEffect(fromPosition, CONST_ME_POFF)
			return false
		end

		local b = table.find(enchantedGems, item.itemid)
		if(b == nil) then
			return false
		end

		local subtype = itemEx.type
		if(not isInArray({2544, 8905}, itemEx.itemid)) then
			subtype = 1000
		end

		doTransformItem(itemEx.uid, enchantedItems[itemEx.itemid][b], subtype)
		doSendMagicEffect(getThingPos(itemEx.uid), CONST_ME_HOLYDAMAGE)
		doDecayItem(itemEx.uid)

		doRemoveItem(item.uid, 1)
		return true
	end

	return false
end
