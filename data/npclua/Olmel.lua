local internalNpcName = "Olmel"
local npcType = Game.createNpcType(internalNpcName)
local npcConfig = {}

npcConfig.name = internalNpcName
npcConfig.description = internalNpcName

npcConfig.health = 100
npcConfig.maxHealth = npcConfig.health
npcConfig.walkInterval = 2000
npcConfig.walkRadius = 2

npcConfig.outfit = {
	lookType = 151,
	lookHead = 114,
	lookBody = 70,
	lookLegs = 144,
	lookFeet = 124,
	lookAddons = 1
}

npcConfig.flags = {
	floorchange = false
}

npcConfig.voices = {
	interval = 15000,
	chance = 50,
	{text = 'Passages to Fynn Castle, Sohan Town and to Samaransa Bay.'}
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

-- Travel
local function addTravelKeyword(keyword, cost, destination, action, condition)
	if condition then
		keywordHandler:addKeyword({keyword}, StdModule.say, {npcHandler = npcHandler, text = 'I\'m sorry but I don\'t sail there.'}, condition)
	end

	local travelKeyword = keywordHandler:addKeyword({keyword}, StdModule.say, {npcHandler = npcHandler, text = 'Do you seek a passage to ' .. keyword:titleCase() .. ' for |TRAVELCOST|?', cost = cost, discount = 'postman'})
	travelKeyword:addChildKeyword({'yes'}, StdModule.travel, {npcHandler = npcHandler, premium = false, cost = cost, discount = 'postman', destination = destination}, nil, action)
	travelKeyword:addChildKeyword({'no'}, StdModule.say, {npcHandler = npcHandler, text = 'We would like to serve you some time.', reset = true})
end

-- Block route
--[[
addTravelKeyword('sohan', 5000, Position(336, 1128, 6))
addTravelKeyword('samaransa bay', 1000, Position(354, 1282, 6))
]]

addTravelKeyword('fynn', 1000, Position(958, 1025, 6))

-- Kick
keywordHandler:addKeyword({'kick'}, StdModule.kick, {npcHandler = npcHandler, destination = {Position(962, 1027, 7), Position(962, 1023, 7)}})

-- Basic
keywordHandler:addKeyword({'name'}, StdModule.say, {npcHandler = npcHandler, text = 'My name is Captain Olmel from the DarkKonia Line.'})
keywordHandler:addKeyword({'job'}, StdModule.say, {npcHandler = npcHandler, text = 'I am the captain of this sailing-ship.'})
keywordHandler:addKeyword({'captain'}, StdModule.say, {npcHandler = npcHandler, text = 'I am the captain of this sailing-ship.'})
keywordHandler:addKeyword({'ship'}, StdModule.say, {npcHandler = npcHandler, text = 'The DarkKonia Line connects all seaside towns of the world.'})
keywordHandler:addKeyword({'line'}, StdModule.say, {npcHandler = npcHandler, text = 'The DarkKonia Line connects all seaside towns of the world.'})
keywordHandler:addKeyword({'company'}, StdModule.say, {npcHandler = npcHandler, text = 'The DarkKonia Line connects all seaside towns of the world.'})
keywordHandler:addKeyword({'world'}, StdModule.say, {npcHandler = npcHandler, text = 'The DarkKonia Line connects all seaside towns of world.'})
keywordHandler:addKeyword({'good'}, StdModule.say, {npcHandler = npcHandler, text = 'We can transport everything you want.'})
keywordHandler:addKeyword({'passenger'}, StdModule.say, {npcHandler = npcHandler, text = 'We would like to welcome you on board.'})
keywordHandler:addKeyword({'trip'}, StdModule.say, {npcHandler = npcHandler, text = 'Where do you want to go? To {Fynn Castle}, {Sohan Town} and to {Samaransa Bay}?'})
keywordHandler:addKeyword({'route'}, StdModule.say, {npcHandler = npcHandler, text = 'Where do you want to go? To {Fynn Castle}, {Sohan Town} and to {Samaransa Bay}?'})
keywordHandler:addKeyword({'passage'}, StdModule.say, {npcHandler = npcHandler, text = 'Where do you want to go? To {Fynn Castle}, {Sohan Town} and to {Samaransa Bay}?'})
keywordHandler:addKeyword({'destination'}, StdModule.say, {npcHandler = npcHandler, text = 'Where do you want to go? To {Fynn Castle}, {Sohan Town} and to {Samaransa Bay}?'})
keywordHandler:addKeyword({'sail'}, StdModule.say, {npcHandler = npcHandler, text = 'Where do you want to go? To {Fynn Castle}, {Sohan Town} and to {Samaransa Bay}?'})
keywordHandler:addKeyword({'go'}, StdModule.say, {npcHandler = npcHandler, text = 'Where do you want to go? To {Fynn Castle}, {Sohan Town} and to {Samaransa Bay}?'})

-- Block route
keywordHandler:addKeyword({'samaransa'}, StdModule.say, {npcHandler = npcHandler, text = 'This area {Samaransa Bay} is not completed not have access ...'})
keywordHandler:addKeyword({'sohan'}, StdModule.say, {npcHandler = npcHandler, text = 'This area {Sohan Town} is not completed not have access ...'})

npcHandler:setMessage(MESSAGE_GREET, 'Welcome on board, |PLAYERNAME|. Where can I {sail} you today?')
npcHandler:setMessage(MESSAGE_FAREWELL, 'Good bye. Recommend us if you were satisfied with our service.')
npcHandler:setMessage(MESSAGE_WALKAWAY, 'Good bye then.')

npcHandler:addModule(FocusModule:new())

-- npcType registering the npcConfig table
npcType:register(npcConfig)
