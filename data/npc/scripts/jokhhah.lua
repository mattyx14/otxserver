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

local function creatureSayCallback(cid, type, msg)
	if not npcHandler:isFocused(cid) then
		return false
	end
	local player = Player(cid)
	if isInArray({"enchanted chicken wing", "enchanted chicken wing"}, msg) then
		npcHandler:say('Do you want to trade enchanted chicken wing for boots of haste?', cid)
		npcHandler.topic[cid] = 1
	elseif isInArray({"warrior sweat", "warrior helmets"}, msg) then
		npcHandler:say('Do you want to trade 4 Warrior Helmets for Warrior Sweat?', cid)
		npcHandler.topic[cid] = 2
	elseif isInArray({"fighting spirit", "royal helmets"}, msg) then
		npcHandler:say('Do you want to trade 2 Royal Helmets for Fighting Spirit?', cid)
		npcHandler.topic[cid] = 3
	elseif isInArray({"magic sulphur", "fire swords"}, msg) then
		npcHandler:say('Do you want to trade 2 Fire Swords for Magic Sulphur?', cid)
		npcHandler.topic[cid] = 4
	elseif isInArray({"magic sulphur", "giant swords"}, msg) then
		npcHandler:say('Do you want to trade 3 Giant Swords for Huge chunk of Crude Iron?', cid)
		npcHandler.topic[cid] = 5
	elseif isInArray({"piece of royal steel", "crown armors"}, msg) then
		npcHandler:say('Do you want to trade 2 Crown Armors for Piece of Royal Steel?', cid)
		npcHandler.topic[cid] = 6
	elseif isInArray({"spider silks", "spool of yarn"}, msg) then
		npcHandler:say('Do you want to trade 10 Spider Silks for Spool of Yarn?', cid)
		npcHandler.topic[cid] = 7
	elseif isInArray({"job", "items"}, msg) then
		npcHandler:say('I trade: {Enchanted Chicken Wing}, {Warrior Helmets}, {Royal Helmets}, {Fire Swords}, {Giant Swords}, {Crown Armors}, {Spider Silks}.', cid)
		npcHandler.topic[cid] = 0
	elseif msgcontains(msg,'yes') and npcHandler.topic[cid] <= 7 and npcHandler.topic[cid] >= 1 then
		local trade = {
			{ NeedItem = 5891, Ncount = 1, GiveItem = 2195, Gcount = 1}, -- Boots of Haste
			{ NeedItem = 2475, Ncount = 4, GiveItem = 5885, Gcount = 1}, -- Flask of Warrior's Sweat
			{ NeedItem = 2498, Ncount = 2, GiveItem = 5884, Gcount = 1}, -- Spirit Container
			{ NeedItem = 2392, Ncount = 2, GiveItem = 5904, Gcount = 1},  -- Magic Sulphur
			{ NeedItem = 2393, Ncount = 3, GiveItem = 5892, Gcount = 1},  -- Huge chunk of Crude Iron
			{ NeedItem = 2487, Ncount = 2, GiveItem = 5887, Gcount = 1},  -- Piece of Royal Steel
			{ NeedItem = 5879, Ncount = 10, GiveItem = 5886, Gcount = 1},  -- Piece of Royal Steel
		}
		if player:getItemCount(trade[npcHandler.topic[cid]].NeedItem) >= trade[npcHandler.topic[cid]].Ncount then
			player:removeItem(trade[npcHandler.topic[cid]].NeedItem, trade[npcHandler.topic[cid]].Ncount)
			player:addItem(trade[npcHandler.topic[cid]].GiveItem, trade[npcHandler.topic[cid]].Gcount)
			return npcHandler:say('Here you are.', cid)
		else
			npcHandler:say('Sorry but you don\'t have the item.', cid)
		end
	elseif msgcontains(msg,'no') and (npcHandler.topic[cid] >= 1 and npcHandler.topic[cid] <= 8) then
		npcHandler:say('Ok then.', cid)
		npcHandler.topic[cid] = 0
		npcHandler:releaseFocus(cid)
		npcHandler:resetNpc(cid)
	end
	return true
end

local function onTradeRequest(cid)
	if Player(cid):getStorageValue(Storage.Missions.DjinnMission.Done) ~= 1 then
		npcHandler:say('Sorry, but you do not complete the quest Djinn u need quest {mystic blade}, {heroic Axe}, {skull staff} and {oil lamp} quest. I have to make sure that I can trust in the quality of your wares.', cid)
		return false
	end

	return true
end

npcHandler:setCallback(CALLBACK_ONTRADEREQUEST, onTradeRequest)
npcHandler:setCallback(CALLBACK_MESSAGE_DEFAULT, creatureSayCallback)
npcHandler:addModule(FocusModule:new())
