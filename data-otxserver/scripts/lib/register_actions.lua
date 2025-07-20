local holeId = { 294, 369, 370, 385, 394, 411, 412, 413, 432, 433, 435, 482, 483, 594, 595, 609, 610, 615, 868, 874, 1156, 4824, 7515, 7516, 7517, 7518, 7519, 7520, 7521, 7522, 7737, 7755, 7762, 7767, 7768, 8144, 8690, 8709, 12203, 12961, 17239, 19220, 23364 }
local itemsGrinder = {
	[675] = { item_id = 30004, effect = CONST_ME_BLUE_FIREWORKS }, -- Sapphire dust
	[16122] = { item_id = 21507, effect = CONST_ME_GREENSMOKE }, -- Pinch of crystal dust
}
local holes = { 593, 606, 608, 867, 21341 }
local jungleGrass = { 3696, 3702, 17153 }
local wildGrowth = { 2130, 3635, 30224 }
local fruits = { 3584, 3585, 3586, 3587, 3588, 3589, 3590, 3591, 3592, 3593, 3595, 3596, 5096, 8011, 8012, 8013 }
local lava = { }

local secret_library = {
	crystals = {
		[1] = { storage = Storage.Quest.U11_80.TheSecretLibrary.MoTA.Crystal1, position = Position(33216, 32108, 9) },
		[2] = { storage = Storage.Quest.U11_80.TheSecretLibrary.MoTA.Crystal2, position = Position(33242, 32100, 9) },
		[3] = { storage = Storage.Quest.U11_80.TheSecretLibrary.MoTA.Crystal3, position = Position(33226, 32103, 9) },
		[4] = { storage = Storage.Quest.U11_80.TheSecretLibrary.MoTA.Crystal4, position = Position(33236, 32084, 9) },
		[5] = { storage = Storage.Quest.U11_80.TheSecretLibrary.MoTA.Crystal5, position = Position(33260, 32103, 9) },
		[6] = { storage = Storage.Quest.U11_80.TheSecretLibrary.MoTA.Crystal6, position = Position(33260, 32103, 9) },
		[7] = { storage = Storage.Quest.U11_80.TheSecretLibrary.MoTA.Crystal7, position = Position(33260, 32103, 9) },
		[8] = { storage = Storage.Quest.U11_80.TheSecretLibrary.MoTA.Crystal8, position = Position(33260, 32103, 9) },
	},
	timer = "tsl_crystaltimer",
	exhaustMessage = "Digging crystal is exhausting. You're still weary from your last prospect.",
	items = { 27867, 27868, 27869 },
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
	[25800] = 0,
}

function onDestroyItem(player, item, fromPosition, target, toPosition, isHotkey)
	if not target or type(target) ~= "userdata" or not target:isItem() then
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
	if table.contains(holes, target.itemid) then
		target:transform(target.itemid + 1)
		target:decay()
		toPosition.z = toPosition.z + 1
		if Tile(toPosition):hasFlag(TILESTATE_PROTECTIONZONE) and player:isPzLocked() then
			player:sendCancelMessage(RETURNVALUE_PLAYERISPZLOCKED)
			return true
		end
		player:teleportTo(toPosition, true)
		player:addAchievementProgress("The Undertaker", 500)
	elseif target.itemid == 5730 then
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
	local stonePos = Position(32648, 32134, 10)
	if toPosition == stonePos then
		local tile = Tile(stonePos)
		local stone = tile:getItemById(1772)
		if stone then
			stone:remove(1)
			toPosition:sendMagicEffect(CONST_ME_POFF)
			addEvent(function()
				Game.createItem(1772, 1, stonePos)
			end, 20000)
			return true
		end
	end

	if table.contains({ 354, 355 }, target.itemid) and target.actionid == 101 then
		target:transform(394)
		target:decay()
	elseif target.itemid == 10310 then
		-- shiny stone refining
		local chance = math.random(1, 100)
		if chance == 1 then
			player:addItem(3043, 1) -- 1% chance of getting crystal coin
		elseif chance <= 6 then
			player:addItem(3031, 1) -- 5% chance of getting gold coin
		elseif chance <= 51 then
			player:addItem(3035, 1) -- 45% chance of getting platinum coin
		else
			player:addItem(3028, 1) -- 49% chance of getting small diamond
		end
		target:getPosition():sendMagicEffect(CONST_ME_BLOCKHIT)
		target:remove(1)
		player:addAchievementProgress("Petrologist", 100)
	elseif target.itemid == 7200 then
		target:transform(7236)
		target:decay()
		toPosition:sendMagicEffect(CONST_ME_HITAREA)
	elseif target.itemid == 593 then
		target:transform(594)
		target:decay()
		toPosition:sendMagicEffect(CONST_ME_HITAREA)
	elseif target.itemid == 6298 and target.actionid > 0 then
		target:transform(615)
		target:decay()
		toPosition:sendMagicEffect(CONST_ME_HITAREA)
	elseif target.itemid == 21341 then
		target:transform(21342)
		target:decay()
		toPosition:sendMagicEffect(CONST_ME_HITAREA)
	elseif target.itemid == 606 then
		target:transform(615)
		target:decay()
		toPosition:sendMagicEffect(CONST_ME_HITAREA)
	elseif target.itemid == 608 then
		target:transform(609)
		target:decay()
		toPosition:sendMagicEffect(CONST_ME_HITAREA)
	elseif target.itemid == 867 then
		target:transform(868)
		target:decay()
		toPosition:sendMagicEffect(CONST_ME_HITAREA)
	elseif target.itemid == 22075 then
		-- Grimvale quest
		if player:getStorageValue(Storage.Quest.U10_80.GrimvaleQuest.SilverVein) < os.time() then
			local chance = math.random(1, 10)
			if chance >= 5 then
				player:sendTextMessage(
					MESSAGE_EVENT_ADVANCE,
					"Even after a thorough and frustrating \z
				search you could not find enough liquified silver in this vein to fill a flask."
				)
			elseif chance <= 4 then
				player:sendTextMessage(
					MESSAGE_EVENT_ADVANCE,
					"Carefully you gather some of the liquified \z
				silver from this vein in a small flask. You now feel strangely affected to the moon."
				)
				player:addItem(22058)
				target:transform(4464)
				addEvent(revertItem, 10 * 60 * 1000, toPosition, 4464, 22075)
			end
			player:setStorageValue(Storage.Quest.U10_80.GrimvaleQuest.SilverVein, os.time() + 2 * 60)
		else
			player:sendTextMessage(
				MESSAGE_EVENT_ADVANCE,
				"You are still exhausted from earlier attempts. \z
				Getting liquid silver out of the mountain needs concentration and a steady hand."
			)
		end
	elseif target.itemid == 372 then
		target:transform(394)
		target:decay()
	elseif target.itemid == 20135 then
		local chance = math.random(100)
		if chance > 50 then
			player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Crushing the stone produces some fine gravel.")
			target:transform(20133)
		else
			Game.createMonster("Frazzlemaw", toPosition)
			player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Crushing the stone yields nothing but slightly finer, yet still unusable rubber.")
			target:transform(20134)
		end
		target:decay()
	else
		return false
	end

	return true
end

function onUseMachete(player, item, fromPosition, target, toPosition, isHotkey)
	if table.contains(jungleGrass, target.itemid) then
		target:transform(target.itemid == 17153 and 17151 or target.itemid - 1)
		target:decay()
		return true
	end

	if table.contains(wildGrowth, target.itemid) then
		toPosition:sendMagicEffect(CONST_ME_POFF)
		target:remove()
		return true
	elseif target.itemid == 30623 then -- reed
		target:transform(30624)
		target:decay()
		Game.createItem(30975, 1, toPosition)
	end

	return onDestroyItem(player, item, fromPosition, target, toPosition, isHotkey)
end

function onUseCrowbar(player, item, fromPosition, target, toPosition, isHotkey)
	if not table.contains({ 3304, 9598 }, item.itemid) then
		return false
	end

	return true
end

function onUseSpoon(player, item, fromPosition, target, toPosition, isHotkey)
	if not target or not target.getId then
		return false
	end

	return true
end

function onUseSpikedSquelcher(player, item, fromPosition, target, toPosition, isHotkey)
	if not target or not target.getId then
		return false
	end

	return true
end

function onUseScythe(player, item, fromPosition, target, toPosition, isHotkey)
	if not table.contains({ 3453, 9596 }, item.itemid) then
		return false
	end
	return onDestroyItem(player, item, fromPosition, target, toPosition, isHotkey)
end

function onUseKitchenKnife(player, item, fromPosition, target, toPosition, isHotkey)
	if not table.contains({ 3469, 9594, 9598 }, item.itemid) then
		return false
	end
	return true
end

function onGrindItem(player, item, fromPosition, target, toPosition)
	if not (target.itemid == 21573) then
		return false
	end
	for index, value in pairs(itemsGrinder) do
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
