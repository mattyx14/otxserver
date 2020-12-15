function onThink(interval, lastExecution)
    local orderQuery = db.storeQuery([[
        SELECT 
            MIN(`po`.`player_id`) AS `player_id`,
            `shop`.`id`, 
            `shop`.`type`, 
            `shop`.`itemid`, 
            `shop`.`count`
        FROM `players_online` AS `po`
        INNER JOIN `players` AS `p`
            ON `po`.`player_id` = `p`.`id`
        INNER JOIN `znote_shop_orders` AS `shop`
            ON `p`.`account_id` = `shop`.`account_id`
        WHERE `shop`.`type` IN(1,5,6,7)
        GROUP BY `shop`.`id`
    ]])
    -- Detect if we got any results
    if orderQuery ~= false then
        local type_desc = {
            "itemids",
            "pending premium (skip)",
            "pending gender change (skip)",
            "pending character name change (skip)",
            "Outfit and addons",
            "Mounts",
            "Instant house purchase"
        }
        repeat 
            local player_id = result.getDataInt(orderQuery, 'player_id')
            local orderId = result.getDataInt(orderQuery, 'id')
            local orderType = result.getDataInt(orderQuery, 'type')
            local orderItemId = result.getDataInt(orderQuery, 'itemid')
            local orderCount = result.getDataInt(orderQuery, 'count')
            local served = false

            local player = Player(player_id)
            if player ~= nil then
                print("Processing shop order for: [".. player:getName() .."] type "..orderType..": ".. type_desc[orderType])
                local tile = Tile(player:getPosition())
                if tile ~= nil and tile:hasFlag(TILESTATE_PROTECTIONZONE) then
                    -- ORDER TYPE 1 (Regular item shop products)
                    if orderType == 1 then
                        served = true
                        local itemType = ItemType(orderItemId)
                        -- Get wheight
                        if player:getFreeCapacity() >= itemType:getWeight(orderCount) then
                            local backpack = player:getSlotItem(CONST_SLOT_BACKPACK)
                            -- variable = (condition) and (return if true) or (return if false)
                            local needslots = itemType:isStackable() and math.floor(orderCount / 100) + 1 or orderCount
                            if backpack ~= nil and backpack:getEmptySlots(false) >= needslots then
                                db.query("DELETE FROM `znote_shop_orders` WHERE `id` = " .. orderId .. ";")
                                player:addItem(orderItemId, orderCount)
                                player:sendTextMessage(MESSAGE_INFO_DESCR, "Congratulations! You have received " .. orderCount .. "x " .. ItemType(orderItemId):getName() .. "!")
                                print("Process complete. [".. player:getName() .."] has recieved " .. orderCount .. "x " .. ItemType(orderItemId):getName() .. ".")
                            else -- not enough slots 
                                player:sendTextMessage(MESSAGE_STATUS_WARNING, "Your main backpack is full. You need to free up "..needslots.." available slots to get " .. orderCount .. " " .. ItemType(orderItemId):getName() .. "!")
                                print("Process canceled. [".. player:getName() .."] need more space in his backpack to get " .. orderCount .. "x " .. ItemType(orderItemId):getName() .. ".")
                            end
                        else -- not enough cap 
                            player:sendTextMessage(MESSAGE_STATUS_WARNING, "You need more CAP to carry this order!")
                            print("Process canceled. [".. player:getName() .."] need more cap to carry " .. orderCount .. "x " .. ItemType(orderItemId):getName() .. ".")
                        end
                    end

                    -- ORDER TYPE 5 (Outfit and addon)
                    if orderType == 5 then
                        served = true

                        local itemid = orderItemId
                        local outfits = {}

                        if itemid > 1000 then
                            local first = math.floor(itemid/1000)
                            table.insert(outfits, first)
                            itemid = itemid - (first * 1000)
                        end
                        table.insert(outfits, itemid)

                        for _, outfitId in pairs(outfits) do
                            -- Make sure player don't already have this outfit and addon
                            if not player:hasOutfit(outfitId, orderCount) then
                                db.query("DELETE FROM `znote_shop_orders` WHERE `id` = " .. orderId .. ";")
                                player:addOutfit(outfitId)
                                player:addOutfitAddon(outfitId, orderCount)
                                player:sendTextMessage(MESSAGE_INFO_DESCR, "Congratulations! You have received a new outfit!")
                                print("Process complete. [".. player:getName() .."] has recieved outfit: ["..outfitId.."] with addon: ["..orderCount.."]")
                            else -- Already has outfit 
                                player:sendTextMessage(MESSAGE_STATUS_WARNING, "You already have this outfit and addon!")
                                print("Process canceled. [".. player:getName() .."] already have outfit: ["..outfitId.."] with addon: ["..orderCount.."].")
                            end
                        end
                    end

                    -- ORDER TYPE 6 (Mounts)
                    if orderType == 6 then
                        served = true
                        -- Make sure player don't already have this outfit and addon
                        if not player:hasMount(orderItemId) then
                            db.query("DELETE FROM `znote_shop_orders` WHERE `id` = " .. orderId .. ";")
                            player:addMount(orderItemId)
                            player:sendTextMessage(MESSAGE_INFO_DESCR, "Congratulations! You have received a new mount!")
                            print("Process complete. [".. player:getName() .."] has recieved mount: ["..orderItemId.."]")
                        else -- Already has mount 
                            player:sendTextMessage(MESSAGE_STATUS_WARNING, "You already have this mount!")
                            print("Process canceled. [".. player:getName() .."] already have mount: ["..orderItemId.."].")
                        end
                    end

                    -- ORDER TYPE 7 (Direct house purchase)
                    if orderType == 7 then
                        served = true
                        local house = House(orderItemId)
                        -- Logged in player is not neccesarily the player that bough the house. So we need to load player from db.
                        local buyerQuery = db.storeQuery("SELECT `name` FROM `players` WHERE `id` = "..orderCount.." LIMIT 1")
                        if buyerQuery ~= false then
                            local buyerName = result.getDataString(buyerQuery, "name")
                            result.free(buyerQuery)
                            if house then
                                db.query("DELETE FROM `znote_shop_orders` WHERE `id` = " .. orderId .. ";")
                                house:setOwnerGuid(orderCount)
                                player:sendTextMessage(MESSAGE_INFO_DESCR, "You have successfully bought the house "..house:getName().." on "..buyerName..", be sure to have the money for the rent in the bank.")
                                print("Process complete. [".. buyerName .."] has recieved house: ["..house:getName().."]")
                            else
                                print("Process canceled. Failed to load house with ID: "..orderItemId)
                            end
                        else
                            print("Process canceled. Failed to load player with ID: "..orderCount)
                        end
                    end

                    if not served then -- If this order hasn't been processed yet (missing type handling?)
                        print("Znote shop: Type ["..orderType.."] not properly processed. Missing Lua code?")
                    end
                else -- Not in protection zone 
                    player:sendTextMessage(MESSAGE_INFO_DESCR, 'You have a pending shop order, please enter protection zone.')
                    print("Skipped one shop order. Reason: Player: [".. player:getName() .."] is not inside protection zone.")
                end
            else -- player not logged in 
                print("Skipped one shop order. Reason: Player with id [".. player_id .."] is not online.")
            end

        until not result.next(orderQuery)
        result.free(orderQuery)
    end
    return true
end
