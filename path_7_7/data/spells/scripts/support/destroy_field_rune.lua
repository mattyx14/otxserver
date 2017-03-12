function onCastSpell(cid, var)
	local position = variantToPosition(var)
	local creaturesSize = #getAllCreatures(position)
	local stackSize = getTileStackItemsSize(position) + creaturesSize
	local itemsTable = {}
	local removed = false

	if stackSize > 0 then
		for i=1,stackSize do
			position.stackpos = i
			itemsTable[i] = getTileThingByPos(position)
		end
	end

	if #itemsTable > 0 then
		for k, item in pairs(itemsTable) do
			if isInArray(FIELDS, item.itemid) then
				doRemoveItem(item.uid)
				removed = true
			end
		end
	end

	if removed then
		doSendMagicEffect(position, CONST_ME_POFF)
		return true
	end

	doPlayerSendDefaultCancel(cid, RETURNVALUE_NOTPOSSIBLE)
	doSendMagicEffect(getCreaturePosition(cid), CONST_ME_POFF)
	return false
end