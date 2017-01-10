-- Please don't edit those information!
GameStore = {
	ModuleName = "GameStore",
	Developer = "OTX TEAM",
	Version = "0.3",
	LastUpdated = "24-09-2016 07:15PM"
}

--== Enums ==--
GameStore.OfferTypes = {
	OFFER_TYPE_NONE = 0, -- (this will disable offer)
	OFFER_TYPE_ITEM = 1,
	OFFER_TYPE_STACKABLE = 2,
	OFFER_TYPE_OUTFIT = 3,
	OFFER_TYPE_OUTFIT_ADDON = 4,
	OFFER_TYPE_MOUNT = 5,
	OFFER_TYPE_NAMECHANGE = 6,
	OFFER_TYPE_SEXCHANGE = 7,
	OFFER_TYPE_PROMOTION = 8
}

GameStore.ClientOfferTypes = {
	CLIENT_STORE_OFFER_OTHER = 0,
	CLIENT_STORE_OFFER_NAMECHANGE = 1
}

GameStore.HistoryTypes = {
	HISTORY_TYPE_NONE = 0,
	HISTORY_TYPE_GIFT = 1,
	HISTORY_TYPE_REFUND = 2
}

GameStore.States = {
	STATE_NONE = 0,
	STATE_NEW = 1,
	STATE_SALE = 2,
	STATE_TIMED = 3
}

GameStore.StoreErrors = {
	STORE_ERROR_PURCHASE = 0,
	STORE_ERROR_NETWORK = 1,
	STORE_ERROR_HISTORY = 2,
	STORE_ERROR_TRANSFER = 3,
	STORE_ERROR_INFORMATION = 4
}

GameStore.ServiceTypes = {
	SERVICE_STANDERD = 0,
	SERVICE_OUTFITS = 3,
	SERVICE_MOUNTS = 4
}

GameStore.SendingPackets = {
	S_CoinBalance = 0xDF, -- 223
	S_StoreError = 0xE0, -- 224
	S_RequestPurchaseData = 0xE1, -- 225
	S_CoinBalanceUpdating = 0xF2, -- 242
	S_OpenStore = 0xFB, -- 251
	S_StoreOffers = 0xFC, -- 252
	S_OpenTransactionHistory = 0xFD, -- 253
	S_CompletePurchase = 0xFE  -- 254
}
GameStore.RecivedPackets = {
	C_StoreEvent = 0xE9, -- 233
	C_TransferCoins = 0xEF, -- 239
	C_OpenStore = 0xFA, -- 250
	C_RequestStoreOffers = 0xFB, -- 251
	C_BuyStoreOffer = 0xFC, -- 252
	C_OpenTransactionHistory = 0xFD, -- 253
	C_RequestTransactionHistory = 0xFE, -- 254
}

GameStore.DefaultValues = {
	DEFAULT_VALUE_ENTRIES_PER_PAGE	= 16
}
GameStore.DefaultDescriptions = {
	OUTFIT = {"This outfit looks nice. Only high-class people are able to wear it!",
		"An outfit that was created to suit you. We are sure you'll like it.",
		"Legend says only smart people should wear it, otherwise you will burn!"},
	MOUNT = {"This is a fantastic mount that helps to become faster, try it!",
		"The first rider of this mount became the leader of his country! legends say that."},
	NAMECHANGE = {"Are you hunted? Tired of that? Get a new name, a new life!",
		"A new name to suit your needs!"},
	SEXCHANGE = {"Bored of your character's sex? Get a new sex for him now!!"}
}

--==Parsing==--
GameStore.isItsPacket = function(byte)
	for k, v in pairs(GameStore.RecivedPackets) do
		if v == byte then
			return true
		end
	end
	return false
end

function onRecvbyte(player, msg, byte)
	if byte == GameStore.RecivedPackets.C_StoreEvent then
		-- Not Used!
	elseif byte == GameStore.RecivedPackets.C_TransferCoins then
		parseTransferCoins(player, msg)
	elseif byte == GameStore.RecivedPackets.C_OpenStore then
		parseOpenStore(player, msg)
	elseif byte == GameStore.RecivedPackets.C_RequestStoreOffers then
		parseRequestStoreOffers(player, msg)
	elseif byte == GameStore.RecivedPackets.C_BuyStoreOffer then
		parseBuyStoreOffer(player, msg)
	elseif byte == GameStore.RecivedPackets.C_OpenTransactionHistory then
		parseOpenTransactionHistory(player, msg)
	elseif byte == GameStore.RecivedPackets.C_RequestTransactionHistory then
		parseRequestTransactionHistory(player, msg)
	end
	return true
end
function parseTransferCoins(player, msg)
	local reciver = msg:getString()
	local amount = msg:getU32()

	if reciver:lower() == player:getName():lower() then
		return addPlayerEvent(sendStoreError, 350, player, GameStore.StoreErrors.STORE_ERROR_TRANSFER, "You can't transfer coins to yourself.")
	end

	local resultId = db.storeQuery("SELECT `account_id` FROM `players` WHERE `name` = " .. db.escapeString(reciver:lower()) .. "")
	if not resultId then
		return addPlayerEvent(sendStoreError, 350, player, GameStore.StoreErrors.STORE_ERROR_TRANSFER, "We couldn't find that player.")
	end

	local accountId = result.getDataInt(resultId, "account_id")
	if accountId == player:getAccountId() then
		return addPlayerEvent(sendStoreError, 350, player, GameStore.StoreErrors.STORE_ERROR_TRANSFER, "You cannot transfer coin to a character in the same account.")
	end

	db.asyncQuery("UPDATE `accounts` SET `coins` = `coins` + " .. amount .. " WHERE `id` = " .. accountId)
	player:removeCoinsBalance(amount)
	addPlayerEvent(sendStorePurchaseSuccessful, 550, player, "You have transfered " .. amount .. " coins to " .. reciver .. " successfully")

	-- Adding history for both reciver/sender
	GameStore.insertHistory(accountId, GameStore.HistoryTypes.HISTORY_TYPE_NONE, player:getName() .. " transfered you this amount.", amount)
	GameStore.insertHistory(player:getAccountId(), GameStore.HistoryTypes.HISTORY_TYPE_NONE, "You transfered this amount to " .. reciver, -1 * amount) -- negative
end

function parseOpenStore(player, msg)
	openStore(player)

	local serviceType = msg:getByte()
	local category = GameStore.Categories and GameStore.Categories[1] or nil

	if serviceType == GameStore.ServiceTypes.SERVICE_OUTFITS then
		category = GameStore.getCategoryByName("outfits")
	elseif serviceType == GameStore.ServiceTypes.SERVICE_MOUNTS then
		category = GameStore.getCategoryByName("mounts")
	end

	if category then
		addPlayerEvent(sendShowStoreOffers, 350, player, category)
	end
end

function parseRequestStoreOffers(player, msg)
	local serviceType = GameStore.ServiceTypes.SERVICE_STANDERD
	if player:getClient().version >= 1092 then
		serviceType = msg:getByte()
	end
	local categoryName = msg:getString()

	local category = GameStore.getCategoryByName(categoryName)
	if category then
		addPlayerEvent(sendShowStoreOffers, 350, player, category)
	end
end

function parseBuyStoreOffer(player, msg)
	local offerId = msg:getU32()
	local productType = msg:getByte()

	local offer = GameStore.getOfferById(offerId)
	if offer then
		-- If we don't add type, or offer type is fake
		if not offer.type or offer.type == GameStore.OfferTypes.OFFER_TYPE_NONE then
			return addPlayerEvent(sendStoreError, 250, player, GameStore.StoreErrors.STORE_ERROR_NETWORK, "The offer is either fake or corrupt.")
		end

		-- If no thing id,
		if offer.type ~= GameStore.OfferTypes.OFFER_TYPE_NAMECHANGE and offer.type ~= GameStore.OfferTypes.OFFER_TYPE_SEXCHANGE and not offer.thingId then
			return addPlayerEvent(sendStoreError, 250, player, GameStore.StoreErrors.STORE_ERROR_NETWORK, "The offer is either fake or corrupt.")
		end

		-- We remove coins before doing everything, if it fails, we add coins back!
		if not player:canRemoveCoins(offer.price) then
			return addPlayerEvent(sendStoreError, 250, player, GameStore.StoreErrors.STORE_ERROR_NETWORK, "We couldn't remove coins from your account, try again later.")
		end

		-- count is used in type(item), so we need to show (i.e 10x crystal coins)
		local offerCountStr = offer.count and (offer.count .. "x ") or ""
		-- The message which we will send to player!
		local message = "You have purchased " .. offerCountStr .. offer.name .. " for " .. offer.price .. " coins."

		-- If offer is item.
		if offer.type == GameStore.OfferTypes.OFFER_TYPE_ITEM then
			local inbox = player:getSlotItem(CONST_SLOT_STORE_INBOX)
			if inbox and inbox:getEmptySlots() > offer.count then
				for t = 1,offer.count do
					inbox:addItem(offer.thingId, offer.count or 1)
				end
			else
				return addPlayerEvent(sendStoreError, 250, player, GameStore.StoreErrors.STORE_ERROR_NETWORK, "Please make sure you have free slots in your store inbox.")
			end
		-- If offer is Stackable.
		elseif offer.type == GameStore.OfferTypes.OFFER_TYPE_STACKABLE then
			local inbox = player:getSlotItem(CONST_SLOT_STORE_INBOX)
			if inbox and inbox:getEmptySlots() > 0 then
				local parcel = inbox:addItem(2596, 1)
				local packagename = ''.. offer.count..'x '.. offer.name ..' package.'
					if parcel then
						parcel:setAttribute(ITEM_ATTRIBUTE_NAME, packagename)
						for e = 1,offer.count do
							parcel:addItem(offer.thingId, 1)
					end
				end
			else
				return addPlayerEvent(sendStoreError, 250, player, GameStore.StoreErrors.STORE_ERROR_NETWORK, "Please make sure you have free slots in your store inbox.")
		end
		-- If offer is outfit/addon
		elseif offer.type == GameStore.OfferTypes.OFFER_TYPE_OUTFIT or offer.type == GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON then
			local outfitLookType
			if player:getSex() == PLAYERSEX_MALE then
				outfitLookType = offer.thingId.male
			else
				outfitLookType = offer.thingId.female
			end
			if not outfitLookType then
				return addPlayerEvent(sendStoreError, 250, player, GameStore.StoreErrors.STORE_ERROR_NETWORK, "This outfit seems not to suit your sex, we are sorry for that!")
			end

			player:addOutfitAddon(outfitLookType, offer.addon or 0)
		-- If offer is mount
		elseif offer.type == GameStore.OfferTypes.OFFER_TYPE_MOUNT then
			player:addMount(offer.thingId)
		-- If offer is name change
		elseif offer.type == GameStore.OfferTypes.OFFER_TYPE_NAMECHANGE then
			-- If player typed name yet!
			if productType == GameStore.ClientOfferTypes.CLIENT_STORE_OFFER_NAMECHANGE then
				local newName = msg:getString()

				local resultId = db.storeQuery("SELECT * FROM `players` WHERE `name` = " .. db.escapeString(newName) .. "")
				if resultId ~= false then
					return addPlayerEvent(sendStoreError, 650, player, GameStore.StoreErrors.STORE_ERROR_NETWORK, "This name is already used, please try again!")
				end

				local result = GameStore.canChangeToName(newName)
				if not result.ability then
					return addPlayerEvent(sendStoreError, 650, player, GameStore.StoreErrors.STORE_ERROR_NETWORK, result.reason)
				end

				newName = newName:lower():gsub("(%l)(%w*)", function(a, b) return string.upper(a) .. b end)
				db.asyncQuery("UPDATE `players` SET `name` = " .. db.escapeString(newName) .. " WHERE `id` = " .. player:getGuid())
				message =  "You have successfully changed you name, you must relog to see changes."
			-- If not, we ask him to do!
			else
				return addPlayerEvent(sendRequestPurchaseData, 250, player, offer.id, GameStore.ClientOfferTypes.CLIENT_STORE_OFFER_NAMECHANGE)
			end
		-- If offer is sex change
		elseif offer.type == GameStore.OfferTypes.OFFER_TYPE_SEXCHANGE then
			player:toggleSex()
		-- If offer is promotion
		elseif offer.type == GameStore.OfferTypes.OFFER_TYPE_PROMOTION then
			if not GameStore.addPromotionToPlayer(player, offer.thingId) then
				return false
			end
		-- You can add whatever offer types to suit your needs!
		else
			-- ToDo :: implement purchase function
			return addPlayerEvent(sendStoreError, 250, player, GameStore.StoreErrors.STORE_ERROR_NETWORK, "This offer is fake, please contact admin.")
		end
		-- Removing coins
		player:removeCoinsBalance(offer.price)
		-- We add this purchase to history!
		GameStore.insertHistory(player:getAccountId(), GameStore.HistoryTypes.HISTORY_TYPE_NONE, offerCountStr .. offer.name, offer.price * -1)
		-- Send to client that purchase is successful!
		return addPlayerEvent(sendStorePurchaseSuccessful, 650, player, message)
	end

	-- If we didn't found the offer or error happened
	addPlayerEvent(sendStoreError, 350, player, GameStore.StoreErrors.STORE_ERROR_INFORMATION, "We couldn't locate this offer, please try again later.")
end

-- Both functions use same formula!
function parseOpenTransactionHistory(player, msg)
	local page = 1
	GameStore.DefaultValues.DEFAULT_VALUE_ENTRIES_PER_PAGE = msg:getByte()
	sendStoreTransactionHistory(player, page, GameStore.DefaultValues.DEFAULT_VALUE_ENTRIES_PER_PAGE)
end

function parseRequestTransactionHistory(player, msg)
	local page = msg:getU32()
	sendStoreTransactionHistory(player, page, GameStore.DefaultValues.DEFAULT_VALUE_ENTRIES_PER_PAGE)
end

--==Sending==--
function openStore(player)
	if not GameStore.Categories then
		return false
	end
	local msg = NetworkMessage()
	msg:addByte(GameStore.SendingPackets.S_OpenStore)
	msg:addByte(0x00)

	msg:addU16(#GameStore.Categories)
	for k, category in ipairs(GameStore.Categories) do
		msg:addString(category.name)
		msg:addString(category.description)

		if player:getClient().version >= 1093 then
			msg:addByte(category.state or GameStore.States.STATE_NONE)
		end

		msg:addByte(#category.icons)
		for m, icon in ipairs(category.icons) do
			msg:addString(icon)
		end

		msg:addString(category.parentCategory)
	end
	msg:sendToPlayer(player)

	sendCoinBalanceUpdating(player, true)
end

function sendShowStoreOffers(player, category)
	local msg = NetworkMessage()
	msg:addByte(GameStore.SendingPackets.S_StoreOffers)

	msg:addString(category.name)

	msg:addU16(category.offers and #category.offers or 0x00)

	if category.offers then
		for k, offer in ipairs(category.offers) do
			msg:addU32(offer.id and offer.id or 0xFFFF) -- we later detect this number!

			local name = ""
			if offer.type == GameStore.OfferTypes.OFFER_TYPE_ITEM and offer.count then
				name = offer.count .. "x "
			end

			if offer.type == GameStore.OfferTypes.OFFER_TYPE_STACKABLE and offer.count then
				name = offer.count .. "x "
			end

			name = name .. (offer.name or "Something Special")

			msg:addString(name)
			msg:addString(offer.description or GameStore.getDefaultDescription(offer.type))

			msg:addU32(offer.price and offer.price or 0xFFFF)
			msg:addByte(offer.state or GameStore.States.STATE_NONE) -- default is none

			local disabled, disabledReason = 0, ""
			if offer.disabled == true or not offer.type then
				disabled = 1
			end

			if offer.type ~= GameStore.OfferTypes.OFFER_TYPE_NAMECHANGE and offer.type ~= GameStore.OfferTypes.OFFER_TYPE_SEXCHANGE and not offer.thingId then
				disabled = 1
			end

			if disabled == 1 and offer.disabledReason then -- dynamic disable
				disabledReason = offer.disabledReason
			end

			if disabled ~= 1 then
				if offer.type == GameStore.OfferTypes.OFFER_TYPE_OUTFIT or offer.type == GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON then
					local outfitLookType
					if player:getSex() == PLAYERSEX_MALE then
						outfitLookType = offer.thingId.male
					else
						outfitLookType = offer.thingId.female
					end

					if outfitLookType then
						if offer.type == GameStore.OfferTypes.OFFER_TYPE_OUTFIT and player:hasOutfit(outfitLookType) then
							disabled = 1
							disabledReason = "You already have this outfit."
						elseif offer.type == GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON then
							if player:hasOutfit(outfitLookType) then
								if player:hasOutfit(outfitLookType, offer.addon) then
									disabled = 1
									disabledReason = "You already have this addon."
								end
							else
								disabled = 1
								disabledReason = "You don't have the outfit, you can't buy the addon."
							end
						end
					else
						disabled = 1
						disabledReason = "The offer is fake."
					end
				elseif offer.type == GameStore.OfferTypes.OFFER_TYPE_MOUNT then
					local hasMount = player:hasMount(offer.thingId)
					if hasMount == true then
						disabled = 1
						disabledReason = "You already have this mount."
					end
				elseif offer.type == GameStore.OfferTypes.OFFER_TYPE_PROMOTION then
					if GameStore.canAddPromotionToPlayer(player, offer.thingId).ability == false then
						disabled = 1
						disabledReason = "You can't get this promotion"
					end
				end
			end

			msg:addByte(disabled)

			if disabled == 1 and player:getClient().version >= 1093 then
				msg:addString(disabledReason)
			end

			msg:addByte(#offer.icons)
			for k, icon in ipairs(offer.icons) do
				msg:addString(icon)
			end

			msg:addU16(0) -- We still don't support SubOffers!
		end
	end
	msg:sendToPlayer(player)
end

function sendStoreTransactionHistory(player, page, entriesPerPage)
	local entries = GameStore.retrieveHistoryEntries(player:getAccountId()) -- this makes everything easy!
	if #entries == 0 then
		return addPlayerEvent(sendStoreError, 250, player, GameStore.StoreErrors.STORE_ERROR_HISTORY, "You don't have any entries yet.")
	end

	local toSkip = (page - 1) * entriesPerPage
	for i = 1, toSkip do
		table.remove(entries, 1) -- we remove first!
	end

	local msg = NetworkMessage()
	msg:addByte(GameStore.SendingPackets.S_OpenTransactionHistory)
	msg:addU32(page)
	msg:addU32(#entries > entriesPerPage and 0x01 or 0x00)

	msg:addByte(#entries >= entriesPerPage and entriesPerPage or #entries)
	for k, entry in ipairs(entries) do
		if k >= entriesPerPage then break end
		msg:addU32(entry.time)
		msg:addByte(entry.mode)
		msg:addU32(entry.amount)
		msg:addString(entry.description)
	end
	msg:sendToPlayer(player)
end

function sendStorePurchaseSuccessful(player, message)
	local msg = NetworkMessage()
	msg:addByte(GameStore.SendingPackets.S_CompletePurchase)

	msg:addByte(0x00)

	msg:addString(message)
	msg:addU32(player:getCoinsBalance())
	msg:addU32(player:getCoinsBalance())

	msg:sendToPlayer(player)
end

function sendStoreError(player, errorType, message)
	local msg = NetworkMessage()
	msg:addByte(GameStore.SendingPackets.S_StoreError)

	msg:addByte(errorType)
	msg:addString(message)

	msg:sendToPlayer(player)
end

function sendCoinBalanceUpdating(player, updating)	
	local msg = NetworkMessage()
	msg:addByte(GameStore.SendingPackets.S_CoinBalanceUpdating)
	msg:addByte(0x00)
	msg:sendToPlayer(player)

	if updating == true then
		sendUpdateCoinBalance(player)
	end
end

function sendUpdateCoinBalance(player)	
	local msg = NetworkMessage()
	msg:addByte(GameStore.SendingPackets.S_CoinBalanceUpdating)
	msg:addByte(0x01)

	msg:addByte(GameStore.SendingPackets.S_CoinBalance)
	msg:addByte(0x01)

	msg:addU32(player:getCoinsBalance())
	msg:addU32(player:getCoinsBalance())

	msg:sendToPlayer(player)
end

function sendRequestPurchaseData(player, offerId, type)
	local msg = NetworkMessage()
	msg:addByte(GameStore.SendingPackets.S_RequestPurchaseData)
	msg:addU32(offerId)
	msg:addByte(type)
	msg:sendToPlayer(player)
end

--==GameStoreFunctions==--
GameStore.getCategoryByName = function(name)
	for k, category in ipairs(GameStore.Categories) do
		if category.name:lower() == name:lower() then
			return category
		end
	end
	return nil
end

GameStore.getOfferById = function(id)
	for Cat_k, category in ipairs(GameStore.Categories) do
		if category.offers then
			for Off_k, offer in ipairs(category.offers) do
				if offer.id == id then
					return offer
				end
			end
		end
	end
	return nil
end

GameStore.insertHistory = function(accountId, mode, description, amount)
	return db.asyncQuery(string.format("INSERT INTO `store_history`(`account_id`, `mode`, `description`, `coin_amount`, `time`) VALUES (%s, %s, %s, %s, %s)", accountId, mode, db.escapeString(description), amount, os.time()))
end

GameStore.retrieveHistoryEntries = function(accountId)
	local entries = {}
	local resultId = db.storeQuery("SELECT * FROM `store_history` WHERE `account_id` = " .. accountId .. " ORDER BY `time` DESC LIMIT 15;")
	if resultId ~= false then
		repeat
			local entry = {
				mode = result.getDataInt(resultId, "mode"),
				description = result.getDataString(resultId, "description"),
				amount = result.getDataInt(resultId, "coin_amount"),
				time = result.getDataInt(resultId, "time"),
			}
			table.insert(entries, entry)
		until not result.next(resultId)
		result.free(resultId)
	end
	return entries
end

GameStore.getDefaultDescription = function(offerType)
	local t, descList = GameStore.OfferTypes
	if offerType == t.OFFER_TYPE_OUTFIT or offerType == t.OFFER_TYPE_OUTFIT_ADDON then
		descList = GameStore.DefaultDescriptions.OUTFIT
	elseif offerType == t.OFFER_TYPE_MOUNT then
		descList = GameStore.DefaultDescriptions.MOUNT
	elseif offerType == t.OFFER_TYPE_NAMECHANGE then
		descList = GameStore.DefaultDescriptions.NAMECHANGE
	elseif offerType == t.OFFER_TYPE_SEXCHANGE then
		descList = GameStore.DefaultDescriptions.SEXCHANGE
	else
		return ""
	end

	return descList[math.floor(math.random(1, #descList))] or ""
end

GameStore.canChangeToName = function(name)
	local result = {
		ability = false
	}
	if name:len() < 3 or name:len() > 14 then
		result.reason = "Your new name's length should be lower than 3 or higher than 14."
		return result
	end

	-- just copied from znote aac.
	local words = {"owner", "gamemaster", "hoster", "admin", "staff", "tibia", "account", "god", "anal", "ass", "fuck", "sex", "hitler", "pussy", "dick", "rape", "cm", "gm", "tutor", "counsellor"}
	local split = name:split(" ")
	for k, word in ipairs(words) do
		for k, nameWord in ipairs(split) do
			if nameWord:lower() == word then
				result.reason = "You can't use word \"" .. word .. "\" in your new name."
				return result
			end
		end
	end

	if MonsterType(name) then
		result.reason = "Your new name \"" .. name .. "\" can't be a monster's name."
		return result
	elseif Npc(name) then
		result.reason = "Your new name \"" .. name .. "\" can't be a npc's name."
		return result
	end

	local letters = "{}|_*+-=<>0123456789@#%^&()/*\\.,:;~!\"$"
	for i = 1, letters:len() do
		local c = letters:sub(i, i)
		for i = 1, name:len() do
			local m = name:sub(i, i)
			if m == c then
				result.reason = "You can't use this letter \"" .. c .. "\" in your new name."
				return result
			end
		end
	end
	result.ability = true
	return result
end

GameStore.canAddPromotionToPlayer = function(player, promotion, send)
	local result = {
		ability = true
	}
	local vocation = player:getVocation()
	-- Working --
	local vocationCopy, baseVocation = vocation, vocation
	vocation = vocation:getDemotion()
	while vocation do
		baseVocation = vocation
		vocation = vocation:getDemotion()
	end

	local baseVocationsCount = GameStore.BaseVocationsCount or 4

	local newVocId = (baseVocationsCount * promotion) + baseVocation:getId()

	if not Vocation(newVocId) then
		if send then
			addPlayerEvent(sendStoreError, 350, player, GameStore.StoreErrors.STORE_ERROR_NETWORK, "The offer is fake, please report it!")
		end
		result.ability = false
		return result
	end
	-- If promotion is less than player's voc, or player don't have previous promotion
	if newVocId <= vocationCopy:getId() then
		if send then
			addPlayerEvent(sendStoreError, 350, player, GameStore.StoreErrors.STORE_ERROR_NETWORK, "You already have this promotion!")
		end
		result.ability = false
		return result
	end

	if (newVocId - baseVocationsCount) ~= vocationCopy:getId() then
		if send then
			addPlayerEvent(sendStoreError, 350, player, GameStore.StoreErrors.STORE_ERROR_NETWORK, "You need higher promotion to get his one.")
		end
		result.ability = false
		return result
	end

	result.vocId = newVocId
	return result
end

GameStore.addPromotionToPlayer = function(player, promotion)
	local result = GameStore.canAddPromotionToPlayer(player, promotion, true)
	if result.ability == false then return false end

	local basics = {
		health = 185,
		mana = 40,
		cap = 500
	}

	player:setVocation(result.vocId)
	local newVoc = player:getVocation()
	player:setMaxHealth(basics.health + (newVoc:getHealthGain() * player:getLevel()))
	player:setMaxMana(basics.mana + (newVoc:getManaGain() * player:getLevel()))
	player:setCapacity(basics.cap + (newVoc:getCapacityGain() * player:getLevel()))

	player:addHealth(player:getMaxHealth())
	player:addMana(player:getMaxMana())

	player:sendTextMessage(MESSAGE_INFO_DESCR, "You have been promoted to " .. newVoc:getName())
	return true
end

--==Player==--
function Player.getCoinsBalance(self)
	resultId = db.storeQuery("SELECT `coins` FROM `accounts` WHERE `id` = " .. self:getAccountId())
	if not resultId then return 0 end
	return result.getDataInt(resultId, "coins")
end

function Player.setCoinsBalance(self, coins)
	db.asyncQuery("UPDATE `accounts` SET `coins` = " .. coins .. " WHERE `id` = " .. self:getAccountId())
	return true
end

function Player.canRemoveCoins(self, coins)
	if self:getCoinsBalance() < coins then
		return false
	end
	return true
end

function Player.removeCoinsBalance(self, coins)
	if self:canRemoveCoins(coins) then
		return self:setCoinsBalance(self:getCoinsBalance() - coins)
	end

	return false
end

function Player.addCoinsBalance(self, coins, update)
	self:setCoinsBalance(self:getCoinsBalance() + coins)
	if update then sendCoinBalanceUpdating(self, true) end
	return true
end

function Player.toggleSex(self)
	local currentSex = self:getSex()
	local playerOutfit = self:getOutfit()

	if currentSex == PLAYERSEX_FEMALE then
		self:setSex(PLAYERSEX_MALE)
		playerOutfit.lookType = 128
	else
		self:setSex(PLAYERSEX_FEMALE)
		playerOutfit.lookType = 136
	end
	self:setOutfit(playerOutfit)
end
