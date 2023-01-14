local internalNpcName = "Yura"
local npcType = Game.createNpcType(internalNpcName)
local npcConfig = {}

npcConfig.name = internalNpcName
npcConfig.description = internalNpcName

npcConfig.health = 100
npcConfig.maxHealth = npcConfig.health
npcConfig.walkInterval = 2000
npcConfig.walkRadius = 2

npcConfig.outfit = {
	lookType = 133, 
	lookHead = 39, 
	lookBody = 122, 
	lookLegs = 125, 
	lookFeet = 57, 
	lookAddons = 3
}

npcConfig.flags = {
	floorchange = false
}

npcConfig.voices = {
	interval = 15000, 
	chance = 50, 
	{text = 'Selling all sorts of magic equipment. Come and have a look'}
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

local RodWand = {
	[VOCATION.BASE_ID.SORCERER] = 3074, 
	[VOCATION.BASE_ID.DRUID] = 3066
}

local function creatureSayCallback(npc, creature, type, message)
	local player = Player(creature)
	local playerId = player:getId()

	if not npcHandler:checkInteraction(npc, creature) then
		return false
	end

	local itemIdRodWand = RodWand[player:getVocation():getBaseId()]
	if MsgContains(message, 'first rod') or MsgContains(message, 'first wand') then
		if player:isMage() then
			if player:getStorageValue(Storage.FirstQuest.FirstWeapon) == -1 then
				npcHandler:say('You ask me you begin your adventure with the {' .. ItemType(itemIdRodWand):getName() .. '}, ok?', npc, creature)
				npcHandler:setTopic(playerId, 1)
			else
				npcHandler:say('What? I have already gave you one {' .. ItemType(itemIdRodWand):getName() .. '}!', npc, creature)
			end
		else
			npcHandler:say('Sorry, you aren\'t a druid either a sorcerer.', npc, creature)
		end
	elseif MsgContains(message, 'yes') then
		if npcHandler:getTopic(playerId) == 1 then
			player:addItem(itemIdRodWand, 1)
			npcHandler:say('Here you are young adept, take care yourself.', npc, creature)
			player:setStorageValue(Storage.FirstQuest.FirstWeapon, 1)
		end
		npcHandler:setTopic(playerId, 0)
	elseif MsgContains(message, 'no') and npcHandler:getTopic(playerId) == 1 then
		npcHandler:say('Ok then.', npc, creature)
		npcHandler:setTopic(playerId, 0)
	end
	return true
end

keywordHandler:addKeyword({'mystical'}, StdModule.say, {npcHandler = npcHandler, text = "Or do you want to look at {runes}, {potions}, {magic weapons} or {spellbooks}?."})
keywordHandler:addKeyword({'magic weapons'}, StdModule.say, {npcHandler = npcHandler, text = "I'm selling rods and wands. If you like to see my offers, ask me for a {trade}."})
keywordHandler:addKeyword({'runes'}, StdModule.say, {npcHandler = npcHandler, text = "I'm selling runes. If you like to see my offers, ask me for a {trade}."})
keywordHandler:addKeyword({'potions'}, StdModule.say, {npcHandler = npcHandler, text = "I'm selling potions. If you like to see my offers, ask me for a {trade}."})
keywordHandler:addKeyword({'spellbooks'}, StdModule.say, {npcHandler = npcHandler, text = "I'm also buy powerful spellbooks. If you like to see my offers, ask me for a {trade}."})
keywordHandler:addKeyword({'free rod and wand'}, StdModule.say, {npcHandler = npcHandler, text = "If need {first rod} or {first wand} only ask me."})

npcHandler:setCallback(CALLBACK_MESSAGE_DEFAULT, creatureSayCallback)
npcHandler:setMessage(MESSAGE_GREET, "Hi there |PLAYERNAME|, and welcome to the {mystical} store. Or need your {free rod and wand}.")
npcHandler:setMessage(MESSAGE_FAREWELL, "See you, |PLAYERNAME|.")
npcHandler:setMessage(MESSAGE_WALKAWAY, "See you, |PLAYERNAME|.")
npcHandler:setMessage(MESSAGE_SENDTRADE, "Of course, just browse through my wares.")
npcHandler:addModule(FocusModule:new())

npcConfig.shop = {
	{ itemName = "animate dead rune", clientId = 3203, buy = 375 },
	{ itemName = "avalanche rune", clientId = 3161, buy = 57 },
	{ itemName = "blue rose", clientId = 3659, sell = 200 },
	{ itemName = "butterfly ring", clientId = 25698, sell = 2000 },
	{ itemName = "chameleon rune", clientId = 3178, buy = 210 },
	{ itemName = "convince creature rune", clientId = 3177, buy = 80 },
	{ itemName = "cure poison rune", clientId = 3153, buy = 65 },
	{ itemName = "destroy field rune", clientId = 3148, buy = 15 },
	{ itemName = "disintegrate rune", clientId = 3197, buy = 26 },
	{ itemName = "dream blossom staff", clientId = 25700, sell = 5000 },
	{ itemName = "durable exercise rod", clientId = 35283, buy = 945000, count = 1800 },
	{ itemName = "durable exercise wand", clientId = 35284, buy = 945000, count = 1800 },
	{ itemName = "empty potion flask", clientId = 283, sell = 5 },
	{ itemName = "empty potion flask", clientId = 284, sell = 5 },
	{ itemName = "empty potion flask", clientId = 285, sell = 5 },
	{ itemName = "energy bomb rune", clientId = 3149, buy = 203 },
	{ itemName = "energy field rune", clientId = 3164, buy = 38 },
	{ itemName = "energy wall rune", clientId = 3166, buy = 85 },
	{ itemName = "exercise rod", clientId = 28556, buy = 262500, count = 500 },
	{ itemName = "exercise wand", clientId = 28557, buy = 262500, count = 500 },
	{ itemName = "explosion rune", clientId = 3200, buy = 31 },
	{ itemName = "fire bomb rune", clientId = 3192, buy = 147 },
	{ itemName = "fire field rune", clientId = 3188, buy = 28 },
	{ itemName = "fire wall rune", clientId = 3190, buy = 61 },
	{ itemName = "fireball rune", clientId = 3189, buy = 30 },
	{ itemName = "hailstorm rod", clientId = 3067, buy = 15000, sell = 3000 },
	{ itemName = "great fireball rune", clientId = 3191, buy = 57 },
	{ itemName = "great health potion", clientId = 239, buy = 225 },
	{ itemName = "great mana potion", clientId = 238, buy = 144 },
	{ itemName = "great spirit potion", clientId = 7642, buy = 228 },
	{ itemName = "green crystal fragment", clientId = 16127, sell = 800 },
	{ itemName = "green crystal shard", clientId = 16121, sell = 1500 },
	{ itemName = "green crystal splinter", clientId = 16122, sell = 400 },
	{ itemName = "health potion", clientId = 266, buy = 50 },
	{ itemName = "heavy magic missile rune", clientId = 3198, buy = 12 },
	{ itemName = "holy missile rune", clientId = 3182, buy = 16 },
	{ itemName = "icicle rune", clientId = 3158, buy = 30 },
	{ itemName = "intense healing rune", clientId = 3152, buy = 95 },
	{ itemName = "lasting exercise rod", clientId = 35289, buy = 7560000, count = 14400 },
	{ itemName = "lasting exercise wand", clientId = 35290, buy = 7560000, count = 14400 },
	{ itemName = "light magic missile rune", clientId = 3174, buy = 4 },
	{ itemName = "magic shield potion", clientId = 35563, buy = 250 },
	{ itemName = "magic wall rune", clientId = 3180, buy = 116 },
	{ itemName = "mana potion", clientId = 268, buy = 56 },
	{ itemName = "moonlight rod", clientId = 3070, buy = 1000, sell = 200 },
	{ itemName = "necrotic rod", clientId = 3069, buy = 5000, sell = 1000 },
	{ itemName = "northwind rod", clientId = 8083, buy = 7500, sell = 1500 },
	{ itemName = "paralyse rune", clientId = 3165, buy = 700 },
	{ itemName = "poison bomb rune", clientId = 3173, buy = 85 },
	{ itemName = "poison field rune", clientId = 3172, buy = 21 },
	{ itemName = "poison wall rune", clientId = 3176, buy = 52 },
	{ itemName = "powder herb", clientId = 3739, sell = 10 },
	{ itemName = "red rose", clientId = 3658, sell = 10 },
	{ itemName = "sling herb", clientId = 3738, sell = 10 },
	{ itemName = "snakebite rod", clientId = 3066, buy = 500, sell = 100 },
	{ itemName = "soulfire rune", clientId = 3195, buy = 46 },
	{ itemName = "spellbook of enlightenment", clientId = 8072, sell = 4000 },
	{ itemName = "spellbook of lost souls", clientId = 8075, sell = 19000 },
	{ itemName = "spellbook of mind control", clientId = 8074, sell = 13000 },
	{ itemName = "spellbook of warding", clientId = 8073, sell = 8000 },
	{ itemName = "springsprout rod", clientId = 8084, buy = 18000, sell = 3600 },
	{ itemName = "stalagmite rune", clientId = 3179, buy = 12 },
	{ itemName = "star herb", clientId = 3736, sell = 15 },
	{ itemName = "stone herb", clientId = 3735, sell = 20 },
	{ itemName = "stone shower rune", clientId = 3175, buy = 37 },
	{ itemName = "strong health potion", clientId = 236, buy = 115 },
	{ itemName = "strong mana potion", clientId = 237, buy = 93 },
	{ itemName = "sudden death rune", clientId = 3155, buy = 135 },
	{ itemName = "supreme health potion", clientId = 23375, buy = 625 },
	{ itemName = "terra rod", clientId = 3065, buy = 10000, sell = 2000 },
	{ itemName = "thunderstorm rune", clientId = 3202, buy = 47 },
	{ itemName = "ultimate healing rune", clientId = 3160, buy = 175 },
	{ itemName = "ultimate health potion", clientId = 7643, buy = 379 },
	{ itemName = "ultimate mana potion", clientId = 23373, buy = 438 },
	{ itemName = "ultimate spirit potion", clientId = 23374, buy = 438 },
	{ itemName = "underworld rod", clientId = 8082, buy = 22000, sell = 4400 },
	{ itemName = "vial", clientId = 2874, sell = 5 },
	{ itemName = "wand of cosmic energy", clientId = 3073, buy = 10000, sell = 2000 },
	{ itemName = "wand of decay", clientId = 3072, buy = 5000, sell = 1000 },
	{ itemName = "wand of defiance", clientId = 16096, sell = 6500 },
	{ itemName = "wand of draconia", clientId = 8093, buy = 7500, sell = 1500 },
	{ itemName = "wand of dragonbreath", clientId = 3075, buy = 1000, sell = 200 },
	{ itemName = "wand of everblazing", clientId = 16115, sell = 6000 },
	{ itemName = "wand of inferno", clientId = 3071, buy = 15000, sell = 3000 },
	{ itemName = "wand of starstorm", clientId = 8092, buy = 18000, sell = 3600 },
	{ itemName = "wand of voodoo", clientId = 8094, buy = 22000, sell = 4400 },
	{ itemName = "wand of vortex", clientId = 3074, buy = 500, sell = 100 },
	{ itemName = "wild growth rune", clientId = 3156, buy = 160 },
	{ itemName = "wooden spellbook", clientId = 25699, sell = 12000 },
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
