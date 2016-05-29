-- Please don't edit those information!
GameStore = {
	ModuleName = "GameStore",
	Developer = "Slavi Dodo",
	Version = "0.3",
	LastUpdated = "[CEST] 15-05-2016 01:42AM"
}

--== Enums ==--
GameStore.OfferTypes = {
	OFFER_TYPE_NONE = 0, -- (this will disable offer)
	OFFER_TYPE_ITEM = 1,
	OFFER_TYPE_OUTFIT = 2,
	OFFER_TYPE_OUTFIT_ADDON = 3,
	OFFER_TYPE_MOUNT = 4,
	OFFER_TYPE_NAMECHANGE = 5,
	OFFER_TYPE_SEXCHANGE = 6,
	OFFER_TYPE_PROMOTION = 7
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
	addPlayerEvent(sendStorePurchaseSuccessful, 350, player, "You have transfered " .. amount .. " coins to " .. reciver .. " successfully")
	
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
			return addPlayerEvent(sendStoreError, 350, player, GameStore.StoreErrors.STORE_ERROR_NETWORK, "The offer is either fake or corrupt.")
		end
		
		-- If no thing id,
		if offer.type ~= GameStore.OfferTypes.OFFER_TYPE_NAMECHANGE and offer.type ~= GameStore.OfferTypes.OFFER_TYPE_SEXCHANGE and not offer.thingId then
			return addPlayerEvent(sendStoreError, 350, player, GameStore.StoreErrors.STORE_ERROR_NETWORK, "The offer is either fake or corrupt.")
		end
		
		-- We remove coins before doing everything, if it fails, we add coins back!
		if not player:removeCoinsBalance(offer.price) then
			return addPlayerEvent(sendStoreError, 350, player, GameStore.StoreErrors.STORE_ERROR_NETWORK, "We couldn't remove coins from your account, try again later.")
		end
		
		-- count is used in type(item), so we need to show (i.e 10x crystal coins)
		local offerCountStr = offer.count and (offer.count .. "x ") or ""
		-- The message which we will send to player!
		local message = "You have purchased " .. offerCountStr .. offer.name .. " for " .. offer.price .. " coins."
		
		-- If offer is item.
		if offer.type == GameStore.OfferTypes.OFFER_TYPE_ITEM then
			local backpack = player:getSlotItem(CONST_SLOT_BACKPACK)
			if backpack and backpack:getEmptySlots() > 0 then
				player:addItemEx(Game.createItem(offer.thingId, offer.count or 1))
			else
				-- ToDo: send items to player's inbox.
				player:addCoinsBalance(offer.price)
				return addPlayerEvent(sendStoreError, 350, player, GameStore.StoreErrors.STORE_ERROR_NETWORK, "Please make sure you have free slots in main backpack.")
			end
		-- If offer is outfit/addon
		elseif offer.type == GameStore.OfferTypes.OFFER_TYPE_OUTFIT or offer.type == GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON then
			player:addOutfitAddon(offer.thingId, offer.addon or 0)
		-- If offer is mount
		elseif offer.type == GameStore.OfferTypes.OFFER_TYPE_MOUNT then
			player:addMount(offer.thingId)
		-- If offer is name change
		elseif offer.type == GameStore.OfferTypes.OFFER_TYPE_NAMECHANGE then
			-- If player typed name yet!
			if productType == GameStore.ClientOfferTypes.CLIENT_STORE_OFFER_NAMECHANGE then
				local newName = msg:getString()
				
				local resultId = db.storeQuery("SELECT * FROM `players` WHERE `name` = '" .. newName .. "'")
				if resultId ~= false then
					player:addCoinsBalance(offer.price, true)
					return addPlayerEvent(sendStoreError, 350, player, GameStore.StoreErrors.STORE_ERROR_NETWORK, "This name is already used, please try again!")
				end
				
				db.asyncQuery("UPDATE `players` SET `name` = '" .. newName .. "' WHERE `id` = " .. player:getGuid())
				return addPlayerEvent(sendStorePurchaseSuccessful, 350, player, "You have successfully changed you name, you must relog to see changes.")
			-- If not, we ask him to do!
			else
				player:addCoinsBalance(offer.price)
				return addPlayerEvent(sendStorePurchaseSuccessful, 350, player, offer.id, GameStore.ClientOfferTypes.CLIENT_STORE_OFFER_NAMECHANGE)
			end
		-- If offer is sex change
		elseif offer.type == GameStore.OfferTypes.OFFER_TYPE_SEXCHANGE then
			player:toggleSex()
		elseif offer.type == GameStore.OfferTypes.OFFER_TYPE_PROMOTION then
			if not GameStore.addPromotionToPlayer(player, offer.thingId) then
				player:addCoinsBalance(offer.price, true)
				return false
			end
		else
			player:addCoinsBalance(offer.price, true)
			return addPlayerEvent(sendStoreError, 350, player, GameStore.StoreErrors.STORE_ERROR_NETWORK, "This offer is fake, please contact admin.")
		end
		-- We add this purchase to history!
		GameStore.insertHistory(player:getAccountId(), GameStore.HistoryTypes.HISTORY_TYPE_NONE, offerCountStr .. offer.name, offer.price * -1)
		-- Send to client that purchase is successful!
		return addPlayerEvent(sendStorePurchaseSuccessful, 350, player, message)
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
	local page = msg:getU16()
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
			
			name = name .. (offer.name or "Something Special")
			
			msg:addString(name)
			msg:addString(offer.description)
			
			
			msg:addU32(offer.price and offer.price or 0xFFFF)
			msg:addByte(offer.state or GameStore.States.STATE_NONE) -- default is none
			
			local disabled, disabledReason
			if offer.disabled == true then disabled = 1 end
			if offer.disableReason then
				disabledReason = offer.disableReason
			end
			
			if disabled ~= 1 then
				disabled = 0
				if offer.type == GameStore.OfferTypes.OFFER_TYPE_OUTFIT or offer.type == GameStore.OfferTypes.OFFER_TYPE_OUTFIT_ADDON then
					local addon = player:hasOutfit(offer.thingId, offer.addon and offer.addon or 0)
					if addon == nil or addon == true then
						disabled = 1
						disabledReason = "You already have this outfit/addon."
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
		return sendStoreError(player, GameStore.StoreErrors.STORE_ERROR_HISTORY, "You don't have any entries yet.")
	end
	
	local toSkip = (page - 1) * entriesPerPage
	for i = 1, toSkip do
		table.remove(entries, 1) -- we remove first!
	end
	
	local msg = NetworkMessage()
	msg:addByte(GameStore.SendingPackets.S_OpenTransactionHistory)
	msg:addU16(page)
	msg:addByte(#entries > entriesPerPage and 0x01 or 0x00)
	
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
	local resultId = db.storeQuery("SELECT * FROM `store_history` WHERE `account_id` = " .. accountId .. " ORDER BY `time` DESC")
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

function Player.removeCoinsBalance(self, coins)
	if self:getCoinsBalance() < coins then
		return false
	end
	return self:setCoinsBalance(self:getCoinsBalance() - coins)
end

function Player.addCoinsBalance(self, coins, update)
	self:setCoinsBalance(self:getCoinsBalance() + coins)
	if update then sendCoinBalanceUpdating(self, true) end
	return true
end

function Player.toggleSex(self)
	local currentSex = self:getSex()
	if currentSex == PLAYERSEX_FEMALE then
		self:setSex(PLAYERSEX_MALE)
	else
		self:setSex(PLAYERSEX_FEMALE)
	end
end
