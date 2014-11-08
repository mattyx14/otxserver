SPOTS = { 384, 418, 8278, 8592 }
ROPABLE = { 294, 369, 370, 383, 392, 408, 409, 427, 428, 430, 462, 469, 470, 482, 484, 485, 489, 924, 3135, 3136, 7933, 7938, 8170, 8286, 8285,
	8284, 8281, 8280, 8279, 8277, 8276, 8323, 8380, 8567, 8585, 8596, 8595, 8249, 8250, 8251, 8252, 8253, 8254, 8255, 8256, 8972, 9606, 9625 }

HOLES = { 468, 481, 483, 7932, 8579 }
SAND = { 231, 9059 }

JUNGLE_GRASS = { 2782, 3985 }
SPIDER_WEB = { 7538, 7539 }
BAMBOO_FENCE = { 3798, 3799 }
WILD_GROWTH = { 1499, 11099 }

PUMPKIN = 2683
PUMPKIN_HEAD = 2096

POOL = 2016

SPECIAL_FOODS = {
	[9992] = "Gulp.", [9993] = "Chomp.", [9994] = "Chomp.", [9995] = "Chomp.", [9997] = "Yum.",
	[9998] = "Munch.", [9999] = "Chomp.", [10000] = "Mmmm.", [10001] = "Smack.", [12540] = "Yum.", 
	[12542] = "Gulp.", [12543] = "?", [12544] = "Slurp!"
}

DOORS = {
	[1209] = 1211, [1212] = 1214, [1231] = 1233, [1234] = 1236, [1249] = 1251, [1252] = 1254, [3535] = 3537, [3544] = 3546, [4913] = 4915, [4916] = 4918,
	[5098] = 5100, [5107] = 5109, [5116] = 5118, [5125] = 5127, [5134] = 5136, [5137] = 5139, [5140] = 5142, [5143] = 5145, [5278] = 5280, [5281] = 5283,
	[5732] = 5734, [5735] = 5737, [6192] = 6194, [6195] = 6197, [6249] = 6251, [6252] = 6254, [6891] = 6893, [6900] = 6902, [7033] = 7035, [7042] = 7044,
	[8541] = 8543, [8544] = 8546, [9165] = 9167, [9168] = 9170, [9267] = 9269, [9270] = 9272, [10268] = 10270, [10271] = 10273, [10468] = 10470,
	[10477] = 10479, [10775] = 10777, [10784] = 10786, [12092] = 12094, [12099] = 12101, [12188] = 12190, [12197] = 12199
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

	if(itemEx.itemid == 7200) then
		doTransformItem(itemEx.uid, 7236)
		doSendMagicEffect(toPosition, CONST_ME_BLOCKHIT)
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