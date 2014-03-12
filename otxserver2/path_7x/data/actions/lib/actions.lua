SPOTS = { 384, 418 }
ROPABLE = { 294, 369, 370, 383, 392, 408, 409, 427, 428, 430, 462, 469, 470, 482, 484, 485, 489, 924, 3135, 3136 }

HOLES = { 468, 481, 483 }
SAND = { 231 }

JUNGLE_GRASS = { 2782, 3985 }
SPIDER_WEB = { 7538, 7539 }
BAMBOO_FENCE = { 3798, 3799 }
WILD_GROWTH = { 1499 }

PUMPKIN = 2683
PUMPKIN_HEAD = 2096

POOL = 2016


DOORS = {
	[1209] = 1211, [1212] = 1214, [1231] = 1233, [1234] = 1236, [1249] = 1251, [1252] = 1254, [3535] = 3537, [3544] = 3546, [4913] = 4915, [4916] = 4918
}

function destroyItem(cid, itemEx, toPosition)
	if(itemEx.uid <= 65535 or itemEx.actionid > 0) then
		return false
	end

	if(isInArray(SPIDER_WEB, itemEx.itemid)) then
		if math.random(3) == 1 then
			doTransformItem(itemEx.uid, (itemEx.itemid + 6))
			doDecayItem(itemEx.uid)
		end

		doSendMagicEffect(toPosition, CONST_ME_POFF)
		return true
	end

	if(isInArray(BAMBOO_FENCE, itemEx.itemid)) then
		if math.random(3) == 1 then
			if(itemEx.itemid == BAMBOO_FENCE[1]) then
				doTransformItem(itemEx.uid, (itemEx.itemid + 161))
			elseif(itemEx.itemid == BAMBOO_FENCE[2]) then
				doTransformItem(itemEx.uid, (itemEx.itemid + 159))
			end
			doDecayItem(itemEx.uid)
		end

		doSendMagicEffect(toPosition, CONST_ME_POFF)
		return true
	end

	if not(isInArray({1770, 2098, 1774, 1775, 2064, 2094, 2095, 1619, 2602, 3805, 3806}, itemEx.itemid) or
		(itemEx.itemid >= 1724 and itemEx.itemid <= 1741) or
		(itemEx.itemid >= 2581 and itemEx.itemid <= 2588) or
		(itemEx.itemid >= 1747 and itemEx.itemid <= 1753) or
		(itemEx.itemid >= 1714 and itemEx.itemid <= 1717) or
		(itemEx.itemid >= 1650 and itemEx.itemid <= 1653) or
		(itemEx.itemid >= 1666 and itemEx.itemid <= 1677) or
		(itemEx.itemid >= 1614 and itemEx.itemid <= 1616) or
		(itemEx.itemid >= 3813 and itemEx.itemid <= 3820) or
		(itemEx.itemid >= 3807 and itemEx.itemid <= 3810) or
		(itemEx.itemid >= 2080 and itemEx.itemid <= 2085) or
		(itemEx.itemid >= 2116 and itemEx.itemid <= 2119)) then
		return false
	end

	if(math.random(1, 7) == 1) then
		if(isInArray({1738, 1739, 1770, 2098, 1774, 1775, 2064}, itemEx.itemid) or
			(itemEx.itemid >= 2581 and itemEx.itemid <= 2588)) then
				doCreateItem(2250, 1, toPosition)
		elseif((itemEx.itemid >= 1747 and itemEx.itemid <= 1749) or itemEx.itemid == 1740) then
			doCreateItem(2251, 1, toPosition)
		elseif((itemEx.itemid >= 1714 and itemEx.itemid <= 1717)) then
			doCreateItem(2252, 1, toPosition)
		elseif((itemEx.itemid >= 1650 and itemEx.itemid <= 1653) or
			(itemEx.itemid >= 1666 and itemEx.itemid <= 1677) or
			(itemEx.itemid >= 1614 and itemEx.itemid <= 1616) or
			(itemEx.itemid >= 3813 and itemEx.itemid <= 3820) or
			(itemEx.itemid >= 3807 and itemEx.itemid <= 3810)) then
				doCreateItem(2253, 1, toPosition)
		elseif((itemEx.itemid >= 1724 and itemEx.itemid <= 1737) or
			(itemEx.itemid >= 2080 and itemEx.itemid <= 2085) or
			(itemEx.itemid >= 2116 and itemEx.itemid <= 2119) or
			isInArray({2094, 2095}, itemEx.itemid)) then
				doCreateItem(2254, 1, toPosition)
		elseif((itemEx.itemid >= 1750 and itemEx.itemid <= 1753) or isInArray({1619, 1741}, itemEx.itemid)) then
			doCreateItem(2255, 1, toPosition)
		elseif(itemEx.itemid == 2602) then
			doCreateItem(2257, 1, toPosition)
		elseif(itemEx.itemid == 3805 or itemEx.itemid == 3806) then
			doCreateItem(2259, 1, toPosition)
		end

		doRemoveItem(itemEx.uid, 1)
	end

	doSendMagicEffect(toPosition, CONST_ME_POFF)
	return true
end

TOOLS = {}
TOOLS.ROPE = function(cid, item, fromPosition, itemEx, toPosition)
	if(toPosition.x == CONTAINER_POSITION) then
		doPlayerSendDefaultCancel(cid, RETURNVALUE_NOTPOSSIBLE)
		return true
	end

	toPosition.stackpos = STACKPOS_GROUND
	errors(false)
	local ground = getThingFromPos(toPosition)
	errors(true)
	if(isInArray(SPOTS, ground.itemid)) then
		doTeleportThing(cid, {x = toPosition.x, y = toPosition.y + 1, z = toPosition.z - 1}, false)
		return true
	elseif(isInArray(ROPABLE, itemEx.itemid)) then
		local canOnlyRopePlayers = getBooleanFromString(getConfigValue('canOnlyRopePlayers'))
		local hole = getThingFromPos({x = toPosition.x, y = toPosition.y, z = toPosition.z + 1, stackpos = STACKPOS_TOP_MOVEABLE_ITEM_OR_CREATURE})
		if(canOnlyRopePlayers) then
			if(isPlayer(hole.uid) and (not isPlayerGhost(hole.uid) or getPlayerGhostAccess(cid) >= getPlayerGhostAccess(hole.uid))) then
				doTeleportThing(hole.uid, {x = toPosition.x, y = toPosition.y + 1, z = toPosition.z}, false)
			else
				doPlayerSendDefaultCancel(cid, RETURNVALUE_NOTPOSSIBLE)
			end
		else
			if(hole.itemid > 0) then
				doTeleportThing(hole.uid, {x = toPosition.x, y = toPosition.y + 1, z = toPosition.z}, false)
			else
				doPlayerSendDefaultCancel(cid, RETURNVALUE_NOTPOSSIBLE)
			end
		end

		return true
	end

	return false
end

TOOLS.PICK = function(cid, item, fromPosition, itemEx, toPosition)
	errors(false)
	local ground = getThingFromPos({x = toPosition.x, y = toPosition.y, z = toPosition.z + 1, stackpos = STACKPOS_GROUND})
	errors(true)
	if(isInArray(SPOTS, ground.itemid) and isInArray({354, 355}, itemEx.itemid)) then
		doTransformItem(itemEx.uid, 392)
		doDecayItem(itemEx.uid)

		doSendMagicEffect(toPosition, CONST_ME_POFF)
		return true
	end

	return false
end

TOOLS.MACHETE = function(cid, item, fromPosition, itemEx, toPosition, destroy)
	if(isInArray(JUNGLE_GRASS, itemEx.itemid)) then
		doTransformItem(itemEx.uid, itemEx.itemid - 1)
		doDecayItem(itemEx.uid)
		return true
	end

	if(isInArray(SPIDER_WEB, itemEx.itemid)) then
		if math.random(3) == 1 then
			doTransformItem(itemEx.uid, (itemEx.itemid + 6))
			doDecayItem(itemEx.uid)
		end
		doSendMagicEffect(toPosition, CONST_ME_POFF)
		return true
	end

	if(isInArray(BAMBOO_FENCE, itemEx.itemid)) then
		if math.random(3) == 1 then
			if(itemEx.itemid == BAMBOO_FENCE[1]) then
				doTransformItem(itemEx.uid, (itemEx.itemid + 161))
			elseif(itemEx.itemid == BAMBOO_FENCE[2]) then
				doTransformItem(itemEx.uid, (itemEx.itemid + 159))
			end
			doDecayItem(itemEx.uid)
		end
		doSendMagicEffect(toPosition, CONST_ME_POFF)
		return true
	end

	if(isInArray(WILD_GROWTH, itemEx.itemid)) then
		doSendMagicEffect(toPosition, CONST_ME_POFF)
		doRemoveItem(itemEx.uid)
		return true
	end

	return destroy and destroyItem(cid, itemEx, toPosition) or false
end

TOOLS.SHOVEL = function(cid, item, fromPosition, itemEx, toPosition)
	if(isInArray(HOLES, itemEx.itemid)) then
		local newId = itemEx.itemid + 1
		if(itemEx.itemid == 8579) then
			newId = 8585
		end

		doTransformItem(itemEx.uid, newId)
		doDecayItem(itemEx.uid)
	elseif(isInArray(SAND, itemEx.itemid)) then
		local rand = math.random(1, 100)
		local ground = getThingFromPos({x = toPosition.x, y = toPosition.y, z = toPosition.z + 1, stackpos = STACKPOS_GROUND})
		if(isInArray(SPOTS, ground.itemid) and rand <= 20) then
			doTransformItem(itemEx.uid, 489)
			doDecayItem(itemEx.uid)
		elseif(rand >= 1 and rand <= 5) then
			doCreateItem(2159, 1, toPosition)
		elseif(rand > 85) then
			doCreateMonster("Scarab", toPosition, false)
		end

		doSendMagicEffect(toPosition, CONST_ME_POFF)
	end

	return true
end

TOOLS.SCYTHE = function(cid, item, fromPosition, itemEx, toPosition, destroy)
	if(itemEx.itemid == 2739) then
		doTransformItem(itemEx.uid, 2737)
		doCreateItem(2694, 1, toPosition)

		doDecayItem(itemEx.uid)
		return true
	end

	return destroy and destroyItem(cid, itemEx, toPosition) or false
end

TOOLS.KNIFE = function(cid, item, fromPosition, itemEx, toPosition)
	if(itemEx.itemid ~= PUMPKIN) then
		return false
	end

	doTransformItem(itemEx.uid, PUMPKIN_HEAD)
	return true
end