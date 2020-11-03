-- Advanced NPC System (Created by Jiddo),
-- Modified by TheForgottenServer Team,
-- Modified by The OTX Server Team.

if(Modules == nil) then
	-- Constants used to separate buying from selling.
	SHOPMODULE_SELL_ITEM = 1
	SHOPMODULE_BUY_ITEM = 2
	SHOPMODULE_BUY_ITEM_CONTAINER = 3

	-- Constants used for shop mode. Notice: addBuyableItemContainer is working on all modes
	SHOPMODULE_MODE_TALK = 1 -- Old system used before Tibia 8.2: sell/buy item name
	SHOPMODULE_MODE_TRADE = 2 -- Trade window system introduced in Tibia 8.2
	SHOPMODULE_MODE_BOTH = 3 -- Both working at one time

	-- Used in shop mode
	SHOPMODULE_MODE = SHOPMODULE_MODE_BOTH

	-- Constants used for outfit giving mode
	OUTFITMODULE_FUNCTION_OLD = { doPlayerAddOutfit, canPlayerWearOutfit } -- lookType usage
	OUTFITMODULE_FUNCTION_NEW = { doPlayerAddOutfitId, canPlayerWearOutfitId } -- OutfitId usage

	-- Used in outfit module
	OUTFITMODULE_FUNCTION = OUTFITMODULE_FUNCTION_NEW
	if(OUTFITMODULE_FUNCTION[1] == nil or OUTFITMODULE_FUNCTION[2] == nil) then
		OUTFITMODULE_FUNCTION = OUTFITMODULE_FUNCTION_OLD
	end

	Modules = {
		parseableModules = {}
	}

	StdModule = {}

	-- These callback function must be called with parameters.npcHandler = npcHandler in the parameters table or they will not work correctly.
	-- Notice: The members of StdModule have not yet been tested. If you find any bugs, please report them to me.
	-- Usage:
		-- keywordHandler:addKeyword({'offer'}, StdModule.say, {npcHandler = npcHandler, text = 'I sell many powerful melee weapons.'})
	function StdModule.say(cid, message, keywords, parameters, node)
		local npcHandler = parameters.npcHandler
		if(npcHandler == nil) then
			print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'StdModule.say - Call without any npcHandler instance.')
			return false
		end

		local onlyFocus = (parameters.onlyFocus == nil or parameters.onlyFocus == true)
		if(not npcHandler:isFocused(cid) and onlyFocus) then
			return false
		end

		local parseInfo = {[TAG_PLAYERNAME] = getCreatureName(cid)}
		npcHandler:say(npcHandler:parseMessage(parameters.text or parameters.message, parseInfo), cid, parameters.publicize and true)
		if(parameters.reset) then
			npcHandler:resetNpc(cid)
		elseif(parameters.moveup and type(parameters.moveup) == 'number') then
			npcHandler.keywordHandler:moveUp(parameters.moveup)
		end

		return true
	end

	--Usage:
		-- local node1 = keywordHandler:addKeyword({'promot'}, StdModule.say, {npcHandler = npcHandler, text = 'I can promote you for 20000 brozne coins. Do you want me to promote you?'})
		-- 		node1:addChildKeyword({'yes'}, StdModule.promotePlayer, {npcHandler = npcHandler, cost = 20000, promotion = 1, level = 20}, text = 'Congratulations! You are now promoted.')
		-- 		node1:addChildKeyword({'no'}, StdModule.say, {npcHandler = npcHandler, text = 'Alright then, come back when you are ready.'}, reset = true)
	function StdModule.promotePlayer(cid, message, keywords, parameters, node)
		local npcHandler = parameters.npcHandler
		if(npcHandler == nil) then
			print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'StdModule.promotePlayer - Call without any npcHandler instance.')
			return false
		end

		if(not npcHandler:isFocused(cid)) then
			return false
		end

		if(isPremium(cid) or not getBooleanFromString(getConfigValue('premiumForPromotion'))) then
			if(getPlayerPromotionLevel(cid) >= parameters.promotion) then
				npcHandler:say('You are already promoted!', cid)
			elseif(getPlayerLevel(cid) < parameters.level) then
				npcHandler:say('I am sorry, but I can only promote you once you have reached level ' .. parameters.level .. '.', cid)
			elseif(not doPlayerRemoveMoney(cid, parameters.cost)) then
				npcHandler:say('You do not have enough money!', cid)
			else
				doPlayerSetPromotionLevel(cid, parameters.promotion)
				npcHandler:say(parameters.text, cid)
			end
		else
			npcHandler:say("You need a premium account in order to get promoted.", cid)
		end

		npcHandler:resetNpc(cid)
		return true
	end

	function StdModule.learnSpell(cid, message, keywords, parameters, node)
		local npcHandler = parameters.npcHandler
		if(npcHandler == nil) then
			print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'StdModule.learnSpell - Call without any npcHandler instance.')
			return false
		end

		if(not npcHandler:isFocused(cid)) then
			return false
		end

		if(isPremium(cid) or not(parameters.premium)) then
			if(getPlayerLearnedInstantSpell(cid, parameters.spellName)) then
				npcHandler:say('You already know this spell.', cid)
			elseif(getPlayerLevel(cid) < parameters.level) then
				npcHandler:say('You need to obtain a level of ' .. parameters.level .. ' or higher to be able to learn ' .. parameters.spellName .. '.', cid)
			elseif(not parameters.vocation(cid)) then
				npcHandler:say('This spell is not for your vocation', cid)
			elseif(not doPlayerRemoveMoney(cid, parameters.price)) then
				npcHandler:say('You do not have enough money, this spell costs ' .. parameters.price .. ' gold coins.', cid)
			else
				npcHandler:say('You have learned ' .. parameters.spellName .. '.', cid)
				playerLearnInstantSpell(cid, parameters.spellName)
			end
		else
			npcHandler:say('You need a premium account in order to buy ' .. parameters.spellName .. '.', cid)
		end

		npcHandler:resetNpc(cid)
		return true
	end

	function StdModule.bless(cid, message, keywords, parameters, node)
		local npcHandler = parameters.npcHandler
		if(npcHandler == nil) then
			print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'StdModule.bless - Call without any npcHandler instance.')
			return false
		end

		if(not getBooleanFromString(getConfigValue('blessings'))) then
			npcHandler:say("Sorry, but Gods moved back my permission to bless anyone.", cid)
			return false
		end

		if(not npcHandler:isFocused(cid)) then
			return false
		end

		if(isPremium(cid) or not getBooleanFromString(getConfigValue('blessingsOnlyPremium')) or not parameters.premium) then
			local price = parameters.baseCost
			if(getPlayerLevel(cid) > parameters.startLevel) then
				price = (price + ((math.min(parameters.endLevel, getPlayerLevel(cid)) - parameters.startLevel) * parameters.levelCost))
			end

			if(parameters.number > 0) then
				if(getPlayerBlessing(cid, parameters.number)) then
					npcHandler:say("Gods have already blessed you with this blessing!", cid)
				elseif(not doPlayerRemoveMoney(cid, price)) then
					npcHandler:say("You don't have enough money for blessing.", cid)
				else
					npcHandler:say("You have been blessed by one of the five gods!", cid)
					doPlayerAddBlessing(cid, parameters.number)
				end
			else
				if(getPlayerPVPBlessing(cid)) then
					npcHandler:say("Gods have already blessed you with this blessing!", cid)
				elseif(not doPlayerRemoveMoney(cid, price)) then
					npcHandler:say("You don't have enough money for blessing.", cid)
				else
					local any = false
					for i = 1, 5 do
						if(getPlayerBlessing(cid, i)) then
							any = true
							break
						end
					end

					if(any) then
						npcHandler:say("You have been blessed by the god of war!", cid)
						doPlayerSetPVPBlessing(cid)
					else
						npcHandler:say("You need to be blessed by at least one god to get this blessing.", cid)
					end
				end
			end
		else
			npcHandler:say('You need a premium account in order to be blessed.', cid)
		end

		npcHandler:resetNpc(cid)
		return true
	end

	function StdModule.travel(cid, message, keywords, parameters, node)
		local npcHandler = parameters.npcHandler
		if(npcHandler == nil) then
			print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'StdModule.travel - Call without any npcHandler instance.')
			return false
		end

		if(not npcHandler:isFocused(cid)) then
			return false
		end

		local storage, pzLocked = parameters.storageValue or (EMPTY_STORAGE + 1), parameters.allowLocked or false
		if(parameters.premium and not isPremium(cid)) then
			npcHandler:say('I\'m sorry, but you need a premium account in order to travel onboard our ships.', cid)
		elseif(parameters.level ~= nil and getPlayerLevel(cid) < parameters.level) then
			npcHandler:say('You must reach level ' .. parameters.level .. ' before I can let you go there.', cid)
		elseif(parameters.storageId ~= nil and getPlayerStorageValue(cid, parameters.storageId) < storage) then
			npcHandler:say(parameters.storageInfo or 'You may not travel there yet!', cid)
		elseif(not pzLocked and isPlayerPzLocked(cid)) then
			npcHandler:say('First get rid of those blood stains! You are not going to ruin my vehicle!', cid)
		elseif(not doPlayerRemoveMoney(cid, parameters.cost)) then
			npcHandler:say('You don\'t have enough money.', cid)
		else
			npcHandler:say('Set the sails!', cid)
			npcHandler:releaseFocus(cid)

			doTeleportThing(cid, parameters.destination, false)
			doSendMagicEffect(parameters.destination, CONST_ME_TELEPORT)
		end

		npcHandler:resetNpc(cid)
		return true
	end

	FocusModule = {
		npcHandler = nil
	}

	-- Creates a new instance of FocusModule without an associated NpcHandler.
	function FocusModule:new()
		local obj = {}
		setmetatable(obj, self)
		self.__index = self
		return obj
	end

	-- Inits the module and associates handler to it.
	function FocusModule:init(handler)
		self.npcHandler = handler
		for i, word in pairs(FOCUS_GREETWORDS) do
			local obj = {}
			table.insert(obj, word)

			obj.callback = FOCUS_GREETWORDS.callback or FocusModule.messageMatcher
			handler.keywordHandler:addKeyword(obj, FocusModule.onGreet, {module = self})
		end

		for i, word in pairs(FOCUS_FAREWELLWORDS) do
			local obj = {}
			table.insert(obj, word)

			obj.callback = FOCUS_FAREWELLWORDS.callback or FocusModule.messageMatcher
			handler.keywordHandler:addKeyword(obj, FocusModule.onFarewell, {module = self})
		end
	end

	-- Greeting callback function.
	function FocusModule.onGreet(cid, message, keywords, parameters)
		parameters.module.npcHandler:onGreet(cid)
		return true
	end

	-- UnGreeting callback function.
	function FocusModule.onFarewell(cid, message, keywords, parameters)
		if(not parameters.module.npcHandler:isFocused(cid)) then
			return false
		end

		parameters.module.npcHandler:onFarewell(cid)
		parameters.module.npcHandler:resetNpc(cid)
		return true
	end

	-- Custom message matching callback function for greeting messages.
	function FocusModule.messageMatcher(keywords, message)
		local spectators = getSpectators(getCreaturePosition(getNpcId()), 7, 7)
		for i, word in pairs(keywords) do
			if(type(word) == 'string') then
				if(string.find(message, word) and not string.find(message, '[%w+]' .. word) and not string.find(message, word .. '[%w+]')) then
					if(string.find(message, getCreatureName(getNpcId()))) then
						return true
					end

					for i, uid in ipairs(spectators) do
						if(string.find(message, getCreatureName(uid))) then
							return false
						end
					end

					return true
				end
			end
		end

		return false
	end

	KeywordModule = {
		npcHandler = nil
	}
	-- Add it to the parseable module list.
	Modules.parseableModules['module_keywords'] = KeywordModule

	function KeywordModule:new()
		local obj = {}
		setmetatable(obj, self)
		self.__index = self
		return obj
	end

	function KeywordModule:init(handler)
		self.npcHandler = handler
	end

	-- Parses all known parameters.
	function KeywordModule:parseParameters()
		local ret = NpcSystem.getParameter('keywords')
		if(ret ~= nil) then
			self:parseKeywords(ret)
		end
	end

	function KeywordModule:parseKeywords(data)
		local n = 1
		for keys in string.gmatch(data, '[^;]+') do
			local i = 1

			local keywords = {}
			for temp in string.gmatch(keys, '[^,]+') do
				table.insert(keywords, temp)
				i = i + 1
			end

			if(i ~= 1) then
				local reply = NpcSystem.getParameter('keyword_reply' .. n)
				if(reply ~= nil) then
					self:addKeyword(keywords, reply)
				else
					print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'Parameter \'' .. 'keyword_reply' .. n .. '\' missing. Skipping...')
				end
			else
				print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'No keywords found for keyword set #' .. n .. '. Skipping...')
			end

			n = n + 1
		end
	end

	function KeywordModule:addKeyword(keywords, reply)
		self.npcHandler.keywordHandler:addKeyword(keywords, StdModule.say, {npcHandler = self.npcHandler, onlyFocus = true, text = reply, reset = true})
	end

	TravelModule = {
		npcHandler = nil,
		destinations = nil,
		yesNode = nil,
		noNode = nil,
	}
	-- Add it to the parseable module list.
	Modules.parseableModules['module_travel'] = TravelModule

	function TravelModule:new()
		local obj = {}
		setmetatable(obj, self)
		self.__index = self
		return obj
	end

	function TravelModule:init(handler)
		self.npcHandler = handler
		self.yesNode = KeywordNode:new(SHOP_YESWORD, TravelModule.onConfirm, {module = self})
		self.noNode = KeywordNode:new(SHOP_NOWORD, TravelModule.onDecline, {module = self})

		self.destinations = {}
	end

	-- Parses all known parameters.
	function TravelModule:parseParameters()
		local ret = NpcSystem.getParameter('travel_destinations')
		if(ret ~= nil) then
			self:parseDestinations(ret)
			for _, word in ipairs({'destination', 'list', 'where', 'travel'}) do
				self.npcHandler.keywordHandler:addKeyword({word}, TravelModule.listDestinations, {module = self})
			end
		end
	end

	function TravelModule:parseDestinations(data)
		for destination in string.gmatch(data, '[^;]+') do
			local i, name, pos, cost, premium, level, storage = 1, nil, {x = nil, y = nil, z = nil}, nil, false
			for tmp in string.gmatch(destination, '[^,]+') do
				if(i == 1) then
					name = tmp
				elseif(i == 2) then
					pos.x = tonumber(tmp)
				elseif(i == 3) then
					pos.y = tonumber(tmp)
				elseif(i == 4) then
					pos.z = tonumber(tmp)
				elseif(i == 5) then
					cost = tonumber(tmp)
				elseif(i == 6) then
					premium = getBooleanFromString(tmp)
				else
					print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'Unknown parameter found in travel destination parameter.', tmp, destination)
				end

				i = i + 1
			end

			if(name ~= nil and pos.x ~= nil and pos.y ~= nil and pos.z ~= nil and cost ~= nil) then
				self:addDestination(name, pos, cost, premium)
			else
				print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'Parameter(s) missing for travel destination:', name, pos, cost, premium)
			end
		end
	end

	function TravelModule:addDestination(name, position, price, premium)
		table.insert(self.destinations, name)
		local parameters = {
			cost = price,
			destination = position,
			premium = premium,
			module = self
		}

		local keywords, bringWords = {}, {}
		table.insert(keywords, name)

		table.insert(bringWords, 'bring me to ' .. name)
		self.npcHandler.keywordHandler:addKeyword(bringWords, TravelModule.bring, parameters)

		local node = self.npcHandler.keywordHandler:addKeyword(keywords, TravelModule.travel, parameters)
		node:addChildKeywordNode(self.yesNode)
		node:addChildKeywordNode(self.noNode)
	end

	function TravelModule.travel(cid, message, keywords, parameters, node)
		local module = parameters.module
		if(not module.npcHandler:isFocused(cid)) then
			return false
		end

		module.npcHandler:say('Do you want to travel to ' .. keywords[1] .. ' for ' .. parameters.cost .. ' gold coins?', cid)
		return true
	end

	function TravelModule.onConfirm(cid, message, keywords, parameters, node)
		local module = parameters.module
		if(not module.npcHandler:isFocused(cid)) then
			return false
		end

		local parent = node:getParent():getParameters()
		if(isPremium(cid) or not parent.premium) then
			if(not isPlayerPzLocked(cid)) then
				if(doPlayerRemoveMoney(cid, parent.cost)) then
					module.npcHandler:say('Set the sails!', cid)
					module.npcHandler:releaseFocus(cid)

					doTeleportThing(cid, parent.destination, true)
					doSendMagicEffect(parent.destination, CONST_ME_TELEPORT)
				else
					module.npcHandler:say('You don\'t have enough money.', cid)
				end
			else
				module.npcHandler:say('First get rid of those blood stains! You are not going to ruin my vehicle!', cid)
			end
		else
			module.npcHandler:say('I\'m sorry, but you need a premium account in order to travel onboard our ships.', cid)
		end

		module.npcHandler:resetNpc(cid)
		return true
	end

	-- onDecline keyword callback function. Generally called when the player sais 'no' after wanting to buy an item.
	function TravelModule.onDecline(cid, message, keywords, parameters, node)
		local module = parameters.module
		if(not module.npcHandler:isFocused(cid)) then
			return false
		end

		module.npcHandler:say(module.npcHandler:parseMessage(module.npcHandler:getMessage(MESSAGE_DECLINE), {[TAG_PLAYERNAME] = getCreatureName(cid)}), cid)
		module.npcHandler:resetNpc(cid)
		return true
	end

	function TravelModule.bring(cid, message, keywords, parameters, node)
		local module = parameters.module
		if(not module.npcHandler:isFocused(cid)) then
			return false
		end

		if((isPremium(cid) or not parameters.premium) and not isPlayerPzLocked(cid) and doPlayerRemoveMoney(cid, parameters.cost)) then
			module.npcHandler:say('Set the sails!', cid)
			module.npcHandler:releaseFocus(cid)

			doTeleportThing(cid, parameters.destination, false)
			doSendMagicEffect(parameters.destination, CONST_ME_TELEPORT)
		end

		module.npcHandler:releaseFocus(cid)
		return true
	end

	function TravelModule.listDestinations(cid, message, keywords, parameters, node)
		local module = parameters.module
		if(not module.npcHandler:isFocused(cid)) then
			return false
		end

		local msg = nil
		for _, destination in ipairs(module.destinations) do
			if(msg ~= nil) then
				msg = msg .. ", "
			else
				msg = ""
			end

			msg = msg .. "{" .. destination .. "}"
		end

		module.npcHandler:say(msg .. ".", cid)
		module.npcHandler:resetNpc(cid)
		return true
	end

	OutfitModule = {
		npcHandler = nil,
		outfits = nil,
		yesNode = nil,
		noNode = nil,
	}
	-- Add it to the parseable module list.
	Modules.parseableModules['module_outfit'] = OutfitModule

	function OutfitModule:new()
		if(OUTFITMODULE_FUNCTION[1] == nil or OUTFITMODULE_FUNCTION[2] == nil) then
			return nil
		end

		local obj = {}
		setmetatable(obj, self)
		self.__index = self
		return obj
	end

	function OutfitModule:init(handler)
		self.npcHandler = handler
		self.yesNode = KeywordNode:new(SHOP_YESWORD, OutfitModule.onConfirm, {module = self})
		self.noNode = KeywordNode:new(SHOP_NOWORD, OutfitModule.onDecline, {module = self})

		self.outfits = {}
	end

	-- Parses all known parameters.
	function OutfitModule:parseParameters()
		local ret = NpcSystem.getParameter('outfits')
		if(ret ~= nil) then
			self:parseKeywords(ret)
			for _, word in ipairs({'outfits', 'addons'}) do
				self.npcHandler.keywordHandler:addKeyword({word}, OutfitModule.listOutfits, {module = self})
			end
		end
	end

	function OutfitModule:parseKeywords(data)
		local n = 1
		for outfit in string.gmatch(data, '[^;]+') do
			local i, keywords = 1, {}
			for tmp in string.gmatch(outfit, '[^,]+') do
				table.insert(keywords, tmp)
				i = i + 1
			end

			if(i > 0) then
				local ret = NpcSystem.getParameter('outfit' .. n)
				if(ret ~= nil) then
					self:parseList(keywords, ret)
				else
					print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'Missing \'outfit' .. n .. '\' parameter, skipping...')
				end
			else
				print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'No keywords found for outfit set #' .. n .. ', skipping...')
			end

			n = n + 1
		end
	end

	function OutfitModule:parseList(keywords, data)
		local outfit, items = nil, {}
		for list in string.gmatch(data, '[^;]+') do
			local a, b, c, d, e = nil, nil, nil, nil, 1
			for tmp in string.gmatch(list, '[^,]+') do
				if(e == 1) then
					a = tmp
				elseif(e == 2) then
					b = tmp
				elseif(e == 3) then
					c = tmp
				elseif(e == 4) then
					d = tmp
				else
					print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'Unknown parameter found in outfit list while parsing ' .. (outfit == nil and 'outfit' or 'item') .. '.', tmp, list)
				end

				e = e + 1
			end

			if(outfit == nil) then
				outfit = {tonumber(a), tonumber(b), getBooleanFromString(c), d}
			elseif(a ~= nil) then
				local tmp = tonumber(a)
				if((tmp ~= nil or tostring(a) == "money") and b ~= nil and c ~= nil) then
					a = tmp or 20000
					tmp = tonumber(d)
					if(tmp == nil) then
						tmp = -1
					end

					items[a] = {b, tmp, c}
				else
					print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'Missing parameter(s) for outfit items.', b, c, d)
				end
			else
				print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'Missing base parameter for outfit items.', a)
			end
		end

		if(type(outfit) == 'table') then
			local tmp = true
			for i = 1, 2 do
				if(outfit[i] == nil) then
					tmp = false
					break
				end
			end

			if(tmp and table.maxn(items) > 0) then
				self:addOutfit(keywords, outfit, items)
			else
				print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'Invalid outfit, addon or empty items pool.', data)
			end
		end
	end

	function OutfitModule:addOutfit(keywords, outfit, items)
		table.insert(self.outfits, keywords[1])
		local parameters = {
			outfit = outfit[1],
			addon = outfit[2],
			premium = outfit[3],
			gender = nil,
			items = items,
			module = self
		}

		if(outfit[4] ~= nil) then
			local tmp = string.lower(tostring(outfit[5]))
			if(tmp == 'male' or tmp == '1') then
				parameters.gender = 1
			elseif(tmp == 'female' or tmp == '0') then
				parameters.gender = 0
			end
		end

		for i, name in pairs(keywords) do
			local words = {}
			table.insert(words, name)

			local node = self.npcHandler.keywordHandler:addKeyword(words, OutfitModule.obtain, parameters)
			node:addChildKeywordNode(self.yesNode)
			node:addChildKeywordNode(self.noNode)
		end
	end

	function OutfitModule.obtain(cid, message, keywords, parameters, node)
		local module = parameters.module
		if(not module.npcHandler:isFocused(cid)) then
			return false
		end

		local i, items, size = 0, nil, table.maxn(parameters.items)
		for k, v in pairs(parameters.items) do
			if(v[1] ~= "storageset") then
				i = i + 1
				if(items ~= nil) then
					if(i == size) then
						items = items .. " and "
					else
						items = items .. ", "
					end
				else
					items = ""
				end

				if(tonumber(v[1]) ~= nil and tonumber(v[1]) > 1) then
					items = items .. v[1] .. " "
				end

				items = items .. v[3]
			end
		end

		module.npcHandler:say('Do you want ' .. keywords[1] .. ' ' .. (addon == 0 and "outfit" or "addon") .. ' for ' .. items .. '?', cid)
		return true

	end

	function OutfitModule.onConfirm(cid, message, keywords, parameters, node)
		local module = parameters.module
		if(not module.npcHandler:isFocused(cid)) then
			return false
		end

		local parent = node:getParent():getParameters()
		if(isPremium(cid) or not parent.premium) then
			if(not OUTFITMODULE_FUNCTION[2](cid, parent.outfit, parent.addon)) then
				if(parent.addon == 0 or OUTFITMODULE_FUNCTION[2](cid, parent.outfit)) then
					if(parent.gender == nil or parent.gender == getPlayerSex(cid)) then
						local found = true
						for k, v in pairs(parent.items) do
							local tmp = tonumber(v[1])
							if(tmp == nil) then
								if(v[1] == "storagecheck") then
									if(getCreatureStorage(cid, k) < v[2]) then
										found = false
									end
								elseif(v[1] == "outfitid") then
									if(not canPlayerWearOutfitId(cid, k, v[2])) then
										found = false
									end
								elseif(v[1] == "outfit") then
									if(not canPlayerWearOutfit(cid, k, v[2])) then
										found = false
									end
								else
									found = false
								end
							elseif(k == 20000) then
								if(getPlayerMoney(cid) < tmp) then
									found = false
								end
							elseif(getPlayerItemCount(cid, k, v[2]) < tmp) then
								found = false
							end

							if(not found) then
								break
							end
						end

						if(found) then
							for k, v in pairs(parent.items) do
								if(tonumber(v[1]) ~= nil) then
									if(k == 20000) then
										doPlayerRemoveMoney(cid, v[1])
									else
										doPlayerRemoveItem(cid, k, v[1], v[2])
									end
								elseif(v[1] == "storageset") then
									doCreatureSetStorage(cid, k, v[2])
								end
							end

							module.npcHandler:say('It was a pleasure to dress you.', cid)
							OUTFITMODULE_FUNCTION[1](cid, parent.outfit, parent.addon)
							doPlayerSetStorageValue(cid, parent.storageId, storage)
						else
							module.npcHandler:say('You don\'t have these items!', cid)
						end
					else
						module.npcHandler:say('Sorry, this ' .. (parent.addon == 0 and 'outfit' or 'addon') .. ' is not for your gender.', cid)
					end
				else
					module.npcHandler:say('I will not dress you with addon of outfit you cannot wear!', cid)
				end
			else
				module.npcHandler:say('You already have this ' .. (parent.addon == 0 and 'outfit' or 'addon') .. '!', cid)
			end
		else
			module.npcHandler:say('Sorry, I dress only premium players.', cid)
		end

		module.npcHandler:resetNpc(cid)
		return true
	end

	-- onDecline keyword callback function. Generally called when the player sais 'no' after wanting to buy an item.
	function OutfitModule.onDecline(cid, message, keywords, parameters, node)
		local module = parameters.module
		if(not module.npcHandler:isFocused(cid)) then
			return false
		end

		module.npcHandler:say(module.npcHandler:parseMessage(module.npcHandler:getMessage(MESSAGE_DECLINE), {[TAG_PLAYERNAME] = getCreatureName(cid)}), cid)
		module.npcHandler:resetNpc(cid)
		return true
	end

	function OutfitModule.listOutfits(cid, message, keywords, parameters, node)
		local module = parameters.module
		if(not module.npcHandler:isFocused(cid)) then
			return false
		end

		local msg, size = nil, table.maxn(module.outfits)
		if(size > 0) then
			for i, outfit in ipairs(module.outfits) do
				if(msg ~= nil) then
					if(i == size) then
						msg = msg .. " and "
					else
						msg = msg .. ", "
					end
				else
					msg = "I can dress you into "
				end

				msg = msg .. "{" .. outfit .. "}"
			end
		else
			msg = "Sorry, I have nothing to offer right now."
		end

		module.npcHandler:say(msg .. ".", cid)
		module.npcHandler:resetNpc(cid)
		return true
	end

	ShopModule = {
		npcHandler = nil,
		yesNode = nil,
		noNode = nil,
		noText = '',
		maxCount = 100,
		amount = 0
	}

	-- Add it to the parseable module list.
	Modules.parseableModules['module_shop'] = ShopModule

	-- Creates a new instance of ShopModule
	function ShopModule:new()
		local obj = {}
		setmetatable(obj, self)
		self.__index = self
		return obj
	end

	-- Parses all known parameters.
	function ShopModule:parseParameters()
		local ret = NpcSystem.getParameter('shop_buyable')
		if(ret ~= nil) then
			self:parseBuyable(ret)
		end

		local ret = NpcSystem.getParameter('shop_sellable')
		if(ret ~= nil) then
			self:parseSellable(ret)
		end

		local ret = NpcSystem.getParameter('shop_buyable_containers')
		if(ret ~= nil) then
			self:parseBuyableContainers(ret)
		end
	end

	-- Parse a string contaning a set of buyable items.
	function ShopModule:parseBuyable(data)
		for item in string.gmatch(data, '[^;]+') do
			local i, name, itemid, cost, subType, realName = 1, nil, nil, nil, nil, nil
			for tmp in string.gmatch(item, '[^,]+') do
				if(i == 1) then
					name = tmp
				elseif(i == 2) then
					itemid = tonumber(tmp)
				elseif(i == 3) then
					cost = tonumber(tmp)
				elseif(i == 4) then
					subType = tonumber(tmp)
				elseif(i == 5) then
					realName = tmp
				else
					print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'Unknown parameter found in buyable items parameter.', tmp, item)
				end

				i = i + 1
			end

			if(SHOPMODULE_MODE == SHOPMODULE_MODE_TRADE) then
				if(itemid ~= nil and cost ~= nil) then
					if(isItemFluidContainer(itemid) and subType == nil) then
						print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'SubType missing for parameter item:', item)
					else
						self:addBuyableItem(nil, itemid, cost, subType, realName)
					end
				else
					print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'Parameter(s) missing for item:', itemid, cost)
				end
			elseif(name ~= nil and itemid ~= nil and cost ~= nil) then
				if(isItemFluidContainer(itemid) and subType == nil) then
					print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'SubType missing for parameter item:', item)
				else
					local names = {}
					table.insert(names, name)
					self:addBuyableItem(names, itemid, cost, subType, realName)
				end
			else
				print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'Parameter(s) missing for item:', name, itemid, cost)
			end
		end
	end

	-- Parse a string contaning a set of sellable items.
	function ShopModule:parseSellable(data)
		for item in string.gmatch(data, '[^;]+') do
			local i, name, itemid, cost, realName = 1, nil, nil, nil, nil
			for temp in string.gmatch(item, '[^,]+') do
				if(i == 1) then
					name = temp
				elseif(i == 2) then
					itemid = tonumber(temp)
				elseif(i == 3) then
					cost = tonumber(temp)
				elseif(i == 4) then
					realName = temp
				else
					print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'Unknown parameter found in sellable items parameter.', temp, item)
				end
				i = i + 1
			end

			if(SHOPMODULE_MODE == SHOPMODULE_MODE_TRADE) then
				if(itemid ~= nil and cost ~= nil) then
					self:addSellableItem(nil, itemid, cost, realName)
				else
					print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'Parameter(s) missing for item:', itemid, cost)
				end
			elseif(name ~= nil and itemid ~= nil and cost ~= nil) then
				local names = {}
				table.insert(names, name)
				self:addSellableItem(names, itemid, cost, realName)
			else
				print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'Parameter(s) missing for item:', name, itemid, cost)
			end
		end
	end

	-- Parse a string contaning a set of buyable items.
	function ShopModule:parseBuyableContainers(data)
		for item in string.gmatch(data, '[^;]+') do
			local i, name, container, itemid, cost, subType, realName = 1, nil, nil, nil, nil, nil, nil
			for temp in string.gmatch(item, '[^,]+') do
				if(i == 1) then
					name = temp
				elseif(i == 2) then
					itemid = tonumber(temp)
				elseif(i == 3) then
					itemid = tonumber(temp)
				elseif(i == 4) then
					cost = tonumber(temp)
				elseif(i == 5) then
					subType = tonumber(temp)
				elseif(i == 6) then
					realName = temp
				else
					print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'Unknown parameter found in buyable items parameter.', temp, item)
				end
				i = i + 1
			end

			if(name ~= nil and container ~= nil and itemid ~= nil and cost ~= nil) then
				if(isItemFluidContainer(itemid) and subType == nil) then
					print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'SubType missing for parameter item:', item)
				else
					local names = {}
					table.insert(names, name)
					self:addBuyableItemContainer(names, container, itemid, cost, subType, realName)
				end
			else
				print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'Parameter(s) missing for item:', name, container, itemid, cost)
			end
		end
	end

	-- Initializes the module and associates handler to it.
	function ShopModule:init(handler)
		self.npcHandler = handler
		self.yesNode = KeywordNode:new(SHOP_YESWORD, ShopModule.onConfirm, {module = self})
		self.noNode = KeywordNode:new(SHOP_NOWORD, ShopModule.onDecline, {module = self})

		self.noText = handler:getMessage(MESSAGE_DECLINE)
		if(SHOPMODULE_MODE ~= SHOPMODULE_MODE_TALK) then
			for i, word in pairs(SHOP_TRADEREQUEST) do
				local obj = {}
				table.insert(obj, word)

				obj.callback = SHOP_TRADEREQUEST.callback or ShopModule.messageMatcher
				handler.keywordHandler:addKeyword(obj, ShopModule.requestTrade, {module = self})
			end
		end
	end

	-- Custom message matching callback function for requesting trade messages.
	function ShopModule.messageMatcher(keywords, message)
		for i, word in pairs(keywords) do
			if(type(word) == 'string' and string.find(message, word) and not string.find(message, '[%w+]' .. word) and not string.find(message, word .. '[%w+]')) then
				return true
			end
		end

		return false
	end

	-- Resets the module-specific variables.
	function ShopModule:reset()
		self.amount = 0
	end

	-- Function used to match a number value from a string.
	function ShopModule:getCount(message)
		local ret, b, e = 1, string.find(message, PATTERN_COUNT)
		if(b ~= nil and e ~= nil) then
			ret = tonumber(string.sub(message, b, e))
		end

		return math.max(1, math.min(self.maxCount, ret))
	end

	-- Adds a new buyable item.
	--	names = A table containing one or more strings of alternative names to this item. Used only for old buy/sell system.
	--	itemid = The itemid of the buyable item
	--	cost = The price of one single item
	--	subType - The subType of each rune or fluidcontainer item. Can be left out if it is not a rune/fluidcontainer. Default value is 0 and 1 (depending on shop mode)
	--	realName - The real, full name for the item. Will be used as ITEMNAME in MESSAGE_ONBUY and MESSAGE_ONSELL if defined. Default value is nil (getItemNameById will be used)
	function ShopModule:addBuyableItem(names, itemid, cost, subType, realName)
		if(type(subType) == 'string' and realName == nil) then
			realName = subType
			subType = nil
		end

		local v = getItemInfo(itemid)
		if(SHOPMODULE_MODE ~= SHOPMODULE_MODE_TALK) then
			local item = {
				id = itemid,
				buy = cost,
				sell = -1,
				subType = tonumber(subType) or (v.charges > 0 and v.charges or 0),
				name = realName or v.name
			}

			for i, shopItem in ipairs(self.npcHandler.shopItems) do
				if(shopItem.id == item.id and (shopItem.subType == item.subType or shopItem.subType == 0)) then
					if(item.sell ~= shopItem.sell) then
						item.sell = shopItem.sell
					end

					self.npcHandler.shopItems[i] = item
					item = nil
					break
				end
			end

			if(item ~= nil) then
				table.insert(self.npcHandler.shopItems, item)
			end
		end

		if(names ~= nil and SHOPMODULE_MODE ~= SHOPMODULE_MODE_TRADE) then
			local parameters = {
				itemid = itemid,
				cost = cost,
				eventType = SHOPMODULE_BUY_ITEM,
				module = self,
				realName = realName or v.name,
				subType = tonumber(subType) or (v.charges > 0 and v.charges or 1)
			}

			for i, name in pairs(names) do
				local keywords = {}
				table.insert(keywords, 'buy')
				table.insert(keywords, name)

				local node = self.npcHandler.keywordHandler:addKeyword(keywords, ShopModule.tradeItem, parameters)
				node:addChildKeywordNode(self.yesNode)
				node:addChildKeywordNode(self.noNode)
			end
		end
	end

	-- Adds a new buyable container of items.
	--	names = A table containing one or more strings of alternative names to this item.
	--	container = Backpack, bag or any other itemid of container where bought items will be stored
	--	itemid = The itemid of the buyable item
	--	cost = The price of one single item
	--	subType - The subType of each rune or fluidcontainer item. Can be left out if it is not a rune/fluidcontainer. Default value is 1.
	--	realName - The real, full name for the item. Will be used as ITEMNAME in MESSAGE_ONBUY and MESSAGE_ONSELL if defined. Default value is nil (getItemNameById will be used)
	function ShopModule:addBuyableItemContainer(names, container, itemid, cost, subType, realName)
		if(names ~= nil) then
			local v = getItemInfo(itemid)
			local parameters = {
				container = container,
				itemid = itemid,
				cost = cost,
				eventType = SHOPMODULE_BUY_ITEM_CONTAINER,
				module = self,
				realName = realName or v.name,
				subType = tonumber(subType) or (v.charges > 0 and v.charges or 1)
			}

			for i, name in pairs(names) do
				local keywords = {}
				table.insert(keywords, 'buy')
				table.insert(keywords, name)

				local node = self.npcHandler.keywordHandler:addKeyword(keywords, ShopModule.tradeItem, parameters)
				node:addChildKeywordNode(self.yesNode)
				node:addChildKeywordNode(self.noNode)
			end
		end
	end

	-- Adds a new sellable item.
	--	names = A table containing one or more strings of alternative names to this item. Used only by old buy/sell system.
	--	itemid = The itemid of the sellable item
	--	cost = The price of one single item
	--	realName - The real, full name for the item. Will be used as ITEMNAME in MESSAGE_ONBUY and MESSAGE_ONSELL if defined. Default value is nil (getItemNameById will be used)
	function ShopModule:addSellableItem(names, itemid, cost, realName)
		local v = getItemInfo(itemid)
		if(SHOPMODULE_MODE ~= SHOPMODULE_MODE_TALK) then
			local item = {
				id = itemid,
				buy = -1,
				sell = cost,
				subType = ((v.charges > 0 and v.stackable) and v.charges or 0),
				name = realName or v.name
			}

			for i, shopItem in ipairs(self.npcHandler.shopItems) do
				if(shopItem.id == item.id and shopItem.subType == item.subType) then
					if(item.buy ~= shopItem.buy) then
						item.buy = shopItem.buy
					end

					self.npcHandler.shopItems[i] = item
					item = nil
					break
				end
			end

			if(item ~= nil) then
				table.insert(self.npcHandler.shopItems, item)
			end
		end

		if(names ~= nil and SHOPMODULE_MODE ~= SHOPMODULE_MODE_TRADE) then
			local parameters = {
				itemid = itemid,
				cost = cost,
				eventType = SHOPMODULE_SELL_ITEM,
				module = self,
				realName = realName or v.name
			}

			for i, name in pairs(names) do
				local keywords = {}
				table.insert(keywords, 'sell')
				table.insert(keywords, name)

				local node = self.npcHandler.keywordHandler:addKeyword(keywords, ShopModule.tradeItem, parameters)
				node:addChildKeywordNode(self.yesNode)
				node:addChildKeywordNode(self.noNode)
			end
		end
	end

	-- onModuleReset callback function. Calls ShopModule:reset()
	function ShopModule:callbackOnModuleReset()
		self:reset()
		return true
	end

	-- Callback onBuy() function. If you wish, you can change certain Npc to use your onBuy().
	function ShopModule:callbackOnBuy(cid, itemid, subType, amount, ignoreCap, inBackpacks)
		local shopItem = nil
		for _, item in ipairs(self.npcHandler.shopItems) do
			if(item.id == itemid and item.subType == subType) then
				shopItem = item
				break
			end
		end

		if(shopItem == nil) then
			print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'ShopModule.onBuy - Item not found on shopItems list')
			return false
		end

		if(shopItem.buy == -1) then
			print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'ShopModule.onBuy - Attempt to purchase an item which only sellable')
			return false
		end

		if(amount <= 0) then
			print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'ShopModule.onBuy - Attempt to purchase ' .. amount .. ' items')
			return false
		end

		local backpack = 1988
		local totalCost = amount * shopItem.buy
		if(inBackpacks) then
			totalCost = totalCost + 20 or totalCost + (math.max(1, math.floor(amount / getContainerCapById(backpack))) * 20)
		end

		local parseInfo = {
			[TAG_PLAYERNAME] = getPlayerName(cid),
			[TAG_ITEMCOUNT] = amount,
			[TAG_TOTALCOST] = totalCost,
			[TAG_ITEMNAME] = shopItem.name
		}
		if(getPlayerMoney(cid) < totalCost) then
			local msg = self.npcHandler:getMessage(MESSAGE_NEEDMONEY)
			doPlayerSendCancel(cid, self.npcHandler:parseMessage(msg, parseInfo))
			return false
		end

		local subType = shopItem.subType or 1
		local a, b, item = doNpcSellItem(cid, itemid, amount, subType, ignoreCap, inBackpacks, backpack)
		
		if(a < amount) then
			local msgId = MESSAGE_NEEDMORESPACE
			if(a == 0) then
				msgId = MESSAGE_NEEDSPACE
			end
			

			local msg = self.npcHandler:getMessage(msgId)
			parseInfo[TAG_ITEMCOUNT] = a

			doPlayerSendCancel(cid, self.npcHandler:parseMessage(msg, parseInfo))
			if(NPCHANDLER_CONVBEHAVIOR ~= CONVERSATION_DEFAULT) then
				self.npcHandler.talkStart[cid] = os.time()
			else
				self.npcHandler.talkStart = os.time()
			end
			if(a > 0) then
			
				local prev_money = getPlayerMoney(cid)
				local removeMoney = doPlayerRemoveMoney(cid, (a * shopItem.buy) + (b * 20), false)
				
				if removeMoney == true then
					parseInfo[TAG_TOTALCOST] = a * shopItem.buy
					local msg = self.npcHandler:getMessage(MESSAGE_BOUGHT)
					doPlayerSendTextMessage(cid, MESSAGE_INFO_DESCR, self.npcHandler:parseMessage(msg, parseInfo))
				else
					for i = 1, #item do
						doRemoveItem(item[i])
					end
					
					local currentMoney = getPlayerMoney(cid)
					if currentMoney ~= prev_money and  currentMoney < prev_money then -- Let's make sure the player did not pay for an item he did not receive.
						doPlayerAddMoney(cid, math.abs(prev_money - currentMoney))
					end
					
					doPlayerSendTextMessage(cid, MESSAGE_INFO_DESCR, "You don't have enough space to receive the change when buying "..amount.. "x ".. getItemNameById(itemid).."!")
				end
				return true
			end

			return false
		else
		
			local prev_money = getPlayerMoney(cid)
			
			if doPlayerRemoveMoney(cid, totalCost, false) == true then
				local msg = self.npcHandler:getMessage(MESSAGE_BOUGHT)
				doPlayerSendTextMessage(cid, MESSAGE_INFO_DESCR, self.npcHandler:parseMessage(msg, parseInfo))
			else
			
				doPlayerSendTextMessage(cid, MESSAGE_INFO_DESCR, "You don't have enough space to receive the change when buying "..amount.. "x ".. getItemNameById(itemid).."!")
				for i = 1, #item do
					doRemoveItem(item[i])
				end
				
				local currentMoney = getPlayerMoney(cid)
				if currentMoney ~= prev_money and  currentMoney < prev_money then -- Let's make sure the player did not pay for an item he did not receive.
					doPlayerAddMoney(cid, math.abs(prev_money - currentMoney))
				end
				
			end
			if(NPCHANDLER_CONVBEHAVIOR ~= CONVERSATION_DEFAULT) then
				self.npcHandler.talkStart[cid] = os.time()
			else
				self.npcHandler.talkStart = os.time()
			end
			return true
		end
	end

	-- Callback onSell() function. If you wish, you can change certain Npc to use your onSell().
	function ShopModule:callbackOnSell(cid, itemid, subType, amount, ignoreEquipped, dummy)
		local shopItem = nil
		for _, item in ipairs(self.npcHandler.shopItems) do
			if(item.id == itemid and item.subType == subType) then
				shopItem = item
				break
			end
		end

		if(shopItem == nil) then
			print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'ShopModule.onSell - Item not found on shopItems list')
			return false
		end

		if(shopItem.sell == -1) then
			print('[Warning - ' .. getCreatureName(getNpcId()) .. '] NpcSystem:', 'ShopModule.onSell - Attempt to sell an item which is only buyable')
			return false
		end

		local parseInfo = {
			[TAG_PLAYERNAME] = getPlayerName(cid),
			[TAG_ITEMCOUNT] = amount,
			[TAG_TOTALCOST] = amount * shopItem.sell,
			[TAG_ITEMNAME] = shopItem.name
		}

		if(subType < 1 or getItemInfo(itemid).stackable) then
			subType = -1
		end

		if(doPlayerRemoveItem(cid, itemid, amount, subType, ignoreEquipped)) then
			local msg = self.npcHandler:getMessage(MESSAGE_SOLD)
			doPlayerSendTextMessage(cid, MESSAGE_INFO_DESCR, self.npcHandler:parseMessage(msg, parseInfo))

			doPlayerAddMoney(cid, amount * shopItem.sell)
			if(NPCHANDLER_CONVBEHAVIOR ~= CONVERSATION_DEFAULT) then
				self.npcHandler.talkStart[cid] = os.time()
			else
				self.npcHandler.talkStart = os.time()
			end

			return true
		end

		local msg = self.npcHandler:getMessage(MESSAGE_NEEDITEM)
		doPlayerSendCancel(cid, self.npcHandler:parseMessage(msg, parseInfo))
		if(NPCHANDLER_CONVBEHAVIOR ~= CONVERSATION_DEFAULT) then
			self.npcHandler.talkStart[cid] = os.time()
		else
			self.npcHandler.talkStart = os.time()
		end

		return false
	end

	-- Callback for requesting a trade window with the NPC.
	function ShopModule.requestTrade(cid, message, keywords, parameters, node)
		local module = parameters.module
		if(not module.npcHandler:isFocused(cid)) then
			return false
		end

		local shop = getShopOwner(cid)
		if(shop and shop == getNpcId()) then
			return true
		end

		if(table.maxn(module.npcHandler.shopItems) == 0) then
			local parseInfo = { [TAG_PLAYERNAME] = getPlayerName(cid) }
			local msg = module.npcHandler:parseMessage(module.npcHandler:getMessage(MESSAGE_NOSHOP), parseInfo)

			module.npcHandler:say(msg, cid)
			return true
		end

		local parseInfo = { [TAG_PLAYERNAME] = getPlayerName(cid) }
		local msg = module.npcHandler:parseMessage(module.npcHandler:getMessage(MESSAGE_SENDTRADE), parseInfo)
		addEvent(openShopWindow, 100, cid, module.npcHandler.shopItems,
			function(cid, itemid, subType, amount, ignoreCap, inBackpacks)
				module.npcHandler:onBuy(cid, itemid, subType, amount, ignoreCap, inBackpacks)
			end,
			function(cid, itemid, subType, amount, ignoreCap, inBackpacks)
				module.npcHandler:onSell(cid, itemid, subType, amount, ignoreCap, inBackpacks)
			end
		)

		module.npcHandler:say(msg, cid)
		return true
	end

	-- onConfirm keyword callback function. Sells/buys the actual item.
	function ShopModule.onConfirm(cid, message, keywords, parameters, node)
		local module = parameters.module
		if(not module.npcHandler:isFocused(cid)) then
			return false
		end

		local parentParameters = node:getParent():getParameters()
		local parseInfo = {
			[TAG_PLAYERNAME] = getPlayerName(cid),
			[TAG_ITEMCOUNT] = module.amount,
			[TAG_TOTALCOST] = parentParameters.cost * module.amount,
			[TAG_ITEMNAME] = parentParameters.realName
		}

		if(parentParameters.eventType == SHOPMODULE_SELL_ITEM) then
			local ret = doPlayerSellItem(cid, parentParameters.itemid, module.amount, parentParameters.cost * module.amount)
			if(ret) then
				local msg = module.npcHandler:getMessage(MESSAGE_ONSELL)
				msg = module.npcHandler:parseMessage(msg, parseInfo)
				module.npcHandler:say(msg, cid)
			else
				local msg = module.npcHandler:getMessage(MESSAGE_MISSINGITEM)
				msg = module.npcHandler:parseMessage(msg, parseInfo)
				module.npcHandler:say(msg, cid)
			end
		elseif(parentParameters.eventType == SHOPMODULE_BUY_ITEM) then
			local ret = doPlayerBuyItem(cid, parentParameters.itemid, module.amount, parentParameters.cost * module.amount, parentParameters.subType)
			if(ret) then
				if parentParameters.itemid == ITEM_PARCEL then
					doPlayerBuyItem(cid, ITEM_LABEL, module.amount, 0, parentParameters.subType)
				end
				local msg = module.npcHandler:getMessage(MESSAGE_ONBUY)
				msg = module.npcHandler:parseMessage(msg, parseInfo)
				module.npcHandler:say(msg, cid)
			else
				local msg = module.npcHandler:getMessage(MESSAGE_MISSINGMONEY)
				msg = module.npcHandler:parseMessage(msg, parseInfo)
				module.npcHandler:say(msg, cid)
			end
		elseif(parentParameters.eventType == SHOPMODULE_BUY_ITEM_CONTAINER) then
			local ret = doPlayerBuyItemContainer(cid, parentParameters.container, parentParameters.itemid, module.amount, parentParameters.cost * module.amount, parentParameters.subType)
			if(ret) then
				local msg = module.npcHandler:getMessage(MESSAGE_ONBUY)
				msg = module.npcHandler:parseMessage(msg, parseInfo)
				module.npcHandler:say(msg, cid)
			else
				local msg = module.npcHandler:getMessage(MESSAGE_MISSINGMONEY)
				msg = module.npcHandler:parseMessage(msg, parseInfo)
				module.npcHandler:say(msg, cid)
			end
		end

		module.npcHandler:resetNpc(cid)
		return true
	end

	-- onDecliune keyword callback function. Generally called when the player sais 'no' after wanting to buy an item.
	function ShopModule.onDecline(cid, message, keywords, parameters, node)
		local module = parameters.module
		if(not module.npcHandler:isFocused(cid)) then
			return false
		end

		local parentParameters = node:getParent():getParameters()
		local parseInfo = {
			[TAG_PLAYERNAME] = getPlayerName(cid),
			[TAG_ITEMCOUNT] = module.amount,
			[TAG_TOTALCOST] = parentParameters.cost * module.amount,
			[TAG_ITEMNAME] = parentParameters.realName
		}

		local msg = module.npcHandler:parseMessage(module.noText, parseInfo)
		module.npcHandler:say(msg, cid)
		module.npcHandler:resetNpc(cid)
		return true
	end

	-- tradeItem callback function. Makes the npc say the message defined by MESSAGE_BUY or MESSAGE_SELL
	function ShopModule.tradeItem(cid, message, keywords, parameters, node)
		local module = parameters.module
		if(not module.npcHandler:isFocused(cid)) then
			return false
		end

		local count = module:getCount(message)
		module.amount = count
		local parseInfo = {
			[TAG_PLAYERNAME] = getPlayerName(cid),
			[TAG_ITEMCOUNT] = module.amount,
			[TAG_TOTALCOST] = parameters.cost * module.amount,
			[TAG_ITEMNAME] = parameters.realName
		}

		if(parameters.eventType == SHOPMODULE_SELL_ITEM) then
			local msg = module.npcHandler:getMessage(MESSAGE_SELL)
			msg = module.npcHandler:parseMessage(msg, parseInfo)
			module.npcHandler:say(msg, cid)
		elseif(parameters.eventType == SHOPMODULE_BUY_ITEM) then
			local msg = module.npcHandler:getMessage(MESSAGE_BUY)
			msg = module.npcHandler:parseMessage(msg, parseInfo)
			module.npcHandler:say(msg, cid)
		elseif(parameters.eventType == SHOPMODULE_BUY_ITEM_CONTAINER) then
			local msg = module.npcHandler:getMessage(MESSAGE_BUY)
			msg = module.npcHandler:parseMessage(msg, parseInfo)
			module.npcHandler:say(msg, cid)
		end

		return true
	end
end
