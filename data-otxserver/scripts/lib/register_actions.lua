local holeId = {
	294, 369, 370, 385, 394, 411, 412, 413, 432, 433, 435, 8709, 594, 595, 615, 609, 610, 615, 1156, 482, 483, 868, 874, 4824, 7768, 433, 432, 413, 7767, 411, 370, 369, 7737, 7755, 7768, 7767, 7515, 7516, 7517, 7518, 7519, 7520, 7521, 7522, 7762, 8144, 8690, 8709, 12203, 12961, 17239, 19220, 23364
}

local Itemsgrinder = {
	[675] = {item_id = 30004, effect = CONST_ME_BLUE_FIREWORKS}, -- Sapphire dust
	[16122] = {item_id = 21507, effect = CONST_ME_GREENSMOKE} -- Pinch of crystal dust
	}

local holes = {
	593, 606, 608, 867, 21341
}

local JUNGLE_GRASS = {
	3696, 3702, 17153
}
local WILD_GROWTH = {
	2130, 2130, 2982, 2524, 2030, 2029, 10182
}

local fruits = {
	3584, 3585, 3586, 3587, 3588, 3589, 3590, 3591, 3592, 3593, 3595, 3596, 5096, 8011, 8012, 8013
}

local lava = {
	Position(32808, 32336, 11),
	Position(32809, 32336, 11),
	Position(32810, 32336, 11),
	Position(32808, 32334, 11),
	Position(32807, 32334, 11),
	Position(32807, 32335, 11),
	Position(32807, 32336, 11),
	Position(32807, 32337, 11),
	Position(32806, 32337, 11),
	Position(32805, 32337, 11),
	Position(32805, 32338, 11),
	Position(32805, 32339, 11),
	Position(32806, 32339, 11),
	Position(32806, 32338, 11),
	Position(32807, 32338, 11),
	Position(32808, 32338, 11),
	Position(32808, 32337, 11),
	Position(32809, 32337, 11),
	Position(32810, 32337, 11),
	Position(32811, 32337, 11),
	Position(32811, 32338, 11),
	Position(32806, 32338, 11),
	Position(32810, 32338, 11),
	Position(32810, 32339, 11),
	Position(32809, 32339, 11),
	Position(32809, 32338, 11),
	Position(32811, 32336, 11),
	Position(32811, 32335, 11),
	Position(32810, 32335, 11),
	Position(32809, 32335, 11),
	Position(32808, 32335, 11),
	Position(32809, 32334, 11),
	Position(32809, 32333, 11),
	Position(32810, 32333, 11),
	Position(32811, 32333, 11),
	Position(32806, 32338, 11),
	Position(32810, 32334, 11),
	Position(32811, 32334, 11),
	Position(32812, 32334, 11),
	Position(32813, 32334, 11),
	Position(32814, 32334, 11),
	Position(32812, 32333, 11),
	Position(32810, 32334, 11),
	Position(32812, 32335, 11),
	Position(32813, 32335, 11),
	Position(32814, 32335, 11),
	Position(32814, 32333, 11),
	Position(32813, 32333, 11)
}

local function revertItem(position, itemId, transformId)
	local item = Tile(position):getItemById(itemId)
	if item then
		item:transform(transformId)
	end
end

local function removeRemains(toPosition)
	local item = Tile(toPosition):getItemById(3133)
	if item then
		item:remove()
	end
end

local function revertCask(position)
	local caskItem = Tile(position):getItemById(3134)
	if caskItem then
		caskItem:transform(4848)
		position:sendMagicEffect(CONST_ME_MAGIC_GREEN)
	end
end

local cutItems = {
	[2291] = 3146,
	[2292] = 3146,
	[2293] = 3145,
	[2294] = 3145,
	[2295] = 3145,
	[2296] = 3145,
	[2314] = 3136,
	[2315] = 3136,
	[2316] = 3136,
	[2319] = 3136,
	[2358] = 3138,
	[2359] = 3138,
	[2360] = 3138,
	[2361] = 3138,
	[2366] = 3137,
	[2367] = 3137,
	[2368] = 3137,
	[2369] = 3137,
	[2374] = 3137,
	[2375] = 3137,
	[2376] = 3137,
	[2377] = 3137,
	[2378] = 3137,
	[2379] = 3137,
	[2380] = 3137,
	[2381] = 3137,
	[2382] = 3138,
	[2384] = 3137,
	[2385] = 3138,
	[2431] = 3136,
	[2432] = 3136,
	[2433] = 3136,
	[2441] = 3137,
	[2442] = 3137,
	[2443] = 3137,
	[2444] = 3137,
	[2445] = 3139,
	[2446] = 3139,
	[2447] = 3139,
	[2448] = 3139,
	[2449] = 3139,
	[2450] = 3139,
	[2452] = 3139,
	[2524] = 3135,
	[2904] = 3137,
	[4995] = 4996,
	[2997] = 3139,
	[2998] = 3139,
	[2999] = 3139,
	[3000] = 3139,
	[6123] = 3139,
	[2959] = 3139,
	[2960] = 3139,
	[2961] = 3139,
	[2962] = 3139,
	[2963] = 3139,
	[2964] = 3139,
	[2974] = 3135,
	[2975] = 3135,
	[2976] = 3135,
	[2979] = 3135,
	[2980] = 3135,
	[2982] = 3135,
	[2987] = 3135,
	[2986] = 3135,
	[3465] = 3142,
	[3484] = 3143,
	[3485] = 3143,
	[3486] = 3143,
	[2346] = 6266,
	[2347] = 6266,
	[2348] = 3137,
	[2349] = 3137,
	[2350] = 3137,
	[2351] = 3137,
	[2352] = 3140,
	[2353] = 6266,
	[2418] = 3137,
	[2419] = 3137,
	[2420] = 3137,
	[2421] = 3137,
	[2422] = 3137,
	[2423] = 3137,
	[2424] = 3137,
	[2425] = 3137,
	[2426] = 3140,
	[2465] = 3140,
	[2466] = 3140,
	[2467] = 3140,
	[2468] = 3140,
	[6355] = 3142,
	[6356] = 3142,
	[6357] = 3142,
	[6358] = 3142,
	[6359] = 3142,
	[6360] = 3142,
	[6362] = 3142,
	[6367] = 3135,
	[6368] = 3135,
	[6369] = 3135,
	[6370] = 3135,
	[2469] = 3135,
	[2471] = 3136,
	[2472] = 3135,
	[2473] = 3140,
	[2480] = 3135,
	[2481] = 3135,
	[2482] = 2483,
	[2483] = 3139,
	[2484] = 3139,
	[2485] = 3139,
	[2486] = 3139,
	[2519] = 3136,
	[2523] = 3135,
	[6085] = 3139,
	[116] = 3136,
	[117] = 3136,
	[118] = 3136,
	[119] = 3135,
	[404] = 3136,
	[405] = 3136,
	[6109] = 3139,
	[6110] = 3139,
	[6111] = 3139,
	[6112] = 3139,
	[182] = 188,
	[183] = 189,
	[233] = 234,
	[25798] = 0,
	[25800] = 0
}

function onDestroyItem(player, item, fromPosition, target, toPosition, isHotkey)
	if not target or target == nil or type(target) ~= "userdata" or not target:isItem() then
		return false
	end

	if target:hasAttribute(ITEM_ATTRIBUTE_UNIQUEID) or target:hasAttribute(ITEM_ATTRIBUTE_ACTIONID) then
		return false
	end

	if toPosition.x == CONTAINER_POSITION then
		player:sendCancelMessage(Game.getReturnMessage(RETURNVALUE_NOTPOSSIBLE))
		return true
	end

	local destroyId = cutItems[target.itemid] or ItemType(target.itemid):getDestroyId()
	if destroyId == 0 then
		if target.itemid ~= 25798 and target.itemid ~= 25800 then
			return false
		end
	end

	local watt = ItemType(item.itemid):getAttack()
	if math.random(1, 80) <= (watt and watt > 10 and watt or 10) then
		-- Move items outside the container
		if target:isContainer() then
			for i = target:getSize() - 1, 0, -1 do
				local containerItem = target:getItem(i)
				if containerItem then
					containerItem:moveTo(toPosition)
				end
			end
		end

		-- Being better than cipsoft
		if target:getFluidType() ~= 0 then
			local fluid = Game.createItem(2886, target:getFluidType(), toPosition)
			if fluid ~= nil then
				fluid:decay()
			end
		end

		target:remove(1)

		local itemDestroy = Game.createItem(destroyId, 1, toPosition)
		if itemDestroy ~= nil then
			itemDestroy:decay()
		end

		-- Energy barrier na threatned dreams quest (feyrist)
		if target.itemid == 25798 or target.itemid == 25800 then
			addEvent(Game.createItem, math.random(13000, 17000), target.itemid, 1, toPosition)
		end
	end

	toPosition:sendMagicEffect(CONST_ME_POFF)
	return true
end

function onUseRope(player, item, fromPosition, target, toPosition, isHotkey)
	if toPosition.x == CONTAINER_POSITION then
		return false
	end

	local tile = Tile(toPosition)
if table.contains(holeId, target.itemid) then
		toPosition.z = toPosition.z + 1
		tile = Tile(toPosition)
		if tile then
			local thing = tile:getTopVisibleThing()
			if thing:isItem() and thing:getType():isMovable() then
				return thing:moveTo(toPosition:moveUpstairs())
			elseif thing:isCreature() and thing:isPlayer() then
				return thing:teleportTo(toPosition:moveUpstairs())
			end
		end

		player:sendCancelMessage(RETURNVALUE_NOTPOSSIBLE)
	else
		return false
	end
	return true
end

function onUseShovel(player, item, fromPosition, target, toPosition, isHotkey)
	if target.itemid == 5730 then
		if not player:removeItem(5090, 1) then
			return false
		end

		target:transform(5731)
		target:decay()
		toPosition:sendMagicEffect(CONST_ME_POFF)
	else
		return false
	end
	return true
end

function onUsePick(player, item, fromPosition, target, toPosition, isHotkey)
	if target.itemid == 372 then
		target:transform(394)
		target:decay()
	else
		return false
	end
	return true
end

function onUseMachete(player, item, fromPosition, target, toPosition, isHotkey)
	if table.contains(JUNGLE_GRASS, target.itemid) then
		target:transform(target.itemid == 17153 and 17151 or target.itemid - 1)
		target:decay()
		return true
	end

	if table.contains(WILD_GROWTH, target.itemid) then
		toPosition:sendMagicEffect(CONST_ME_POFF)
		target:remove()
		return true
	end

	return onDestroyItem(player, item, fromPosition, target, toPosition, isHotkey)
end

function onUseCrowbar(player, item, fromPosition, target, toPosition, isHotkey)
	if not table.contains({3304, 9598}, item.itemid) then
		return false
	end
	return true
end

function onUseSpoon(player, item, fromPosition, target, toPosition, isHotkey)
	return true
end

function onUseScythe(player, item, fromPosition, target, toPosition, isHotkey)
	if not table.contains({3453, 9596}, item.itemid) then
		return false
	end

	if target.itemid == 5464 then
		target:transform(5463)
		target:decay()
		Game.createItem(5466, 1, toPosition)
	elseif target.itemid == 3653 then
		target:transform(3651)
		target:decay()
		Game.createItem(3605, 1, toPosition)
	else
		return false
	end
	return onDestroyItem(player, item, fromPosition, target, toPosition, isHotkey)
end

function onUseKitchenKnife(player, item, fromPosition, target, toPosition, isHotkey)
	if not table.contains({3469, 9594, 9598}, item.itemid) then
		return false
	end
	if table.contains(fruits, target.itemid) and player:removeItem(6277, 1) then
		target:remove(1)
		player:addItem(6278, 1)
		player:getPosition():sendMagicEffect(CONST_ME_MAGIC_GREEN)
	else
		return false
	end
	return true
end

function onGrindItem(player, item, fromPosition, target, toPosition)
	if not(target.itemid == 21573) then
		return false
	end
	for index, value in pairs(Itemsgrinder) do
		if item.itemid == index then
			local topParent = item:getTopParent()
			if topParent.isItem and (not topParent:isItem() or topParent.itemid ~= 470) then
				local parent = item:getParent()
				if not parent:isTile() and (parent:addItem(value.item_id, 1) or topParent:addItem(value.item_id, 1)) then
					item:remove(1)
					player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You grind a " .. ItemType(index):getName() .. " into fine, " .. ItemType(value.item_id):getName() .. ".")
					doSendMagicEffect(target:getPosition(), value.effect)
					return true
				else
					Game.createItem(value.item_id, 1, item:getPosition())
				end
			else
				Game.createItem(value.item_id, 1, item:getPosition())
			end
			player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "You grind a " .. ItemType(index):getName() .. " into fine, " .. ItemType(value.item_id):getName() .. ".")
			item:remove(1)
			doSendMagicEffect(target:getPosition(), value.effect)
			return
		end
	end
end
