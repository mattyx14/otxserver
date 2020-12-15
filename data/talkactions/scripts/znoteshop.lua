-- Znote Shop v1.0 for Znote AAC on TFS 1.1
function onSay(player, words, param)
	local storage = 54073 -- Make sure to select non-used storage. This is used to prevent SQL load attacks.
	local cooldown = 15 -- in seconds.

	if player:getStorageValue(storage) <= os.time() then
		player:setStorageValue(storage, os.time() + cooldown)
		
		local type_desc = {
			"itemids",
			"pending premium (skip)",
			"pending gender change (skip)",
			"pending character name change (skip)",
			"Outfit and addons",
			"Mounts",
			"Instant house purchase"
		}
		print("Player: " .. player:getName() .. " triggered !shop talkaction.")
		-- Create the query
		local orderQuery = db.storeQuery("SELECT `id`, `type`, `itemid`, `count` FROM `znote_shop_orders` WHERE `account_id` = " .. player:getAccountId() .. ";")
		local served = false 

		-- Detect if we got any results
		if orderQuery ~= false then
			repeat
				-- Fetch order values
				local q_id = result.getNumber(orderQuery, "id")
				local q_type = result.getNumber(orderQuery, "type")
				local q_itemid = result.getNumber(orderQuery, "itemid")
				local q_count = result.getNumber(orderQuery, "count")

				print("Processing type "..q_type..": ".. type_desc[q_type])

				-- ORDER TYPE 1 (Regular item shop products)
				if q_type == 1 then
					served = true
					-- Get wheight
					if player:getFreeCapacity() >= ItemType(q_itemid):getWeight(q_count) then
						db.query("DELETE FROM `znote_shop_orders` WHERE `id` = " .. q_id .. ";")
						player:addItem(q_itemid, q_count)
						player:sendTextMessage(MESSAGE_INFO_DESCR, "Congratulations! You have received " .. q_count .. " x " .. ItemType(q_itemid):getName() .. "!")
					else
						player:sendTextMessage(MESSAGE_STATUS_WARNING, "Need more CAP!")
					end
				end

				-- ORDER TYPE 5 (Outfit and addon)
				if q_type == 5 then
					served = true

					local itemid = q_itemid
					local outfits = {}

					if itemid > 1000 then
						local first = math.floor(itemid/1000)
						table.insert(outfits, first)
						itemid = itemid - (first * 1000)
					end
					table.insert(outfits, itemid)

					for _, outfitId in pairs(outfits) do
						-- Make sure player don't already have this outfit and addon
						if not player:hasOutfit(outfitId, q_count) then
							db.query("DELETE FROM `znote_shop_orders` WHERE `id` = " .. q_id .. ";")
							player:addOutfit(outfitId)
							player:addOutfitAddon(outfitId, q_count)
							player:sendTextMessage(MESSAGE_INFO_DESCR, "Congratulations! You have received a new outfit!")
						else
							player:sendTextMessage(MESSAGE_STATUS_WARNING, "You already have this outfit and addon!")
						end
					end
				end

				-- ORDER TYPE 6 (Mounts)
				if q_type == 6 then
					served = true
					-- Make sure player don't already have this outfit and addon
					if not player:hasMount(q_itemid) then
						db.query("DELETE FROM `znote_shop_orders` WHERE `id` = " .. q_id .. ";")
						player:addMount(q_itemid)
						player:sendTextMessage(MESSAGE_INFO_DESCR, "Congratulations! You have received a new mount!")
					else
						player:sendTextMessage(MESSAGE_STATUS_WARNING, "You already have this mount!")
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
				        end
				    end
				end
				

				-- Add custom order types here
				-- Type 1 is for itemids (Already coded here)
				-- Type 2 is for premium (Coded on web)
				-- Type 3 is for gender change (Coded on web)
				-- Type 4 is for character name change (Coded on web)
				-- Type 5 is for character outfit and addon (Already coded here)
				-- Type 6 is for mounts (Already coded here)
				-- So use type 7+ for custom stuff, like etc packages.
				-- if q_type == 7 then
				-- end
			until not result.next(orderQuery)
			result.free(orderQuery)
			if not served then
				player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, "You have no orders to process in-game.")
			end
		else
			player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, "You have no orders.")
		end
	else
		player:sendTextMessage(MESSAGE_STATUS_CONSOLE_BLUE, "Can only be executed once every " .. cooldown .. " seconds. Remaining cooldown: " .. player:getStorageValue(storage) - os.time())
	end
	return false
end
