local holeId = {
    294, 369, 370, 383, 392, 408, 409, 410, 427, 428, 430, 462, 469, 470, 482, 484, 485, 489, 924, 3135, 3136, 7933, 7938, 8170, 8286, 8285, 8284, 8281, 8280, 8279, 8277, 8276, 8380, 8567, 8585, 8596, 8595, 8249, 8250, 8251, 8252, 8253, 8254, 8255, 8256, 8592, 8972, 9606, 9625, 13190, 14461, 19519, 21536, 26020
}

local Itemsgrinder = {
    [7759] = {item_id = 34642, effect = CONST_ME_BLUE_FIREWORKS}, -- Sapphire dust
    [18416] = {item_id = 23876, effect = CONST_ME_GREENSMOKE} -- Pinch of crystal dust
}

local holes = {
    468, 481, 483, 7932, 23712
}

local JUNGLE_GRASS = {
    2782, 3985, 19433
}
local WILD_GROWTH = {
    1499, 11099, 2101, 1775, 1447, 1446, 20670
}

local fruits = {
    2673, 2674, 2675, 2676, 2677, 2678, 2679, 2680, 2681, 2682, 2684, 2685, 5097, 8839, 8840, 8841
}

local lava = {
	--
	-- Position(32808, 32336, 11),
}

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

local cutItems = {
    [3794] = 3959,
    [3795] = 3959,
    [3796] = 3958,
    [3797] = 3958,
    [3798] = 3958,
    [3799] = 3958,
    [1614] = 2251,
    [1615] = 2251,
    [1616] = 2251,
    [1619] = 2251,
    [1650] = 2253,
    [1651] = 2253,
    [1652] = 2253,
    [1653] = 2253,
    [1658] = 2252,
    [1659] = 2252,
    [1660] = 2252,
    [1661] = 2252,
    [1666] = 2252,
    [1667] = 2252,
    [1668] = 2252,
    [1669] = 2252,
    [1670] = 2252,
    [1671] = 2252,
    [1672] = 2252,
    [1673] = 2252,
    [1674] = 2253,
    [1676] = 2252,
    [1677] = 2253,
    [1714] = 2251,
    [1715] = 2251,
    [1716] = 2251,
    [1724] = 2252,
    [1725] = 2252,
    [1726] = 2252,
    [1727] = 2252,
    [1728] = 2254,
    [1729] = 2254,
    [1730] = 2254,
    [1731] = 2254,
    [1732] = 2254,
    [1733] = 2254,
    [1735] = 2254,
    [1775] = 2250,
    [2034] = 2252,
    [4996] = 2252,
    [2116] = 2254,
    [2116] = 2254,
    [2117] = 2254,
    [2118] = 2254,
    [2119] = 2254,
    [6123] = 2254,
    [2080] = 2254,
    [2081] = 2254,
    [2082] = 2254,
    [2083] = 2254,
    [2084] = 2254,
    [2085] = 2254,
    [2093] = 2250,
    [2094] = 2250,
    [2095] = 2250,
    [2098] = 2250,
    [2099] = 2250,
    [2101] = 2250,
    [2106] = 2250,
    [2105] = 2250,
    [2562] = 2257,
    [2581] = 2258,
    [2582] = 2258,
    [2582] = 2258,
    [2583] = 2258,
    [3805] = 6267,
    [3806] = 6267,
    [3807] = 2252,
    [3808] = 2252,
    [3809] = 2252,
    [3810] = 2252,
    [3811] = 2255,
    [3812] = 6267,
    [3813] = 2252,
    [3814] = 2252,
    [3815] = 2252,
    [3816] = 2252,
    [3817] = 2252,
    [3818] = 2252,
    [3819] = 2252,
    [3820] = 2252,
    [3821] = 2255,
    [3832] = 2255,
    [3833] = 2255,
    [3834] = 2255,
    [3835] = 2255,
    [6356] = 2257,
    [6357] = 2257,
    [6358] = 2257,
    [6359] = 2257,
    [6360] = 2257,
    [6361] = 2257,
    [6363] = 2257,
    [6368] = 2250,
    [6369] = 2250,
    [6370] = 2250,
    [6371] = 2250,
    [1738] = 2250,
    [1739] = 2251,
    [1740] = 2250,
    [1741] = 2255,
    [1747] = 2250,
    [1748] = 2250,
    [1749] = 1750,
    [1750] = 2254,
    [1751] = 2254,
    [1752] = 2254,
    [1753] = 2254,
    [1770] = 2251,
    [1774] = 2250,
    [6085] = 2254,
    [7481] = 2251,
    [7482] = 2251,
    [7483] = 2251,
    [7484] = 2250,
    [7706] = 2251,
    [7707] = 2251,
    [1738] = 2250,
    [1739] = 2251,
    [6109] = 2254,
    [6110] = 2254,
    [6111] = 2254,
    [6112] = 2254,
    [7538] = 7544,
    [7539] = 7545,
    [7585] = 7586,
    [29087] = 0,
    [29088] = 0
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
        if target.itemid ~= 29087 and target.itemid ~= 29088 then
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
            local fluid = Game.createItem(2016, target:getFluidType(), toPosition)
            if fluid ~= nil then
                fluid:decay()
            end
        end

        target:remove(1)

        local itemDestroy = Game.createItem(destroyId, 1, toPosition)
        if itemDestroy ~= nil then
            itemDestroy:decay()
        end
    end

    toPosition:sendMagicEffect(CONST_ME_POFF)
    return true
end

function onUseRope(player, item, fromPosition, target, toPosition, isHotkey)
	local tile = Tile(toPosition)
	if not tile then
		return false
	end

	local ground = tile:getGround()

	if ground and table.contains(ropeSpots, ground:getId()) or tile:getItemById(14435) then
		tile = Tile(toPosition:moveUpstairs())
		if not tile then
			return false
		end

		if tile:hasFlag(TILESTATE_PROTECTIONZONE) and player:isPzLocked() then
			return true
		end

		player:teleportTo(toPosition, false)
		return true
	end

	if table.contains(holeId, target.itemid) then
		toPosition.z = toPosition.z + 1
		tile = Tile(toPosition)
		if not tile then
			return false
		end

		local thing = tile:getTopVisibleThing()
		if not thing then
			return true
		end

		if thing:isPlayer() then
			if Tile(toPosition:moveUpstairs()):queryAdd(thing) ~= RETURNVALUE_NOERROR then
				return false
			end

			return thing:teleportTo(toPosition, false)
		elseif thing:isItem() and thing:getType():isMovable() then
			return thing:moveTo(toPosition:moveUpstairs())
		end

		return true
	end

	return false
end

function onUseShovel(player, item, fromPosition, target, toPosition, isHotkey)
    if table.contains(holes, target.itemid) then
        target:transform(target.itemid + 1)
        target:decay()
    elseif table.contains({231, 9059}, target.itemid) then
        local rand = math.random(100)
        if target.actionid == 100 and rand <= 20 then
            target:transform(489)
            target:decay()
        elseif rand == 1 then
            Game.createItem(2159, 1, toPosition)
        elseif rand > 95 then
            Game.createMonster("Scarab", toPosition)
        end
        toPosition:sendMagicEffect(CONST_ME_POFF)
    elseif target.itemid == 22674 then
        if not player:removeItem(5091, 1) then
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
    if table.contains({354, 355}, target.itemid) and target.actionid == 101 then
        target:transform(392)
        target:decay()
        toPosition:sendMagicEffect(CONST_ME_POFF)
    elseif target.itemid == 11227 then
        -- shiny stone refining
        local chance = math.random(1, 100)
        if chance == 1 then
            player:addItem(2160, 1) -- 1% chance of getting crystal coin
        elseif chance <= 6 then
            player:addItem(2148, 1) -- 5% chance of getting gold coin
        elseif chance <= 51 then
            player:addItem(2152, 1) -- 45% chance of getting platinum coin
        else
            player:addItem(2145, 1) -- 49% chance of getting small diamond
        end
        target:getPosition():sendMagicEffect(CONST_ME_BLOCKHIT)
        target:remove(1)
    elseif target.itemid == 11227 then
        target:remove(1)
        toPosition:sendMagicEffect(CONST_ME_POFF)
        player:addItem(2152, 10)
    elseif target.itemid == 7200 then
        target:transform(7236)
        target:decay()
        toPosition:sendMagicEffect(CONST_ME_HITAREA)
    elseif target.itemid == 468 then
        target:transform(469)
        target:decay()
        toPosition:sendMagicEffect(CONST_ME_HITAREA)
    elseif target.itemid == 6299 and target.actionid > 0 then
        target:transform(482)
        target:decay()
        toPosition:sendMagicEffect(CONST_ME_HITAREA)
    elseif target.itemid == 23712 then
        target:transform(23713)
        target:decay()
        toPosition:sendMagicEffect(CONST_ME_HITAREA)
    elseif target.itemid == 481 then
        target:transform(482)
        target:decay()
        toPosition:sendMagicEffect(CONST_ME_HITAREA)
    elseif target.itemid == 483 then
        target:transform(484)
        target:decay()
        toPosition:sendMagicEffect(CONST_ME_HITAREA)
    elseif target.itemid == 7932 then
        target:transform(7933)
        target:decay()
        toPosition:sendMagicEffect(CONST_ME_HITAREA)
    else
        return false
    end
    if (target ~= nil) and target:isItem() and (target:getId() == 22469) then
        --Lower Roshamuul
        if math.random(100) > 50 then
            player:sendTextMessage(MESSAGE_EVENT_ADVANCE, "Crushing the stone produces some fine gravel.")
            target:transform(22467)
            target:decay()
        else
            Game.createMonster("Frazzlemaw", toPosition)
            player:sendTextMessage(
            MESSAGE_EVENT_ADVANCE,
            "Crushing the stone yields nothing but slightly finer, yet still unusable rubber."
            )
            target:transform(22468)
            target:decay()
        end
        return true
    end
    return true
end

function onUseMachete(player, item, fromPosition, target, toPosition, isHotkey)
    if table.contains(JUNGLE_GRASS, target.itemid) then
        target:transform(target.itemid == 19433 and 19431 or target.itemid - 1)
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
    if not table.contains({2416, 10515}, item.itemid) then
        return false
    end
    return true
end

function onUseSpoon(player, item, fromPosition, target, toPosition, isHotkey)
    return true
end

function onUseScythe(player, item, fromPosition, target, toPosition, isHotkey)
    if not table.contains({2550, 10513}, item.itemid) then
        return false
    end

    if target.itemid == 5465 then
        target:transform(5464)
        target:decay()
        Game.createItem(5467, 1, toPosition)
    elseif target.itemid == 2739 then
        target:transform(2737)
        target:decay()
        Game.createItem(2694, 1, toPosition)
    else
        return false
    end
    return onDestroyItem(player, item, fromPosition, target, toPosition, isHotkey)
end

function onUseKitchenKnife(player, item, fromPosition, target, toPosition, isHotkey)
    if not table.contains({2566, 10511, 10515}, item.itemid) then
        return false
    end
    return true
end

function onGrindItem(player, item, fromPosition, target, toPosition)
	if not(target.itemid == 23942) then
		return false
	end
	for index, value in pairs(Itemsgrinder) do
		if item.itemid == index then 
			local topParent = item:getTopParent()
			if topParent.isItem and (not topParent:isItem() or topParent.itemid ~= 460) then
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