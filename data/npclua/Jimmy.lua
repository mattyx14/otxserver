local internalNpcName = "Jimmy"
local npcType = Game.createNpcType(internalNpcName)
local npcConfig = {}

npcConfig.name = internalNpcName
npcConfig.description = internalNpcName

npcConfig.health = 100
npcConfig.maxHealth = npcConfig.health
npcConfig.walkInterval = 2000
npcConfig.walkRadius = 2

npcConfig.outfit = {
	lookType = 128,
	lookHead = 120,
	lookBody = 111,
	lookLegs = 104,
	lookFeet = 114,
	lookAddons = 1
}

npcConfig.flags = {
	floorchange = false
}

npcConfig.voices = {
	interval = 15000,
	chance = 50,
	{text = 'Stop by and rest a while, tired adventurer! Have a look at my wares!'}
}

local keywordHandler = KeywordHandler:new()
local npcHandler = NpcHandler:new(keywordHandler)

npcType.onThink = function(npc, interval)
	npcHandler:onThink(npc, interval)
end

npcType.onAppear = function(npc, creature)
	npcHandler:onAppear(npc, creature)
end

npcType.onDisappear = function(npc, creature)
	npcHandler:onDisappear(npc, creature)
end

npcType.onMove = function(npc, creature, fromPosition, toPosition)
	npcHandler:onMove(npc, creature, fromPosition, toPosition)
end

npcType.onSay = function(npc, creature, type, message)
	npcHandler:onSay(npc, creature, type, message)
end

npcType.onCloseChannel = function(npc, creature)
	npcHandler:onCloseChannel(npc, creature)
end

local function creatureSayCallback(npc, creature, type, message)
	local player = Player(creature)
	local playerId = player:getId()

	if not npcHandler:checkInteraction(npc, creature) then
		return false
	end


	-- Citizen outfit addon
	local addonProgress = player:getStorageValue(Storage.OutfitQuest.Citizen.AddonBackpack)
	if MsgContains(message, 'addon') or MsgContains(message, 'outfit')
			or (addonProgress == 1 and MsgContains(message, 'leather'))
			or ((addonProgress == 1 or addonProgress == 2) and MsgContains(message, 'backpack')) then
		if addonProgress < 1 then
			npcHandler:say('Sorry, the backpack I wear is not for sale. It\'s handmade from rare minotaur leather.', npc, creature)
			npcHandler:setTopic(playerId, 1)
		elseif addonProgress == 1 then
			npcHandler:say('Ah, right, almost forgot about the backpack! Have you brought me 100 pieces of minotaur leather as requested?', npc, creature)
			npcHandler:setTopic(playerId, 3)
		elseif addonProgress == 2 then
			if player:getStorageValue(Storage.OutfitQuest.Citizen.AddonBackpackTimer) < os.time() then
				npcHandler:say('Just in time! Your backpack is finished. Here you go, I hope you like it.', npc, creature)
				player:getPosition():sendMagicEffect(CONST_ME_MAGIC_BLUE)
				player:setStorageValue(Storage.OutfitQuest.Ref, math.min(0, player:getStorageValue(Storage.OutfitQuest.Ref) - 1))
				player:setStorageValue(Storage.OutfitQuest.Citizen.MissionBackpack, 0)
				player:setStorageValue(Storage.OutfitQuest.Citizen.AddonBackpack, 3)

				player:addOutfitAddon(136, 1)
				player:addOutfitAddon(128, 1)
			else
				npcHandler:say('Uh... I didn\'t expect you to return that early. Sorry, but I\'m not finished yet with your backpack. I\'m doing the best I can, promised.', npc, creature)
			end
		elseif addonProgress == 3 then
			npcHandler:say('Sorry, but I can only make one backpack per person, else I\'d have to close my shop and open a leather manufactory.', npc, creature)
		end
		return true
	end
	if npcHandler:getTopic(playerId) == 1 then
		if MsgContains(message, 'backpack') or MsgContains(message, 'minotaur') or MsgContains(message, 'leather') then
			npcHandler:say('Well, if you really like this backpack, I could make one for you, but minotaur leather is hard to come by these days. Are you willing to put some work into this?', npc, creature)
			npcHandler:setTopic(playerId, 2)
		end
	elseif npcHandler:getTopic(playerId) == 2 then
		if MsgContains(message, 'yes') then
			player:setStorageValue(Storage.OutfitQuest.Ref, math.max(0, player:getStorageValue(Storage.OutfitQuest.Ref)) + 1)
			player:setStorageValue(Storage.OutfitQuest.Citizen.AddonBackpack, 1)
			player:setStorageValue(Storage.OutfitQuest.Citizen.MissionBackpack, 1)
			npcHandler:say('Alright then, if you bring me 100 pieces of fine minotaur leather I will see what I can do for you. You probably have to kill really many minotaurs though... so good luck!', npc, creature)
			npcHandler:removeInteraction(npc, creature)
		else
			npcHandler:say('Sorry, but I don\'t run a welfare office, you know... no pain, no gain.', npc, creature)
		end
		npcHandler:setTopic(playerId, 0)
	elseif npcHandler:getTopic(playerId) == 3 then
		if MsgContains(message, 'yes') then
			if player:getItemCount(5878) < 100 then
				npcHandler:say('Sorry, but that\'s not enough leather yet to make one of these backpacks. Would you rather like to buy a normal backpack for 10 gold?', npc, creature)
			else
				npcHandler:say('Great! Alright, I need a while to finish this backpack for you. Come ask me later, okay?', npc, creature)

				player:removeItem(5878, 100)

				player:setStorageValue(Storage.OutfitQuest.Citizen.MissionBackpack, 2)
				player:setStorageValue(Storage.OutfitQuest.Citizen.AddonBackpack, 2)
				player:setStorageValue(Storage.OutfitQuest.Citizen.AddonBackpackTimer, os.time() + 2 * 60 * 60)
			end
		else
			npcHandler:say('I know, it\'s quite some work... don\'t lose heart, just keep killing minotaurs and you\'ll eventually get lucky. Would you rather like to buy a normal backpack for 10 gold?', npc, creature)
		end
		npcHandler:setTopic(playerId, 0)
	end
	return true
end

npcHandler:setMessage(MESSAGE_GREET, 'Welcome to my adventurer shop, |PLAYERNAME|! What do you need? Ask me for a {trade} to look at my wares.')
npcHandler:setMessage(MESSAGE_FAREWELL, 'Good bye, |PLAYERNAME|.')
npcHandler:setMessage(MESSAGE_WALKAWAY, 'Good bye.')

npcHandler:setCallback(CALLBACK_MESSAGE_DEFAULT, creatureSayCallback)
npcHandler:addModule(FocusModule:new())

npcConfig.shop = {
	{ itemName = "backpack", clientId = 2854, buy = 25 },
	{ itemName = "basket", clientId = 2855, buy = 6 },
	{ itemName = "bottle", clientId = 2875, buy = 3 },
	{ itemName = "bucket", clientId = 2873, buy = 4 },
	{ itemName = "candelabrum", clientId = 2927, buy = 8 },
	{ itemName = "candlestick", clientId = 2917, buy = 2 },
	{ itemName = "closed trap", clientId = 3481, buy = 280, sell = 75 },
	{ itemName = "crowbar", clientId = 3304, buy = 260, sell = 50 },
	{ itemName = "fishing rod", clientId = 3483, buy = 150, sell = 40 },
	{ itemName = "hand auger", clientId = 31334, buy = 25 },
	{ itemName = "machete", clientId = 3308, buy = 35, sell = 6 },
	{ itemName = "net", clientId = 31489, buy = 50 },
	{ itemName = "pick", clientId = 3456, buy = 50, sell = 15 },
	{ itemName = "present", clientId = 2856, buy = 10 },
	{ itemName = "red apple", clientId = 3585, buy = 3 },
	{ itemName = "rope", clientId = 3003, buy = 50, sell = 15 },
	{ itemName = "scythe", clientId = 3453, buy = 50, sell = 10 },
	{ itemName = "shovel", clientId = 3457, buy = 50, sell = 8 },
	{ itemName = "watch", clientId = 2906, buy = 20, sell = 6 },
	{ itemName = "waterskin of water", clientId = 2901, buy = 10, count = 1 },
	{ itemName = "wooden hammer", clientId = 3459, sell = 15 },
	{ itemName = "worm", clientId = 3492, buy = 1 }
}
-- On buy npc shop message
npcType.onBuyItem = function(npc, player, itemId, subType, amount, inBackpacks, name, totalCost)
	npc:sellItem(player, itemId, amount, subType, true, inBackpacks, 2854)
	player:sendTextMessage(MESSAGE_INFO_DESCR, string.format("Bought %ix %s for %i %s.", amount, name, totalCost, ItemType(npc:getCurrency()):getPluralName():lower()))
end
-- On sell npc shop message
npcType.onSellItem = function(npc, player, clientId, subtype, amount, name, totalCost)
	player:sendTextMessage(MESSAGE_INFO_DESCR, string.format("Sold %ix %s for %i gold.", amount, name, totalCost))
end
-- On check npc shop message (look item)
npcType.onCheckItem = function(npc, player, clientId, subType)
end

npcType:register(npcConfig)
