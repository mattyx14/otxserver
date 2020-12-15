local keywordHandler = KeywordHandler:new()
local npcHandler = NpcHandler:new(keywordHandler)
NpcSystem.parseParameters(npcHandler)

function onCreatureAppear(cid)
	npcHandler:onCreatureAppear(cid)
end
function onCreatureDisappear(cid)
	npcHandler:onCreatureDisappear(cid)
end
function onCreatureSay(cid, type, msg)
	npcHandler:onCreatureSay(cid, type, msg)
end
function onThink()
	npcHandler:onThink()
end

local config = {
	price = 10000
}

local function creatureSayCallback(cid, type, msg)
	if not npcHandler:isFocused(cid) then
		return false
	end
	local player = Player(cid)
	if msgcontains(msg, "fynn") then
		if player:getStorageValue(Storage.FirstQuest.rewardMisidia) ~= 1 then
			npcHandler:say("You don't complete the first Misidian quest.", cid)
		elseif player:getStorageValue(Storage.FirstQuest.rewardMisidia) == 1 then
			npcHandler:say("Do you seek a seek a passage to {Fynn}?", cid)
			npcHandler.topic[cid] = 1
		else
		return false
		end
	elseif msgcontains(msg, "yes") then
		if npcHandler.topic[cid] == 1 then
			if player:removeMoneyNpc(config.price) then
				npcHandler:say("Let's go fo' a hunt and bring the beast down!", cid)
				doTeleportThing(cid, Position(958, 1024, 6))
				player:getPosition():sendMagicEffect(CONST_ME_TELEPORT)
				npcHandler.topic[cid] = 0
			else
				npcHandler:say("You don\'t have enough money.", cid)
				npcHandler.topic[cid] = 0
			end
		end
	end

	return true
end

-- Travel
local function addTravelKeyword(keyword, cost, destination, action)
	local travelKeyword = keywordHandler:addKeyword({keyword}, StdModule.say, {npcHandler = npcHandler, text = 'Do you seek a seek a passage to ' .. keyword:titleCase() .. ' for |TRAVELCOST|?', cost = cost, discount = 'postman'})
		travelKeyword:addChildKeyword({'yes'}, StdModule.travel, {npcHandler = npcHandler, premium = true, cost = cost, destination = destination}, nil, action)
		travelKeyword:addChildKeyword({'no'}, StdModule.say, {npcHandler = npcHandler, text = 'We would like to serve you some time.', reset = true})
end

addTravelKeyword('elfic', 1000, Position(657, 1216, 6))
addTravelKeyword('sohan', 1000, Position(336, 1128, 6))
addTravelKeyword('anshara', 1000, Position(647, 361, 6))
addTravelKeyword('sunken', 5000, Position(1271, 274, 7))
keywordHandler:addKeyword({'travel'}, StdModule.say, {npcHandler = npcHandler, text = 'Where do you want to go? To {Anshara Desert}, {Sohan Town}, {Misidia Settlement}, {Sunken Sanctuary} and to {Elfic Ville}?'})

npcHandler:setCallback(CALLBACK_MESSAGE_DEFAULT, creatureSayCallback)
npcHandler:setMessage(MESSAGE_GREET, 'Welcome on board, |PLAYERNAME|. Where may I {travel} you today?')
npcHandler:setMessage(MESSAGE_FAREWELL, 'Good bye. Recommend us if you were satisfied with our service.')
npcHandler:setMessage(MESSAGE_WALKAWAY, 'Good bye then.')
npcHandler:addModule(FocusModule:new())
