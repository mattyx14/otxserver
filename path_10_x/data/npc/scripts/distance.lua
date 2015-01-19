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

-- Buy Item NO STACKEABLE EXAMPLE
shopModule:addBuyableItem({'crossbow'}, 2455, 360, 'crossbow')
shopModule:addBuyableItem({'bow'}, 2456, 200, 'bow')
-- Buy Item STACKEABLE EXAMPLE
shopModule:addBuyableItem({'spear'}, 2389, 20, 1, 'spear')
shopModule:addBuyableItem({'poison arrow'}, 2545, 18, 1, 'poison arrow')
shopModule:addBuyableItem({'bolt'}, 2543, 18, 1, 'bolt')
shopModule:addBuyableItem({'arrow'}, 2545, 3, 1, 'arrow')

-- Sell Item NO STACKEABLE EXAMPLE
shopModule:addSellableItem({'crossbow'}, 2455, 150, 'crossbow')
shopModule:addSellableItem({'bow'}, 2456, 130, 'crossbow')
-- Sell Item STACKEABLE EXAMPLE
shopModule:addSellableItem({'bolt'}, 2543, 8, 1, 'bolt')
shopModule:addSellableItem({'arrow'}, 2545, 2, 1, 'arrow')

npcHandler:setCallback(CALLBACK_MESSAGE_DEFAULT, creatureSayCallback)
npcHandler:addModule(FocusModule:new())
