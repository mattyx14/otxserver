local config = {
	increasing = {[416] = 417, [426] = 425, [446] = 447, [3216] = 3217, [3202] = 3215, [11062] = 11063},
	decreasing = {[417] = 416, [425] = 426, [447] = 446, [3217] = 3216, [3215] = 3202, [11063] = 11062},
	maxLevel = getConfigInfo('maximumDoorLevel')
}

local checkCreature = {isPlayer, isMonster, isNpc}
local function pushBack(cid, position, fromPosition, displayMessage)
	doTeleportThing(cid, fromPosition, false)
	doSendMagicEffect(position, CONST_ME_MAGIC_BLUE)
	if(displayMessage) then
		doPlayerSendTextMessage(cid, MESSAGE_INFO_DESCR, "The tile seems to be protected against unwanted intruders.")
	end
end

function onStepIn(cid, item, position, fromPosition)
	if(not config.increasing[item.itemid]) then
		return false
	end

	if(not isPlayerGhost(cid)) then
		doTransformItem(item.uid, config.increasing[item.itemid])
	end

	if(item.actionid >= 194 and item.actionid <= 196) then
		local f = checkCreature[item.actionid - 193]
		if(f(cid)) then
			pushBack(cid, position, fromPosition, false)
		end

		return true
	end

	if(item.actionid >= 191 and item.actionid <= 193) then
		local f = checkCreature[item.actionid - 190]
		if(not f(cid)) then
			pushBack(cid, position, fromPosition, false)
		end

		return true
	end

	if(not isPlayer(cid)) then
		return true
	end

	if(item.actionid == 189 and not isPremium(cid)) then
		pushBack(cid, position, fromPosition, true)
		return true
	end

	local gender = item.actionid - 186
	if(isInArray({PLAYERSEX_FEMALE,  PLAYERSEX_MALE, PLAYERSEX_GAMEMASTER}, gender)) then
		if(gender ~= getPlayerSex(cid)) then
			pushBack(cid, position, fromPosition, true)
		end

		return true
	end

	local skull = item.actionid - 180
	if(skull >= SKULL_NONE and skull <= SKULL_BLACK) then
		if(skull ~= getCreatureSkullType(cid)) then
			pushBack(cid, position, fromPosition, true)
		end

		return true
	end

	local group = item.actionid - 150
	if(group >= 0 and group < 30) then
		if(group > getPlayerGroupId(cid)) then
			pushBack(cid, position, fromPosition, true)
		end

		return true
	end

	local vocation = item.actionid - 100
	if(vocation >= 0 and vocation < 50) then
		local playerVocation = getVocationInfo(getPlayerVocation(cid))
		if(playerVocation.id ~= vocation and playerVocation.fromVocation ~= vocation) then
			pushBack(cid, position, fromPosition, true)
		end

		return true
	end

	if(item.actionid >= 1000 and item.actionid - 1000 <= config.maxLevel) then
		if(getPlayerLevel(cid) < item.actionid - 1000) then
			pushBack(cid, position, fromPosition, true)
		end

		return true
	end

	if(item.actionid ~= 0 and getCreatureStorage(cid, item.actionid) <= 0) then
		pushBack(cid, position, fromPosition, true)
		return true
	end

	if(getTileInfo(position).protection) then
		local depotItem = getTileItemByType(getCreatureLookPosition(cid), ITEM_TYPE_DEPOT)
		if(depotItem.itemid ~= 0) then
			local depotItems = getPlayerDepotItems(cid, getDepotId(depotItem.uid))
			doPlayerSendTextMessage(cid, MESSAGE_STATUS_DEFAULT, "Your depot contains " .. depotItems .. " item" .. (depotItems > 1 and "s" or "") .. ".")
			return true
		end
	end

	return false
end

function onStepOut(cid, item, position, fromPosition)
	if(config.decreasing[item.itemid] and not isPlayerGhost(cid)) then
		doTransformItem(item.uid, config.decreasing[item.itemid])
	end

	return true
end
