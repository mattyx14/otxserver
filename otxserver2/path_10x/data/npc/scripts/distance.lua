local keywordHandler = KeywordHandler:new()
local npcHandler = NpcHandler:new(keywordHandler)
NpcSystem.parseParameters(npcHandler)
local talkState = {}

function onCreatureAppear(cid)				npcHandler:onCreatureAppear(cid)			end
function onCreatureDisappear(cid)			npcHandler:onCreatureDisappear(cid)			end
function onCreatureSay(cid, type, msg)		npcHandler:onCreatureSay(cid, type, msg)	end
function onThink()							npcHandler:onThink()						end
function onPlayerEndTrade(cid)				npcHandler:onPlayerEndTrade(cid)			end
function onPlayerCloseChannel(cid)			npcHandler:onPlayerCloseChannel(cid)		end

local shopModule = ShopModule:new()
npcHandler:addModule(shopModule)

shopModule:addBuyableItem({'crossbow'}, 2455, 360, 'crossbow')
shopModule:addBuyableItem({'bow'}, 2456, 200, 'bow')
shopModule:addBuyableItem({'spear'}, 2389, 20, 1, 'spear')
shopModule:addBuyableItem({'poison arrow'}, 2545, 18, 1, 'poison arrow')
shopModule:addBuyableItem({'bolt'}, 2543, 18, 1, 'bolt')
shopModule:addBuyableItem({'arrow'}, 2545, 3, 1, 'arrow')

shopModule:addSellableItem({'crossbow'}, 2455, 150, 'crossbow')
shopModule:addSellableItem({'bow'}, 2456, 130, 'crossbow')

npcHandler:setCallback(CALLBACK_MESSAGE_DEFAULT, creatureSayCallback)
npcHandler:addModule(FocusModule:new())
