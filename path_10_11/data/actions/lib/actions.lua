local holeId = {
	294, 369, 370, 383, 392, 408, 409, 410, 427, 428, 429, 430, 462, 469, 470, 482,
	484, 485, 489, 924, 1369, 3111, 3135, 3136, 4835, 4837, 7933, 7938, 8170, 8286,
	8285, 8284, 8281, 8280, 8279, 8277, 8276, 8567, 8585, 8596, 8595, 8249, 8250, 8251,
	8323, 8252, 8253, 8254, 8255, 8256, 8592, 8972, 9606, 9625, 13190, 14461, 19519, 21536
}

local holes = {468, 481, 483, 7932, 23712}

local pickHoleIds = {354, 355}

local others = {7932}

local JUNGLE_GRASS = {2782, 3985, 19433}
local WILD_GROWTH = {1446, 1447, 1499, 1775, 2101, 11099}

local function revertItem(position, itemId, transformId)
	local item = Tile(position):getItemById(itemId)
	if item then
		item:transform(transformId)
	end
end

local function removeRemains(toPosition)
	local item = Tile(toPosition):getItemById(2248)
	if item then
		item:remove()
	end
end

local function revertCask(position)
	local caskItem = Tile(position):getItemById(2249)
	if caskItem then
		caskItem:transform(5539)
		position:sendMagicEffect(CONST_ME_MAGIC_GREEN)
	end
end

function destroyItem(player, item, fromPosition, target, toPosition, isHotkey)
	if type(target) ~= "userdata" or not target:isItem() then
		return false
	end

	if target:hasAttribute(ITEM_ATTRIBUTE_UNIQUEID) or target:hasAttribute(ITEM_ATTRIBUTE_ACTIONID) then
		return false
	end

	if toPosition.x == CONTAINER_POSITION then
		player:sendCancelMessage(Game.getReturnMessage(RETURNVALUE_NOTPOSSIBLE))
		return true
	end

	local destroyId = ItemType(target.itemid):getDestroyId()
	if destroyId == 0 then
		return false
	end

	if math.random(7) == 1 then
		local item = Game.createItem(destroyId, 1, toPosition)
		if item ~= nil then
			item:decay()
		end

		-- Move items outside the container
		if target:isContainer() then
			for i = target:getSize() - 1, 0, -1 do
				local containerItem = target:getItem(i)
				if containerItem then
					containerItem:moveTo(toPosition)
				end
			end
		end

		target:remove(1)
	end

	toPosition:sendMagicEffect(CONST_ME_POFF)
	return true
end

function onUseRope(player, item, fromPosition, target, toPosition, isHotkey)
	local tile = Tile(toPosition)
	if not tile then
		return false
	end

	if table.contains(ropeSpots, tile:getGround():getId()) or tile:getItemById(14435) then
		tile = Tile(toPosition:moveUpstairs())
		if tile:hasFlag(TILESTATE_PROTECTIONZONE) and player:isPzLocked() then
			player:sendTextMessage(MESSAGE_STATUS_SMALL, Game.getReturnMessage(RETURNVALUE_PLAYERISPZLOCKED))
			return true
		end
		player:teleportTo(toPosition, false)
		return true
	elseif table.contains(holeId, target.itemid) then
		toPosition.z = toPosition.z + 1
		tile = Tile(toPosition)
		if tile then
			local thing = tile:getTopVisibleThing()
			if thing:isPlayer() then
				tile = Tile(toPosition:moveUpstairs())
				if tile:hasFlag(TILESTATE_PROTECTIONZONE) and thing:isPzLocked() then
					return false
				end
				return thing:teleportTo(toPosition, false)
			end
			if thing:isItem() and thing:getType():isMovable() then
				return thing:moveTo(toPosition:moveUpstairs())
			end
		end

		player:sendCancelMessage(RETURNVALUE_NOTPOSSIBLE)
		return true
	end

	return false
end

function onUseShovel(player, item, fromPosition, target, toPosition, isHotkey)
	local targetId = target.itemid, target.actionid
	if table.contains(others, targetId) then
		target:transform(targetId + 1)
		target:decay()

		return true
	end

	if toPosition.x == CONTAINER_POSITION then
		return false
	end

	local tile = Tile(toPosition)
	if not tile then
		return false
	end

	local ground = tile:getGround()
	if not ground then
		return false
	end

	local groundId = ground:getId()
	if table.contains(holes, groundId) then
		ground:transform(groundId + 1)
		ground:decay()

		toPosition.z = toPosition.z + 1
		tile:relocateTo(toPosition)
	elseif groundId == 231 then
		local randomValue = math.random(1, 100)
		if randomValue == 1 then
			Game.createItem(2159, 1, toPosition)
		elseif randomValue > 95 then
			Game.createMonster("Scarab", toPosition)
		end
		toPosition:sendMagicEffect(CONST_ME_POFF)
	else
		return false
	end

	return true
end

function onUsePick(player, item, fromPosition, target, toPosition, isHotkey)
	if toPosition.x == CONTAINER_POSITION then
		return false
	end

	local tile = Tile(toPosition)
	if not tile then
		return false
	end

	local ground = tile:getGround()
	if not ground then
		return false
	end

	if (ground.uid > 65535 or ground.actionid == 0) and not table.contains(pickHoleIds, ground.itemid) then
		return false
	end

	ground:transform(392)
	ground:decay()

	toPosition.z = toPosition.z + 1
	tile:relocateTo(toPosition)
	return true
end

function onUseMachete(player, item, fromPosition, target, toPosition, isHotkey)
	local targetId = target.itemid
	if table.contains(JUNGLE_GRASS, targetId) then
		target:transform(targetId == 19433 and 19431 or targetId - 1)
		target:decay()
		return true
	end

	if table.contains(WILD_GROWTH, targetId) then
		toPosition:sendMagicEffect(CONST_ME_POFF)
		target:remove()
		return true
	end

	return destroyItem(player, item, fromPosition, target, toPosition, isHotkey)
end

function onUseScythe(player, item, fromPosition, target, toPosition, isHotkey)
	if not table.contains({2550, 10513}, item.itemid) then
		return false
	end

	if target.itemid == 5464 then
		target:transform(5463)
		target:decay()
		Game.createItem(5467, 1, toPosition)
		return true
	end

	if target.itemid == 2739 then
		target:transform(2737)
		target:decay()
		Game.createItem(2694, 1, toPosition)
		return true
	end

	return destroyItem(player, item, fromPosition, target, toPosition, isHotkey)
end
